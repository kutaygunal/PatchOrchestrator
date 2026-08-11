// PatchOrchestrator — Sprint 3 (A3) shared app state.
//
// A single QObject that holds the shared application state used by all demo
// panels: the active schedule id, the API base URL, and the current rollout
// state. It is the single source of truth for this state; panels read and
// write through it and react to its change signals instead of keeping their
// own copies.

#ifndef PATCHORCHESTRATOR_UI_DEMO_APP_CONTEXT_HPP
#define PATCHORCHESTRATOR_UI_DEMO_APP_CONTEXT_HPP

#include <QObject>
#include <QString>

class DemoAppContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scheduleId READ scheduleId WRITE setScheduleId NOTIFY scheduleIdChanged)
    Q_PROPERTY(QString apiBaseUrl READ apiBaseUrl WRITE setApiBaseUrl NOTIFY apiBaseUrlChanged)
    Q_PROPERTY(QString rolloutState READ rolloutState WRITE setRolloutState NOTIFY rolloutStateChanged)
    Q_PROPERTY(int fleetSize READ fleetSize WRITE setFleetSize NOTIFY fleetSizeChanged)

public:
    explicit DemoAppContext(QObject *parent = nullptr);

    QString scheduleId() const { return m_scheduleId; }
    QString apiBaseUrl() const { return m_apiBaseUrl; }
    QString rolloutState() const { return m_rolloutState; }
    int fleetSize() const { return m_fleetSize; }

public slots:
    // Setters are change-only: assigning the current value is a no-op and does
    // not emit a signal, so panels can safely echo values back without loops.
    void setScheduleId(const QString &id);
    void setApiBaseUrl(const QString &url);
    void setRolloutState(const QString &state);
    void setFleetSize(int size);

signals:
    void scheduleIdChanged(const QString &id);
    void apiBaseUrlChanged(const QString &url);
    void rolloutStateChanged(const QString &state);
    void fleetSizeChanged(int size);

private:
    QString m_scheduleId;
    QString m_apiBaseUrl;
    QString m_rolloutState;
    int m_fleetSize;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_APP_CONTEXT_HPP
