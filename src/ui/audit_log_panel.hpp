// PatchOrchestrator — Sprint 35 (E4) audit log panel.
//
// A QTableWidget panel in the demo hub that displays the live operator action
// log. Each entry shows the action, target, timestamp, and result in one row.
// The panel can replace the whole log (setLog) or append new entries
// (appendEntry), so it stays in sync with the recorded action log.

#ifndef PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP
#define PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP

#include <QList>
#include <QString>
#include <QWidget>

class QTableWidget;

// A single operator action log entry (mirrors the .NET E1/E2 ActionLogEntry
// fields: action, target, timestamp, result).
struct AuditLogEntry
{
    QString action;
    QString target;
    QString timestamp;
    QString result;
};

class AuditLogPanel : public QWidget
{
    Q_OBJECT

public:
    // Display columns, in order.
    enum Column {
        ActionColumn = 0,
        TargetColumn,
        TimestampColumn,
        ResultColumn,
        ColumnCount
    };

    explicit AuditLogPanel(QWidget *parent = nullptr);

    // Replace the whole log with the given entries, clearing any prior rows.
    void setLog(const QList<AuditLogEntry> &entries);

    // Append a single new entry, rendering one more row.
    void appendEntry(const AuditLogEntry &entry);

    // Test accessors.
    QTableWidget *table() const { return m_table; }
    int rowCount() const;
    QString cellText(int row, int column) const;

private:
    void appendRow(const AuditLogEntry &entry);

    QTableWidget *m_table;
};

#endif // PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP
