// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// Polls the .NET API boundary (Phase 7) and renders simulated endpoints and
// their patch status in a table. The API base URL and schedule id are
// configurable via the PATCHORCH_API_URL and PATCHORCH_SCHEDULE_ID env vars.

#include "dashboard.hpp"
#include "animated_progress_bar.hpp"
#include "demo_app_context.hpp"
#include "fleet_summary_panel.hpp"
#include "log.hpp"
#include "state_badge.hpp"

#include <QCloseEvent>
#include <QDockWidget>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStatusBar>
#include <QTableWidget>
#include <QUrl>

#include <cstdlib>

namespace {

constexpr int kDefaultPollIntervalMs = 2000;
constexpr int kDefaultSeed = 42;

// Default endpoint set used to drive the simulation on startup.
const QJsonArray kDefaultEndpoints = [] {
    QJsonArray arr;
    arr.append(QJsonObject{{"id", "ep-1"}, {"failure_rate", 0.1}});
    arr.append(QJsonObject{{"id", "ep-2"}, {"failure_rate", 0.0}});
    arr.append(QJsonObject{{"id", "ep-3"}, {"failure_rate", 0.3}});
    return arr;
}();

QString envOr(const char *name, const QString &fallback)
{
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0') {
        return fallback;
    }
    return QString::fromUtf8(value);
}

} // namespace

// Sprint 19 (C2): centralized state->color mapping (single source of truth).
// Every row/badge in the dashboard derives its color from this one function,
// so a state always maps to the same color everywhere.
//
// Sprint 20 (C3): the mapping now lives in the reusable StateBadge widget;
// the dashboard delegates to it so the badge and the dashboard stay in sync.
QColor DashboardWindow::colorForState(const QString &state)
{
    return StateBadge::colorForState(state);
}

DashboardWindow::DashboardWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_table(nullptr)
    , m_baseUrl(envOr("PATCHORCH_API_URL", QStringLiteral("http://localhost:5000")))
    , m_scheduleId(envOr("PATCHORCH_SCHEDULE_ID", QStringLiteral("sch-1")))
    , m_scheduleReady(false)
    , m_context(nullptr)
    , m_streamReply(nullptr)
    , m_summary(nullptr)
{
    setWindowTitle(QStringLiteral("PatchOrchestrator — Dashboard"));
    resize(720, 420);

    buildUi();

    connect(&m_timer, &QTimer::timeout, this, &DashboardWindow::onPollTick);
    m_timer.start(kDefaultPollIntervalMs);

    ensureSchedule();
}

void DashboardWindow::setContext(DemoAppContext *context)
{
    m_context = context;
    if (m_context == nullptr)
        return;

    // Adopt the shared schedule id and API base URL.
    m_baseUrl = m_context->apiBaseUrl();
    m_scheduleId = m_context->scheduleId();
    statusBar()->showMessage(QStringLiteral("Connecting to %1 ...").arg(m_baseUrl));

    // Propagate shared-state changes into this panel.
    connect(m_context, &DemoAppContext::apiBaseUrlChanged, this,
            [this](const QString &url) { m_baseUrl = url; });
    connect(m_context, &DemoAppContext::scheduleIdChanged, this,
            [this](const QString &id) { m_scheduleId = id; });
}

void DashboardWindow::setPollIntervalMs(int ms)
{
    m_timer.setInterval(ms);
}

void DashboardWindow::stopPolling()
{
    m_timer.stop();
}

int DashboardWindow::rowCount() const
{
    return m_table->rowCount();
}

QString DashboardWindow::cellText(int row, int col) const
{
    QTableWidgetItem *item = m_table->item(row, col);
    return item != nullptr ? item->text() : QString();
}

int DashboardWindow::progressBarCount() const
{
    return m_progressBars.size();
}

int DashboardWindow::progressBarValue(int row) const
{
    if (row < 0 || row >= m_progressBars.size() || m_progressBars[row] == nullptr)
        return -1;
    return m_progressBars[row]->value();
}

int DashboardWindow::progressBarTarget(int row) const
{
    if (row < 0 || row >= m_progressBars.size() || m_progressBars[row] == nullptr)
        return -1;
    return m_progressBars[row]->target();
}

QColor DashboardWindow::rowStateColor(int row) const
{
    if (row < 0 || row >= m_table->rowCount())
        return QColor();  // invalid
    QTableWidgetItem *item = m_table->item(row, 1);  // State column
    if (item == nullptr)
        return QColor();  // invalid
    return item->background().color();
}

QWidget *DashboardWindow::rowStateBadge(int row) const
{
    if (row < 0 || row >= m_table->rowCount())
        return nullptr;
    return m_table->cellWidget(row, 1);  // State column
}

// Sprint 22 (C5): rollout-stage grouping.
void DashboardWindow::setStages(const QVector<RolloutStage> &stages)
{
    m_stages = stages;
}

int DashboardWindow::stageCount() const
{
    return m_stageRow.size();
}

QString DashboardWindow::stageHeaderText(int stageIndex) const
{
    if (stageIndex < 0 || stageIndex >= m_stageRow.size())
        return QString();
    QTableWidgetItem *item = m_table->item(m_stageRow[stageIndex], 0);
    return item != nullptr ? item->text() : QString();
}

int DashboardWindow::stageProgress(int stageIndex) const
{
    if (stageIndex < 0 || stageIndex >= m_stageProgress.size())
        return -1;
    return m_stageProgress[stageIndex];
}

int DashboardWindow::stageEndpointCount(int stageIndex) const
{
    if (stageIndex < 0 || stageIndex >= m_stageEndpointCount.size())
        return -1;
    return m_stageEndpointCount[stageIndex];
}

int DashboardWindow::stageRow(int stageIndex) const
{
    if (stageIndex < 0 || stageIndex >= m_stageRow.size())
        return -1;
    return m_stageRow[stageIndex];
}

int DashboardWindow::endpointStageIndex(int row) const
{
    if (row < 0 || row >= m_endpointStage.size())
        return -1;
    return m_endpointStage[row];
}

// Graceful shutdown: stop the polling timer before the window is destroyed so
// no further network requests are issued after close.
void DashboardWindow::closeEvent(QCloseEvent *event)
{
    PATCHORCH_LOG_INFO(QStringLiteral("Dashboard shutting down; stopping poll timer."));
    m_timer.stop();
    event->accept();
}

void DashboardWindow::refreshNow()
{
    onPollTick();
}

void DashboardWindow::buildUi()
{
    // Sprint 21 (C4): fleet summary panel docked above the endpoint table.
    // It aggregates counts by state and the total, and updates whenever the
    // endpoint data changes (see populateTable).
    m_summary = new FleetSummaryPanel(this);
    auto *summaryDock = new QDockWidget(QStringLiteral("Fleet Summary"), this);
    summaryDock->setObjectName(QStringLiteral("fleetSummaryDock"));
    summaryDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    summaryDock->setWidget(m_summary);
    addDockWidget(Qt::TopDockWidgetArea, summaryDock);

    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Endpoint ID"), QStringLiteral("State"), QStringLiteral("Progress")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    setCentralWidget(m_table);

    statusBar()->showMessage(QStringLiteral("Connecting to %1 ...").arg(m_baseUrl));
}

void DashboardWindow::ensureSchedule()
{
    // POST /api/schedules ensures the schedule exists (the API creates or
    // overwrites and returns 201). On success we begin polling.
    QJsonObject body;
    body["id"] = m_scheduleId;
    body["package"] = QStringLiteral("pkg-v2");
    body["group_id"] = QStringLiteral("grp-1");

    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/api/schedules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    QNetworkReply *reply = m_net.post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCreateReply(reply);
    });
}

void DashboardWindow::onCreateReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        PATCHORCH_LOG_ERROR(QStringLiteral("Failed to create schedule: %1")
                                .arg(reply->errorString()));
        setStatusMessage(QStringLiteral("Failed to create schedule: %1")
                             .arg(reply->errorString()));
        return;
    }
    m_scheduleReady = true;
    PATCHORCH_LOG_INFO(QStringLiteral("Schedule %1 ready").arg(m_scheduleId));
    setStatusMessage(QStringLiteral("Schedule %1 ready").arg(m_scheduleId));
    pollSimulate();
    pollStatus();
    startStatusStream();
}

void DashboardWindow::startStatusStream()
{
    // Open the B5 SSE stream. It emits one event per live state change
    // (pause/resume/rollback/tick) plus a baseline event on open. Each event
    // carries "status" and "endpoints", which we use to re-render immediately.
    QNetworkRequest request(
        QUrl(m_baseUrl + QStringLiteral("/api/schedules/") + m_scheduleId +
             QStringLiteral("/status/stream")));
    QNetworkReply *reply = m_net.get(request);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        onStreamReadyRead(reply);
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStreamFinished(reply);
    });
    m_streamReply = reply;
    PATCHORCH_LOG_INFO(QStringLiteral("Status stream opened for %1").arg(m_scheduleId));
}

void DashboardWindow::onStreamReadyRead(QNetworkReply *reply)
{
    m_streamBuffer += reply->readAll();

    // SSE events are terminated by a blank line ("\n\n"). Process each
    // complete event as it arrives.
    int pos;
    while ((pos = m_streamBuffer.indexOf("\n\n")) != -1) {
        const QByteArray block = m_streamBuffer.left(pos);
        m_streamBuffer.remove(0, pos + 2);

        const QList<QByteArray> lines = block.split('\n');
        for (const QByteArray &line : lines) {
            if (!line.startsWith("data:"))
                continue;
            const QByteArray payload = line.mid(5).trimmed();
            const QJsonDocument doc = QJsonDocument::fromJson(payload);
            if (doc.isObject()) {
                handleStreamEvent(doc.object());
            }
        }
    }
}

void DashboardWindow::onStreamFinished(QNetworkReply *reply)
{
    // The stream ended (server closed or network error). Flush any remaining
    // buffered data, then drop the reply. The poll timer continues to provide
    // updates, so the dashboard keeps working without the stream.
    if (m_streamReply == reply)
        m_streamReply = nullptr;
    reply->deleteLater();
    PATCHORCH_LOG_WARN(QStringLiteral("Status stream closed for %1").arg(m_scheduleId));
}

void DashboardWindow::handleStreamEvent(const QJsonObject &event)
{
    // Re-render the endpoint table immediately from the event payload.
    const QJsonArray endpoints = event.value(QStringLiteral("endpoints")).toArray();
    if (!endpoints.isEmpty()) {
        populateTable(endpoints);
    }

    // Reflect the derived schedule status into the shared context and status bar.
    const QString status = event.value(QStringLiteral("status")).toString();
    if (!status.isEmpty()) {
        if (m_context != nullptr)
            m_context->setRolloutState(status);
        setStatusMessage(QStringLiteral("Schedule %1 — status: %2").arg(m_scheduleId, status));
    }

    PATCHORCH_LOG_DEBUG(QStringLiteral("Stream event: status=%1 endpoints=%2")
                            .arg(status).arg(endpoints.size()));
}

void DashboardWindow::onPollTick()
{
    if (!m_scheduleReady) {
        return;
    }
    pollSimulate();
    pollStatus();
}

void DashboardWindow::pollSimulate()
{
    QJsonObject body;
    body["seed"] = kDefaultSeed;
    body["endpoints"] = kDefaultEndpoints;

    QNetworkRequest request(
        QUrl(m_baseUrl + QStringLiteral("/api/schedules/") + m_scheduleId +
             QStringLiteral("/simulate")));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    QNetworkReply *reply = m_net.post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSimulateReply(reply);
    });
}

void DashboardWindow::onSimulateReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        PATCHORCH_LOG_ERROR(QStringLiteral("Simulate failed: %1").arg(reply->errorString()));
        setStatusMessage(QStringLiteral("Simulate failed: %1").arg(reply->errorString()));
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject root = doc.object();
    const QJsonArray endpoints = root.value(QStringLiteral("endpoints")).toArray();
    PATCHORCH_LOG_DEBUG(QStringLiteral("Simulate returned %1 endpoints").arg(endpoints.size()));
    populateTable(endpoints);
}

void DashboardWindow::pollStatus()
{
    QNetworkRequest request(
        QUrl(m_baseUrl + QStringLiteral("/api/schedules/") + m_scheduleId +
             QStringLiteral("/status")));
    QNetworkReply *reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStatusReply(reply);
    });
}

void DashboardWindow::onStatusReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        PATCHORCH_LOG_WARN(QStringLiteral("Status poll failed: %1").arg(reply->errorString()));
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject root = doc.object();
    const QString status =
        root.value(QStringLiteral("status")).toString(QStringLiteral("unknown"));
    if (m_context != nullptr)
        m_context->setRolloutState(status);
    PATCHORCH_LOG_DEBUG(QStringLiteral("Schedule %1 status: %2").arg(m_scheduleId, status));
    setStatusMessage(QStringLiteral("Schedule %1 — status: %2").arg(m_scheduleId, status));
}

void DashboardWindow::populateTable(const QJsonArray &endpoints)
{
    // Sprint 22 (C5): when rollout stages are set, group endpoints by stage
    // with stage headers and per-stage progress. Otherwise keep the original
    // flat table so P8/C1/C2/C3/C4 behavior is unchanged.
    if (!m_stages.isEmpty()) {
        populateGrouped(endpoints);
        return;
    }
    populateFlat(endpoints);
}

void DashboardWindow::populateFlat(const QJsonArray &endpoints)
{
    const int n = endpoints.size();

    // Shrink the per-endpoint bar vector first, detaching removed bars from
    // the table so they are not double-deleted by the table and the vector.
    if (n < m_progressBars.size()) {
        for (int row = n; row < m_progressBars.size(); ++row) {
            m_table->setCellWidget(row, 2, nullptr);
        }
    }
    m_progressBars.resize(n);
    m_table->setRowCount(n);

    // Sprint 21 (C4): keep the fleet summary in sync with the latest data.
    if (m_summary != nullptr)
        m_summary->setEndpoints(endpoints);

    for (int row = 0; row < n; ++row) {
        const QJsonObject ep = endpoints.at(row).toObject();
        const QString id = ep.value(QStringLiteral("id")).toString();
        const QString state = ep.value(QStringLiteral("state")).toString();
        const double progress = ep.value(QStringLiteral("progress")).toDouble();

        auto *idItem = new QTableWidgetItem(id);
        auto *stateItem = new QTableWidgetItem(state);
        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, stateItem);

        // Sprint 19 (C2): apply the state->color mapping to the state cell
        // (badge). The color comes from the single source of truth so the same
        // state always produces the same color across rows.
        const QColor stateColor = colorForState(state);
        stateItem->setBackground(stateColor);
        stateItem->setForeground(QColor(Qt::white));

        // Sprint 20 (C3): render the state with the reusable StateBadge widget
        // in place of inline color coding. The badge shows the color-coded
        // state with an icon and label. The underlying state item is kept so
        // the existing C2 test accessors (cell text / background color) keep
        // working.
        auto *badge = new StateBadge(state, m_table);
        m_table->setCellWidget(row, 1, badge);

        // Sprint 18 (C1): animated progress bar per endpoint. Reuse the bar so
        // it animates from its current value to the new target instead of
        // resetting to zero on every refresh.
        AnimatedProgressBar *bar = m_progressBars[row];
        if (bar == nullptr) {
            bar = new AnimatedProgressBar(m_table);
            m_progressBars[row] = bar;
            m_table->setCellWidget(row, 2, bar);
        }
        bar->setTarget(static_cast<int>(progress));
    }

    // Sprint 22 (C5): in flat mode there is no stage grouping.
    m_endpointStage.clear();
}

void DashboardWindow::populateGrouped(const QJsonArray &endpoints)
{
    // Bucket endpoints by stage. An endpoint belongs to a stage if its
    // group_id is listed in that stage's group ids. Endpoints that match no
    // stage go into an implicit "Ungrouped" group rendered last.
    QVector<QList<int>> buckets(m_stages.size());
    QList<int> ungrouped;
    for (int i = 0; i < endpoints.size(); ++i) {
        const QJsonObject ep = endpoints.at(i).toObject();
        const QString groupId = ep.value(QStringLiteral("group_id")).toString();
        int matched = -1;
        for (int s = 0; s < m_stages.size(); ++s) {
            if (m_stages[s].groupIds.contains(groupId)) {
                matched = s;
                break;
            }
        }
        if (matched >= 0)
            buckets[matched].append(i);
        else
            ungrouped.append(i);
    }

    // Ordered list of groups to render (stages in order, then ungrouped).
    QVector<QList<int>> groups;
    QVector<QString> groupNames;
    for (int s = 0; s < m_stages.size(); ++s) {
        groups.append(buckets[s]);
        groupNames.append(m_stages[s].id);
    }
    if (!ungrouped.isEmpty()) {
        groups.append(ungrouped);
        groupNames.append(QStringLiteral("Ungrouped"));
    }

    // Total rows = one header per group + one row per endpoint.
    int totalRows = 0;
    for (const auto &g : groups)
        totalRows += 1 + g.size();

    // Detach any previously placed bars so they are not double-deleted.
    for (int row = 0; row < m_progressBars.size(); ++row)
        m_table->setCellWidget(row, 2, nullptr);
    m_progressBars.clear();
    m_progressBars.resize(totalRows);
    m_table->setRowCount(totalRows);

    // Keep the fleet summary in sync with the latest data.
    if (m_summary != nullptr)
        m_summary->setEndpoints(endpoints);

    m_stageRow.clear();
    m_stageEndpointCount.clear();
    m_stageProgress.clear();
    m_endpointStage.clear();
    m_endpointStage.resize(totalRows, -1);

    int row = 0;
    for (int g = 0; g < groups.size(); ++g) {
        const int count = groups[g].size();

        // Per-stage progress = average progress of the stage's endpoints.
        int progress = 0;
        if (count > 0) {
            int sum = 0;
            for (int idx : groups[g])
                sum += static_cast<int>(
                    endpoints.at(idx).toObject()
                        .value(QStringLiteral("progress")).toDouble());
            progress = sum / count;
        }

        m_stageRow.append(row);
        m_stageEndpointCount.append(count);
        m_stageProgress.append(progress);

        // Stage header spanning all columns.
        auto *headerItem = new QTableWidgetItem(
            QStringLiteral("%1 — %2 endpoints — Progress %3%")
                .arg(groupNames[g]).arg(count).arg(progress));
        headerItem->setBackground(QColor(0x33, 0x33, 0x33));
        headerItem->setForeground(QColor(Qt::white));
        headerItem->setFlags(headerItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 0, headerItem);
        m_table->setSpan(row, 0, 1, 3);
        ++row;

        // Endpoint rows for this stage.
        for (int idx : groups[g]) {
            const QJsonObject ep = endpoints.at(idx).toObject();
            const QString id = ep.value(QStringLiteral("id")).toString();
            const QString state = ep.value(QStringLiteral("state")).toString();
            const double progressVal = ep.value(QStringLiteral("progress")).toDouble();

            auto *idItem = new QTableWidgetItem(id);
            auto *stateItem = new QTableWidgetItem(state);
            m_table->setItem(row, 0, idItem);
            m_table->setItem(row, 1, stateItem);

            const QColor stateColor = colorForState(state);
            stateItem->setBackground(stateColor);
            stateItem->setForeground(QColor(Qt::white));

            auto *badge = new StateBadge(state, m_table);
            m_table->setCellWidget(row, 1, badge);

            auto *bar = new AnimatedProgressBar(m_table);
            m_progressBars[row] = bar;
            m_table->setCellWidget(row, 2, bar);
            bar->setTarget(static_cast<int>(progressVal));

            m_endpointStage[row] = g;
            ++row;
        }
    }
}

void DashboardWindow::setStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
