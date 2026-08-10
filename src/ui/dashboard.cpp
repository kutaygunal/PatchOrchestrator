// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// Polls the .NET API boundary (Phase 7) and renders simulated endpoints and
// their patch status in a table. The API base URL and schedule id are
// configurable via the PATCHORCH_API_URL and PATCHORCH_SCHEDULE_ID env vars.

#include "dashboard.hpp"

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
{
    setWindowTitle(QStringLiteral("PatchOrchestrator — Dashboard"));
    resize(720, 420);

    buildUi();

    connect(&m_timer, &QTimer::timeout, this, &DashboardWindow::onPollTick);
    m_timer.start(kDefaultPollIntervalMs);

    ensureSchedule();
}

void DashboardWindow::setPollIntervalMs(int ms)
{
    m_timer.setInterval(ms);
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
        setStatusMessage(QStringLiteral("Failed to create schedule: %1")
                             .arg(reply->errorString()));
        return;
    }
    m_scheduleReady = true;
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
        setStatusMessage(QStringLiteral("Simulate failed: %1").arg(reply->errorString()));
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject root = doc.object();
    const QJsonArray endpoints = root.value(QStringLiteral("endpoints")).toArray();
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
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject root = doc.object();
    const QString status =
        root.value(QStringLiteral("status")).toString(QStringLiteral("unknown"));
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
        auto *progressItem = new QTableWidgetItem(
            QStringLiteral("%1%").arg(static_cast<int>(progress * 100.0)));

        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, stateItem);
        m_table->setItem(row, 2, progressItem);
    }
}

void DashboardWindow::setStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
