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

class QLabel;
class QLineEdit;
class QNetworkReply;
class QPushButton;

class ControlPanelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlPanelWindow(QWidget *parent = nullptr);

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

    QNetworkAccessManager m_net;
    QString m_baseUrl;
};

#endif // PATCHORCHESTRATOR_UI_CONTROL_PANEL_HPP
