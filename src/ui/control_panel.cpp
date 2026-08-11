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
#include "state_badge.hpp"
#include "window_title_bar.hpp"

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
    , m_apiPill(nullptr)
    , m_stateBadge(nullptr)
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
    resize(640, 720);

    // Frameless: this app's only window chrome is the WindowTitleBar built in
    // buildUi() (brand mark, title, API pill, minimize/close). Resizing still
    // works via the status bar's size grip (added in buildUi()), which moves
    // the window by mouse delta rather than relying on native chrome.
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    buildUi();
    setStatusMessage(QStringLiteral("API base URL: %1").arg(m_baseUrl));
}

void ControlPanelWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(14);

    // --- Title bar -----------------------------------------------------
    // This window is frameless (see the constructor), so WindowTitleBar is
    // its only chrome: brand mark, title/role, minimize/close. The active
    // API endpoint is added as trailing content — an operator running both
    // apps side by side can see at a glance which backend each is talking
    // to, instead of having to dig into a status message to find out.
    auto *titleBar = new WindowTitleBar(QStringLiteral("PatchOrchestrator"),
                                         QStringLiteral("Control Panel"), this);
    setMenuWidget(titleBar);

    m_apiPill = new QLabel(m_baseUrl, titleBar);
    m_apiPill->setObjectName(QStringLiteral("apiPill"));
    m_apiPill->setStyleSheet(QStringLiteral(
        "background: #171c25; color: #97a1b3; border: 1px solid #2a3140;"
        "border-radius: 9px; padding: 3px 10px;"));
    titleBar->trailingLayout()->addWidget(m_apiPill);

    // --- Rollout configuration ---------------------------------------------
    // One card instead of five: Schedule/Scenario/Fleet/Failure rate/Seed used
    // to each get their own bordered QGroupBox, which was mostly repeated
    // chrome around single-row controls. Grouping them under one heading
    // reads as a single coherent "what will this rollout look like" step.
    auto *configBox = new QGroupBox(QStringLiteral("Rollout Configuration"), central);
    auto *configLayout = new QVBoxLayout(configBox);
    configLayout->setSpacing(10);

    auto *scheduleRow = new QHBoxLayout;
    scheduleRow->addWidget(new QLabel(QStringLiteral("Schedule ID")));
    m_scheduleId = makeLineEdit(QStringLiteral("e.g. sch-1"));
    scheduleRow->addWidget(m_scheduleId, 1);
    configLayout->addLayout(scheduleRow);

    // Sprint 29 (D5): scenario selector — loads a preset into the config
    // controls, overriding any manually set values via the shared
    // DemoAppContext (A3).
    m_scenario = new ScenarioSelector(nullptr, configBox);
    configLayout->addWidget(m_scenario);

    // Sprint 25 (D1): fleet size.
    m_fleetSize = new FleetSizeControl(nullptr, configBox);
    configLayout->addWidget(m_fleetSize);

    // Sprint 26 (D2): failure rate.
    m_failureRate = new FailureRateControl(nullptr, configBox);
    configLayout->addWidget(m_failureRate);

    // Sprint 27 (D3): seed.
    m_seed = new SeedControl(nullptr, configBox);
    configLayout->addWidget(m_seed);

    // Sprint 30 (D6): config-validation inline error. Shown when a rollout is
    // blocked because the configured fleet size / failure rate / seed is
    // invalid. Empty and hidden when the config is valid.
    m_validationLabel = new QLabel(configBox);
    m_validationLabel->setObjectName(QStringLiteral("validationLabel"));
    m_validationLabel->setWordWrap(true);
    m_validationLabel->setStyleSheet(
        QStringLiteral("color: #ef4444; font-weight: 600;"));
    m_validationLabel->hide();
    configLayout->addWidget(m_validationLabel);

    root->addWidget(configBox);

    // --- Control actions -----------------------------------------------
    // A single row of icon-labeled buttons rather than a tall button stack.
    // Icons come from StateBadge::iconForState() — the same glyphs painted
    // into the state pills the operator sees in this panel's Activity card
    // and in the Dashboard's table — so the action that causes a transition
    // uses the exact same symbol as the state it produces.
    auto *actionsRow = new QHBoxLayout;
    actionsRow->setSpacing(8);

    m_scheduleButton = new QPushButton(
        StateBadge::iconForState(QStringLiteral("running")) + QStringLiteral("  Schedule"), central);
    m_pauseButton = new QPushButton(
        StateBadge::iconForState(QStringLiteral("paused")) + QStringLiteral("  Pause"), central);
    m_resumeButton = new QPushButton(
        StateBadge::iconForState(QStringLiteral("running")) + QStringLiteral("  Resume"), central);
    m_rollbackButton = new QPushButton(
        StateBadge::iconForState(QStringLiteral("rolled_back")) + QStringLiteral("  Rollback"), central);
    m_refreshButton = new QPushButton(QStringLiteral("⟳  Refresh"), central);

    m_scheduleButton->setObjectName(QStringLiteral("scheduleButton"));
    m_pauseButton->setObjectName(QStringLiteral("pauseButton"));
    m_resumeButton->setObjectName(QStringLiteral("resumeButton"));
    m_rollbackButton->setObjectName(QStringLiteral("rollbackButton"));
    m_refreshButton->setObjectName(QStringLiteral("refreshButton"));

    actionsRow->addWidget(m_scheduleButton, 1);
    actionsRow->addWidget(m_pauseButton, 1);
    actionsRow->addWidget(m_resumeButton, 1);
    actionsRow->addWidget(m_rollbackButton, 1);
    actionsRow->addWidget(m_refreshButton, 1);
    root->addLayout(actionsRow);

    // --- Activity ------------------------------------------------------
    // A live StateBadge (the exact widget the Dashboard renders into its
    // table's State column) replaces the old plain-text status line, so both
    // apps show the same rollout state the same way: same pill shape, same
    // color, same icon. The before/after diff and the action confirmation sit
    // beside it as supporting detail.
    auto *activityBox = new QGroupBox(QStringLiteral("Activity"), central);
    auto *activityLayout = new QHBoxLayout(activityBox);
    activityLayout->setSpacing(12);

    m_stateBadge = new StateBadge(QString(), activityBox);
    m_stateBadge->setObjectName(QStringLiteral("controlStateBadge"));
    activityLayout->addWidget(m_stateBadge, 0, Qt::AlignTop);

    auto *activityDetail = new QVBoxLayout;
    m_diffLabel = new QLabel(QStringLiteral("State diff: —"), activityBox);
    m_diffLabel->setObjectName(QStringLiteral("diffLabel"));
    m_diffLabel->setWordWrap(true);
    QFont diffFont = m_diffLabel->font();
    diffFont.setBold(true);
    m_diffLabel->setFont(diffFont);
    activityDetail->addWidget(m_diffLabel);

    m_confirmationLabel = new QLabel(QStringLiteral("No action performed yet."), activityBox);
    m_confirmationLabel->setObjectName(QStringLiteral("confirmationLabel"));
    m_confirmationLabel->setWordWrap(true);
    m_confirmationLabel->setStyleSheet(
        QStringLiteral("color: #1a7f37; font-weight: bold;"));
    activityDetail->addWidget(m_confirmationLabel);
    activityDetail->addStretch(1);

    activityLayout->addLayout(activityDetail, 1);
    root->addWidget(activityBox);

    // --- Footer status line ---------------------------------------------
    m_statusLabel = new QLabel(QStringLiteral("No status yet."), central);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #97a1b3; font-size: 12px;"));
    root->addWidget(m_statusLabel);
    root->addStretch(1);

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
    m_apiPill->setText(m_baseUrl);
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
            [this](const QString &url) {
                m_baseUrl = url;
                m_apiPill->setText(url);
            });
    connect(m_context, &DemoAppContext::scheduleIdChanged, this,
            [this](const QString &id) { m_scheduleId->setText(id); });
    connect(m_context, &DemoAppContext::rolloutStateChanged, this,
            [this](const QString &state) {
                setLastKnownState(state);
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
        setLastKnownState(status);
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

    // Schedule creation (POST /api/schedules) returns the persisted schedule
    // as JSON. Surface a plain-language summary instead of dumping the raw
    // payload into the UI, and reflect the schedule's initial status (the
    // API always starts a new schedule "running") in the Activity badge —
    // the same visual feedback pause/resume/rollback already get.
    if (url.endsWith(QStringLiteral("/api/schedules"))) {
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        const QJsonObject root = doc.object();
        const QString id = root.value(QStringLiteral("id")).toString();
        const QString status =
            root.value(QStringLiteral("status")).toString(QStringLiteral("running"));
        const int fleetSize = root.value(QStringLiteral("fleetSize")).toInt();
        const double failureRate = root.value(QStringLiteral("failureRate")).toDouble();
        const int seed = root.value(QStringLiteral("seed")).toInt();

        setLastKnownState(status);
        if (m_context != nullptr)
            m_context->setRolloutState(status);

        setStatusMessage(QStringLiteral(
            "Schedule '%1' created — %2 endpoint(s), %3% failure rate, seed %4. Rollout started.")
                .arg(id)
                .arg(fleetSize)
                .arg(qRound(failureRate * 100))
                .arg(seed));
        return;
    }

    // Fallback for any other successful response: still no raw JSON, so a
    // future endpoint added here never leaks a payload dump into the UI.
    setStatusMessage(QStringLiteral("Request to %1 succeeded.").arg(url));
}

void ControlPanelWindow::handleActionResult(const QString &before, const QString &after)
{
    setLastKnownState(after);
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

void ControlPanelWindow::setLastKnownState(const QString &state)
{
    m_lastKnownState = state;
    if (m_stateBadge != nullptr)
        m_stateBadge->setState(state);
}
