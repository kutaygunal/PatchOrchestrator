// PatchOrchestrator — Phase 10 control-actions UI.
//
// Wires Schedule / Pause / Resume / Rollback buttons to the .NET API boundary
// (Phase 7). Every control action is confirmed with a dialog before the
// request is sent, and the API response (success or error) is shown in a
// status label. The current schedule status is polled from
// GET /api/schedules/{id}/status. The API base URL is configurable via the
// PATCHORCH_API_URL env var (default http://localhost:5000).

#include "control_panel.hpp"
#include "config_validator.hpp"
#include "demo_app_context.hpp"
#include "failure_rate_control.hpp"
#include "fleet_size_control.hpp"
#include "log.hpp"
#include "seed_control.hpp"
#include "scenario_selector.hpp"

#include <QDoubleSpinBox>
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
#include <QSpinBox>
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
    , m_diffLabel(nullptr)
    , m_confirmationLabel(nullptr)
    , m_validationLabel(nullptr)
    , m_fleetSize(nullptr)
    , m_failureRate(nullptr)
    , m_seed(nullptr)
    , m_scenario(nullptr)
    , m_baseUrl(envOr("PATCHORCH_API_URL", QStringLiteral("http://localhost:5000")))
    , m_context(nullptr)
    , m_lastKnownState()
    , m_beforeState()
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

    // --- Sprint 29 (D5): scenario selector ---
    // A dropdown that loads a preset scenario into the config controls,
    // overriding any manually set values via the shared DemoAppContext (A3).
    auto *scenarioBox = new QGroupBox(QStringLiteral("Scenario"), central);
    auto *scenarioLayout = new QVBoxLayout(scenarioBox);
    m_scenario = new ScenarioSelector(nullptr, scenarioBox);
    scenarioLayout->addWidget(m_scenario);
    root->addWidget(scenarioBox);

    // --- Sprint 25 (D1): fleet size config ---
    // A spin box that sets the number of endpoints in the fleet before
    // simulation, storing the value in the shared DemoAppContext (A3).
    auto *fleetBox = new QGroupBox(QStringLiteral("Fleet"), central);
    auto *fleetLayout = new QVBoxLayout(fleetBox);
    m_fleetSize = new FleetSizeControl(nullptr, fleetBox);
    fleetLayout->addWidget(m_fleetSize);
    root->addWidget(fleetBox);

    // --- Sprint 26 (D2): failure rate config ---
    // A slider/spin box that sets the per-endpoint failure rate (0.0–1.0),
    // storing the value in the shared DemoAppContext (A3).
    auto *failureBox = new QGroupBox(QStringLiteral("Failure Rate"), central);
    auto *failureLayout = new QVBoxLayout(failureBox);
    m_failureRate = new FailureRateControl(nullptr, failureBox);
    failureLayout->addWidget(m_failureRate);
    root->addWidget(failureBox);

    // --- Sprint 27 (D3): seed config ---
    // A spin box that sets the deterministic seed for reproducible demos,
    // storing the value in the shared DemoAppContext (A3).
    auto *seedBox = new QGroupBox(QStringLiteral("Seed"), central);
    auto *seedLayout = new QVBoxLayout(seedBox);
    m_seed = new SeedControl(nullptr, seedBox);
    seedLayout->addWidget(m_seed);
    root->addWidget(seedBox);

    // --- Sprint 30 (D6): config-validation inline error ---
    // Shown near the config controls when a rollout is blocked because the
    // configured fleet size / failure rate / seed is invalid. Empty and hidden
    // when the config is valid.
    m_validationLabel = new QLabel(central);
    m_validationLabel->setObjectName(QStringLiteral("validationLabel"));
    m_validationLabel->setWordWrap(true);
    m_validationLabel->setStyleSheet(
        QStringLiteral("color: #cf222e; font-weight: bold;"));
    m_validationLabel->hide();
    root->addWidget(m_validationLabel);

    // --- Control buttons ---
    auto *controlBox = new QGroupBox(QStringLiteral("Control Actions"), central);
    auto *controlLayout = new QVBoxLayout(controlBox);

    m_scheduleButton = new QPushButton(QStringLiteral("Schedule"), controlBox);
    m_pauseButton = new QPushButton(QStringLiteral("Pause"), controlBox);
    m_resumeButton = new QPushButton(QStringLiteral("Resume"), controlBox);
    m_rollbackButton = new QPushButton(QStringLiteral("Rollback"), controlBox);
    m_refreshButton = new QPushButton(QStringLiteral("Refresh Status"), controlBox);

    m_scheduleButton->setObjectName(QStringLiteral("scheduleButton"));
    m_pauseButton->setObjectName(QStringLiteral("pauseButton"));
    m_resumeButton->setObjectName(QStringLiteral("resumeButton"));
    m_rollbackButton->setObjectName(QStringLiteral("rollbackButton"));
    m_refreshButton->setObjectName(QStringLiteral("refreshButton"));

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

    // --- Sprint 17 (B7): before/after state diff + confirmation ---
    m_diffLabel = new QLabel(QStringLiteral("State diff: —"), central);
    m_diffLabel->setObjectName(QStringLiteral("diffLabel"));
    m_diffLabel->setWordWrap(true);
    QFont diffFont = m_diffLabel->font();
    diffFont.setBold(true);
    m_diffLabel->setFont(diffFont);
    root->addWidget(m_diffLabel);

    m_confirmationLabel = new QLabel(QStringLiteral("No action performed yet."), central);
    m_confirmationLabel->setObjectName(QStringLiteral("confirmationLabel"));
    m_confirmationLabel->setWordWrap(true);
    m_confirmationLabel->setStyleSheet(
        QStringLiteral("color: #1a7f37; font-weight: bold;"));
    root->addWidget(m_confirmationLabel);

    setCentralWidget(central);

    connect(m_scheduleButton, &QPushButton::clicked, this, &ControlPanelWindow::onSchedule);
    connect(m_pauseButton, &QPushButton::clicked, this, &ControlPanelWindow::onPause);
    connect(m_resumeButton, &QPushButton::clicked, this, &ControlPanelWindow::onResume);
    connect(m_rollbackButton, &QPushButton::clicked, this, &ControlPanelWindow::onRollback);
    connect(m_refreshButton, &QPushButton::clicked, this, &ControlPanelWindow::onRefreshStatus);

    // Sprint 30 (D6): keep the inline error in sync with the config controls so
    // an error clears as soon as the offending value is corrected.
    connect(m_fleetSize->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int) { validateConfig(); });
    connect(m_failureRate->spinBox(),
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double) { validateConfig(); });
    connect(m_seed->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int) { validateConfig(); });
}

void ControlPanelWindow::setContext(DemoAppContext *context)
{
    m_context = context;
    if (m_context == nullptr)
        return;

    // Adopt the shared values as the panel's working state.
    m_baseUrl = m_context->apiBaseUrl();
    m_scheduleId->setText(m_context->scheduleId());
    setStatusMessage(QStringLiteral("API base URL: %1").arg(m_baseUrl));

    // Sprint 25 (D1): bind the fleet-size control to the shared context.
    m_fleetSize->setContext(m_context);

    // Sprint 26 (D2): bind the failure-rate control to the shared context.
    m_failureRate->setContext(m_context);

    // Sprint 27 (D3): bind the seed control to the shared context.
    m_seed->setContext(m_context);

    // Sprint 29 (D5): bind the scenario selector to the shared context so
    // selecting a preset populates the other controls.
    m_scenario->setContext(m_context);

    // Propagate shared-state changes into this panel.
    connect(m_context, &DemoAppContext::apiBaseUrlChanged, this,
            [this](const QString &url) { m_baseUrl = url; });
    connect(m_context, &DemoAppContext::scheduleIdChanged, this,
            [this](const QString &id) { m_scheduleId->setText(id); });
    connect(m_context, &DemoAppContext::rolloutStateChanged, this,
            [this](const QString &state) {
                m_lastKnownState = state;
                setStatusMessage(QStringLiteral("Schedule %1 — status: %2")
                                     .arg(m_scheduleId->text().trimmed(), state));
            });

    // Sprint 30 (D6): re-validate whenever the shared config changes so the
    // inline error appears/clears as the values are corrected.
    connect(m_context, &DemoAppContext::fleetSizeChanged, this,
            [this](int) { validateConfig(); });
    connect(m_context, &DemoAppContext::failureRateChanged, this,
            [this](double) { validateConfig(); });
    connect(m_context, &DemoAppContext::seedChanged, this,
            [this](int) { validateConfig(); });

    // Write local edits back into the shared context (change-only setters make
    // the echo from scheduleIdChanged a no-op, so there is no feedback loop).
    connect(m_scheduleId, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_context->setScheduleId(text.trimmed()); });
}

void ControlPanelWindow::setScheduleIdText(const QString &id)
{
    m_scheduleId->setText(id);
}

QString ControlPanelWindow::scheduleId() const
{
    return m_scheduleId->text().trimmed();
}

QJsonObject ControlPanelWindow::schedulePayload() const
{
    // The shared context is the single source of truth when bound; otherwise
    // fall back to the values currently held by the config controls. This
    // mirrors the source-of-truth logic used by validateConfig().
    const int fleetSize =
        m_context != nullptr ? m_context->fleetSize() : m_fleetSize->fleetSize();
    const double failureRate = m_context != nullptr
                                   ? m_context->failureRate()
                                   : m_failureRate->failureRate();
    const int seed = m_context != nullptr ? m_context->seed() : m_seed->seed();

    QJsonObject body;
    body["id"] = scheduleId();
    body["package"] = QStringLiteral("pkg-v2");
    body["group_id"] = QStringLiteral("grp-1");
    body["fleetSize"] = fleetSize;
    body["failureRate"] = failureRate;
    body["seed"] = seed;
    return body;
}

void ControlPanelWindow::onSchedule()
{
    const QString id = scheduleId();
    if (id.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID is required."));
        return;
    }

    // Sprint 30 (D6): block the rollout start if the config is invalid and show
    // an inline error. The confirmation dialog and network request are never
    // reached, so no rollout starts with invalid values.
    if (!validateConfig())
        return;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Schedule"),
        QStringLiteral("Create/overwrite schedule '%1'? This will start the rollout.")
            .arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Schedule cancelled."));
        return;
    }

    // Phase 2 (P2): the body includes id/package/group_id plus the fleet
    // configuration so the API (P1) can persist it as the shared source of
    // truth. sendAction() sends exactly this payload.
    const QJsonObject body = schedulePayload();

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

    m_beforeState = m_lastKnownState;
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

    m_beforeState = m_lastKnownState;
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

    // Sprint 30 (D6): block the rollback if the config is invalid and show an
    // inline error, matching the rollout-start behaviour.
    if (!validateConfig())
        return;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Confirm Rollback"),
        QStringLiteral("Roll back schedule '%1'? This reverts applied patches.")
            .arg(id));
    if (answer != QMessageBox::Yes) {
        setStatusMessage(QStringLiteral("Rollback cancelled."));
        return;
    }

    m_beforeState = m_lastKnownState;
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
    PATCHORCH_LOG_INFO(QStringLiteral("Sending %1 %2 to %3").arg(verb, path, m_baseUrl));
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
        PATCHORCH_LOG_ERROR(QStringLiteral("API request failed (%1): %2")
                                .arg(reply->errorString(), url));
        setStatusMessage(QStringLiteral("Error (%1): %2").arg(reply->error()).arg(reply->errorString()));
        return;
    }
    PATCHORCH_LOG_INFO(QStringLiteral("API request succeeded for %1").arg(url));

    // If this was a status query, surface the status field prominently and
    // publish it to the shared context so other panels react.
    if (url.endsWith(QStringLiteral("/status"))) {
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        const QJsonObject root = doc.object();
        const QString status =
            root.value(QStringLiteral("status")).toString(QStringLiteral("unknown"));
        m_lastKnownState = status;
        if (m_context != nullptr)
            m_context->setRolloutState(status);
        setStatusMessage(QStringLiteral("Schedule %1 — status: %2")
                             .arg(scheduleId(), status));
        return;
    }

    // Sprint 17 (B7): a control action (pause/resume/rollback) returns the new
    // engine state. Show the before/after diff and a visible confirmation.
    if (url.contains(QStringLiteral("/pause")) ||
        url.contains(QStringLiteral("/resume")) ||
        url.contains(QStringLiteral("/rollback"))) {
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        const QJsonObject root = doc.object();
        const QString after =
            root.value(QStringLiteral("status")).toString(QStringLiteral("unknown"));
        handleActionResult(m_beforeState, after);
        return;
    }

    setStatusMessage(QStringLiteral("Success (%1): %2").arg(url, QString::fromUtf8(payload)));
}

void ControlPanelWindow::handleActionResult(const QString &before, const QString &after)
{
    m_lastKnownState = after;
    if (m_context != nullptr)
        m_context->setRolloutState(after);

    // Before/after state diff.
    if (before.isEmpty()) {
        m_diffLabel->setText(QStringLiteral("State diff: %1").arg(after));
    } else {
        m_diffLabel->setText(QStringLiteral("State diff: %1 → %2").arg(before, after));
    }

    // Visible confirmation reflecting the actual new engine state.
    if (before == after) {
        m_confirmationLabel->setText(
            QStringLiteral("No state change — engine already %1.").arg(after));
        m_confirmationLabel->setStyleSheet(
            QStringLiteral("color: #9a6700; font-weight: bold;"));
    } else {
        m_confirmationLabel->setText(
            QStringLiteral("✓ Confirmed: engine %1 → %2").arg(before, after));
        m_confirmationLabel->setStyleSheet(
            QStringLiteral("color: #1a7f37; font-weight: bold;"));
    }

    setStatusMessage(QStringLiteral("Schedule %1 — status: %2")
                         .arg(scheduleId(), after));
}

QString ControlPanelWindow::diffText() const
{
    return m_diffLabel != nullptr ? m_diffLabel->text() : QString();
}

QString ControlPanelWindow::confirmationText() const
{
    return m_confirmationLabel != nullptr ? m_confirmationLabel->text() : QString();
}

QString ControlPanelWindow::validationText() const
{
    return m_validationLabel != nullptr ? m_validationLabel->text() : QString();
}

QString ControlPanelWindow::statusText() const
{
    return m_statusLabel != nullptr ? m_statusLabel->text() : QString();
}

bool ControlPanelWindow::validateConfig()
{
    // The shared context is the single source of truth when bound; otherwise
    // fall back to the values currently held by the config controls.
    const int fleetSize =
        m_context != nullptr ? m_context->fleetSize() : m_fleetSize->fleetSize();
    const double failureRate = m_context != nullptr
                                   ? m_context->failureRate()
                                   : m_failureRate->failureRate();
    const int seed = m_context != nullptr ? m_context->seed() : m_seed->seed();

    const ConfigValidator::Result result =
        ConfigValidator::validate(fleetSize, failureRate, seed);

    if (result.isValid()) {
        m_validationLabel->setText(QString());
        m_validationLabel->hide();
        return true;
    }

    // Show the first offending field's message near the config controls.
    m_validationLabel->setText(result.firstError());
    m_validationLabel->show();
    return false;
}

void ControlPanelWindow::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
    statusBar()->showMessage(message);
}
