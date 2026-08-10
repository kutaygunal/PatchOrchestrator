// PatchOrchestrator — Phase 10 control-actions UI.
//
// Wires Schedule / Pause / Resume / Rollback buttons to the .NET API boundary
// (Phase 7). Every control action is confirmed with a dialog before the
// request is sent, and the API response (success or error) is shown in a
// status label. The current schedule status is polled from
// GET /api/schedules/{id}/status. The API base URL is configurable via the
// PATCHORCH_API_URL env var (default http://localhost:5000).

#include "control_panel.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QUrl>

#include <cstdlib>

namespace {

QString envOr(const char *name, const QString &fallback)
{
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0') {
        return fallback;
    }
    return QString::fromUtf8(value);
}

QLineEdit *makeLineEdit(const QString &placeholder)
{
    auto *edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    return edit;
}

} // namespace

ControlPanelWindow::ControlPanelWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scheduleId(nullptr)
    , m_scheduleButton(nullptr)
    , m_pauseButton(nullptr)
    , m_resumeButton(nullptr)
    , m_rollbackButton(nullptr)
    , m_refreshButton(nullptr)
    , m_statusLabel(nullptr)
    , m_baseUrl(envOr("PATCHORCH_API_URL", QStringLiteral("http://localhost:5000")))
{
    setWindowTitle(QStringLiteral("PatchOrchestrator — Control Panel"));
    resize(560, 360);

    buildUi();
    setStatusMessage(QStringLiteral("API base URL: %1").arg(m_baseUrl));
}

void ControlPanelWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);

    // --- Schedule id ---
    auto *scheduleBox = new QGroupBox(QStringLiteral("Schedule"), central);
    auto *scheduleLayout = new QVBoxLayout(scheduleBox);
    m_scheduleId = makeLineEdit(QStringLiteral("e.g. sch-1"));
    scheduleLayout->addWidget(new QLabel(QStringLiteral("Schedule ID")));
    scheduleLayout->addWidget(m_scheduleId);
    root->addWidget(scheduleBox);

    // --- Control buttons ---
    auto *controlBox = new QGroupBox(QStringLiteral("Control Actions"), central);
    auto *controlLayout = new QVBoxLayout(controlBox);

    m_scheduleButton = new QPushButton(QStringLiteral("Schedule"), controlBox);
    m_pauseButton = new QPushButton(QStringLiteral("Pause"), controlBox);
    m_resumeButton = new QPushButton(QStringLiteral("Resume"), controlBox);
    m_rollbackButton = new QPushButton(QStringLiteral("Rollback"), controlBox);
    m_refreshButton = new QPushButton(QStringLiteral("Refresh Status"), controlBox);

    controlLayout->addWidget(m_scheduleButton);
    controlLayout->addWidget(m_pauseButton);
    controlLayout->addWidget(m_resumeButton);
    controlLayout->addWidget(m_rollbackButton);
    controlLayout->addWidget(m_refreshButton);
    root->addWidget(controlBox);

    // --- Status label ---
    m_statusLabel = new QLabel(QStringLiteral("No status yet."), central);
    m_statusLabel->setWordWrap(true);
    root->addWidget(m_statusLabel, 1);

    setCentralWidget(central);

    connect(m_scheduleButton, &QPushButton::clicked, this, &ControlPanelWindow::onSchedule);
    connect(m_pauseButton, &QPushButton::clicked, this, &ControlPanelWindow::onPause);
    connect(m_resumeButton, &QPushButton::clicked, this, &ControlPanelWindow::onResume);
    connect(m_rollbackButton, &QPushButton::clicked, this, &ControlPanelWindow::onRollback);
    connect(m_refreshButton, &QPushButton::clicked, this, &ControlPanelWindow::onRefreshStatus);
}

QString ControlPanelWindow::scheduleId() const
{
    return m_scheduleId->text().trimmed();
}

void ControlPanelWindow::onSchedule()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Schedule"),
        QStringLiteral("Create/overwrite schedule '%1'? This will start the rollout.")
            .arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Schedule cancelled."));
        return;
    }

    QJsonObject body;
    body["id"] = id;
    body["package"] = QStringLiteral("pkg-v2");
    body["group_id"] = QStringLiteral("grp-1");

    sendAction(QStringLiteral("/api/schedules"), QStringLiteral("POST"), body);
}

void ControlPanelWindow::onPause()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Pause"),
        QStringLiteral("Pause schedule '%1'?").arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Pause cancelled."));
        return;
    }

    sendAction(QStringLiteral("/api/schedules/") + id + QStringLiteral("/pause"),
               QStringLiteral("POST"), QJsonObject());
}

void ControlPanelWindow::onResume()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Resume"),
        QStringLiteral("Resume schedule '%1'?").arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Resume cancelled."));
        return;
    }

    sendAction(QStringLiteral("/api/schedules/") + id + QStringLiteral("/resume"),
               QStringLiteral("POST"), QJsonObject());
}

void ControlPanelWindow::onRollback()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Rollback"),
        QStringLiteral("Roll back schedule '%1'? This reverts applied patches.")
            .arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Rollback cancelled."));
        return;
    }

    sendAction(QStringLiteral("/api/schedules/") + id + QStringLiteral("/rollback"),
               QStringLiteral("POST"), QJsonObject());
}

void ControlPanelWindow::onRefreshStatus()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }
    sendStatusRequest();
}

void ControlPanelWindow::sendAction(const QString &path, const QString &verb,
                                    const QJsonObject &body)
{
    QNetworkRequest request(QUrl(m_baseUrl + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = nullptr;
    if (verb == QStringLiteral("POST")) {
        reply = m_net.post(request, QJsonDocument(body).toJson());
    } else {
        reply = m_net.get(request);
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReply(reply);
    });

    setStatusMessage(QStringLiteral("Sending %1 %2 ...").arg(verb, path));
}

void ControlPanelWindow::sendStatusRequest()
{
    const QString id = scheduleId();
    QNetworkRequest request(
        QUrl(m_baseUrl + QStringLiteral("/api/schedules/") + id + QStringLiteral("/status")));
    QNetworkReply *reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReply(reply);
    });
    setStatusMessage(QStringLiteral("Fetching status for %1 ...").arg(id));
}

void ControlPanelWindow::onReply(QNetworkReply *reply)
{
    reply->deleteLater();

    const QByteArray payload = reply->readAll();
    const QString url = reply->url().toString();

    if (reply->error() != QNetworkReply::NoError) {
        setStatusMessage(QStringLiteral("Error (%1): %2").arg(reply->error()).arg(reply->errorString()));
        return;
    }

    // If this was a status query, surface the status field prominently.
    if (url.endsWith(QStringLiteral("/status"))) {
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        const QJsonObject root = doc.object();
        const QString status =
            root.value(QStringLiteral("status")).toString(QStringLiteral("unknown"));
        setStatusMessage(QStringLiteral("Schedule %1 — status: %2")
                             .arg(scheduleId(), status));
        return;
    }

    setStatusMessage(QStringLiteral("Success (%1): %2").arg(url, QString::fromUtf8(payload)));
}

void ControlPanelWindow::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
    statusBar()->showMessage(message);
}
