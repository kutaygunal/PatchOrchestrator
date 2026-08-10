// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// Polls the .NET API boundary (Phase 7) and renders simulated endpoints and
// their patch status in a table. The API base URL and schedule id are
// configurable via the PATCHORCH_API_URL and PATCHORCH_SCHEDULE_ID env vars.

#include "dashboard.hpp"
#include "demo_app_context.hpp"
#include "log.hpp"

#include <QCloseEvent>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

DashboardWindow::DashboardWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_table(nullptr)
    , m_baseUrl(envOr("PATCHORCH_API_URL", QStringLiteral("http://localhost:5000")))
    , m_scheduleId(envOr("PATCHORCH_SCHEDULE_ID", QStringLiteral("sch-1")))
    , m_scheduleReady(false)
    , m_context(nullptr)
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
    m_table->setRowCount(endpoints.size());
    for (int row = 0; row < endpoints.size(); ++row) {
        const QJsonObject ep = endpoints.at(row).toObject();
        const QString id = ep.value(QStringLiteral("id")).toString();
        const QString state = ep.value(QStringLiteral("state")).toString();
        const double progress = ep.value(QStringLiteral("progress")).toDouble();

        auto *idItem = new QTableWidgetItem(id);
        auto *stateItem = new QTableWidgetItem(state);
        // Engine reports progress as a float in [0.0, 100.0] (already a
        // percentage). Display it directly; do NOT multiply by 100 again.
        auto *progressItem = new QTableWidgetItem(
            QStringLiteral("%1%").arg(static_cast<int>(progress)));

        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, stateItem);
        m_table->setItem(row, 2, progressItem);
    }
}

void DashboardWindow::setStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
