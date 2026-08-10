// PatchOrchestrator — Phase 9 schedule-definition UI.
//
// A QMainWindow with forms for the schedule id, package, group id, a
// maintenance-window editor (start/end), and a rollout-stage editor (a list
// of stages, each with an id, order, and group ids). A "Create Schedule"
// button POSTs the collected data to POST /api/schedules and shows the API
// response (success/error) to the user.

#ifndef PATCHORCHESTRATOR_UI_SCHEDULE_EDITOR_HPP
#define PATCHORCHESTRATOR_UI_SCHEDULE_EDITOR_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>

class QLineEdit;
class QNetworkReply;
class QPushButton;
class QTableWidget;
class QTextEdit;

class ScheduleEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScheduleEditorWindow(QWidget *parent = nullptr);

private slots:
    void onAddStage();
    void onRemoveStage();
    void onCreateSchedule();
    void onCreateReply(QNetworkReply *reply);

private:
    void buildUi();
    void setStatusMessage(const QString &message);

    // Schedule fields.
    QLineEdit *m_scheduleId;
    QLineEdit *m_package;
    QLineEdit *m_groupId;

    // Maintenance-window fields.
    QLineEdit *m_windowId;
    QLineEdit *m_windowStart;
    QLineEdit *m_windowEnd;

    // Rollout-stage editor.
    QTableWidget *m_stageTable;
    QPushButton *m_addStageButton;
    QPushButton *m_removeStageButton;

    // Create + result.
    QPushButton *m_createButton;
    QTextEdit *m_result;

    QNetworkAccessManager m_net;
    QString m_baseUrl;
};

#endif // PATCHORCHESTRATOR_UI_SCHEDULE_EDITOR_HPP
