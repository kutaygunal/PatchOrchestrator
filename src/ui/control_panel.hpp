// PatchOrchestrator — Phase 10 control-actions UI.
//
// A QMainWindow with a schedule-id field and Schedule / Pause / Resume /
// Rollback buttons wired to the .NET API boundary (Phase 7). Each control
// action shows a confirmation dialog before sending the request, and the
// API response (success or error) is shown in a status label. The current
// schedule status is fetched from GET /api/schedules/{id}/status.

#ifndef PATCHORCHESTRATOR_UI_CONTROL_PANEL_HPP
#define PATCHORCHESTRATOR_UI_CONTROL_PANEL_HPP

#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>

#include "failure_rate_control.hpp"
#include "fleet_size_control.hpp"
#include "seed_control.hpp"

class QLabel;
class QLineEdit;
class QNetworkReply;
class QPushButton;
class DemoAppContext;
class ScenarioSelector;

class ControlPanelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlPanelWindow(QWidget *parent = nullptr);

    // Sprint 3 (A3): bind this panel to the shared app context. The panel
    // reads/writes schedule id, API base URL, and rollout state through the
    // context and reacts to its change signals. Passing nullptr (the default)
    // keeps the standalone behaviour with the panel's own local state.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Test/automation hook: set the schedule-id field text, which propagates
    // to the shared context (if bound).
    void setScheduleIdText(const QString &id);

    // Sprint 17 (B7): process the result of a control action and display the
    // before/after state diff plus a visible confirmation that the engine
    // actually changed state. Public so tests can simulate an action result
    // without a live server.
    void handleActionResult(const QString &before, const QString &after);

    // Test accessors for the before/after diff and confirmation labels.
    QString diffText() const;
    QString confirmationText() const;
    QString lastKnownState() const { return m_lastKnownState; }

    // Sprint 25 (D1): the fleet-size config control embedded in this panel.
    FleetSizeControl *fleetSizeControl() const { return m_fleetSize; }

    // Sprint 26 (D2): the failure-rate config control embedded in this panel.
    FailureRateControl *failureRateControl() const { return m_failureRate; }

    // Sprint 27 (D3): the seed config control embedded in this panel.
    SeedControl *seedControl() const { return m_seed; }

    // Sprint 29 (D5): the scenario selector embedded in this panel.
    ScenarioSelector *scenarioSelector() const { return m_scenario; }

    // Sprint 30 (D6): validate the current config before starting a rollout.
    // If invalid, the action is blocked and an inline error is shown near the
    // config controls. Public so tests can read the resulting error state.
    bool validateConfig();

    // Phase 2 (P2): the JSON body sent by onSchedule() in the POST
    // /api/schedules request. It carries id, package, group_id plus the fleet
    // configuration. When a DemoAppContext is bound, fleetSize, failureRate,
    // and seed come from the context; otherwise they fall back to the current
    // values of the embedded config controls. onSchedule() sends exactly this
    // payload. Public so tests can inspect the request body without a server.
    QJsonObject schedulePayload() const;

    // Test accessor for the Schedule button (object name "scheduleButton").
    QPushButton *scheduleButton() const { return m_scheduleButton; }

    // Test accessors for the config-validation inline error label and the
    // current status text.
    QLabel *validationLabel() const { return m_validationLabel; }
    QString validationText() const;
    QString statusText() const;

private slots:
    void onSchedule();
    void onPause();
    void onResume();
    void onRollback();
    void onRefreshStatus();
    void onReply(QNetworkReply *reply);

private:
    void buildUi();
    void setStatusMessage(const QString &message);
    void sendAction(const QString &path, const QString &verb, const QJsonObject &body);
    void sendStatusRequest();
    QString scheduleId() const;

    QLineEdit *m_scheduleId;
    QPushButton *m_scheduleButton;
    QPushButton *m_pauseButton;
    QPushButton *m_resumeButton;
    QPushButton *m_rollbackButton;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;
    QLabel *m_diffLabel;
    QLabel *m_confirmationLabel;
    QLabel *m_validationLabel;
    FleetSizeControl *m_fleetSize;
    FailureRateControl *m_failureRate;
    SeedControl *m_seed;
    ScenarioSelector *m_scenario;

    QNetworkAccessManager m_net;
    QString m_baseUrl;
    DemoAppContext *m_context;

    // Sprint 17 (B7): last known engine state (used as the "before" side of
    // the diff) and the state captured just before a control action is sent.
    QString m_lastKnownState;
    QString m_beforeState;
};

#endif // PATCHORCHESTRATOR_UI_CONTROL_PANEL_HPP
