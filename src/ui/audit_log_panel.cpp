// PatchOrchestrator — Sprint 35 (E4) audit log panel implementation.

#include "audit_log_panel.hpp"

#include "timestamp_format.hpp"

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

const char *const kHeaders[] = {
    "Action", "Target", "Timestamp", "Result",
};

} // namespace

AuditLogPanel::AuditLogPanel(QWidget *parent)
    : QWidget(parent)
    , m_table(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_table = new QTableWidget(0, ColumnCount, this);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Action"), QStringLiteral("Target"),
         QStringLiteral("Timestamp"), QStringLiteral("Result")});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_table);
}

void AuditLogPanel::setLog(const QList<AuditLogEntry> &entries)
{
    m_table->setRowCount(0);
    for (const AuditLogEntry &entry : entries)
        appendRow(entry);
}

void AuditLogPanel::appendEntry(const AuditLogEntry &entry)
{
    appendRow(entry);
}

int AuditLogPanel::rowCount() const
{
    return m_table->rowCount();
}

QString AuditLogPanel::cellText(int row, int column) const
{
    if (row < 0 || row >= m_table->rowCount() ||
        column < 0 || column >= ColumnCount)
        return QString();
    QTableWidgetItem *item = m_table->item(row, column);
    return item != nullptr ? item->text() : QString();
}

void AuditLogPanel::appendRow(const AuditLogEntry &entry)
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);

    // The timestamp column shows the ISO-8601 value formatted to a readable
    // local-time string. The underlying entry's raw timestamp is kept intact;
    // we only format it for display.
    const QString values[] = {
        entry.action, entry.target, formatTimestampLocal(entry.timestamp), entry.result,
    };
    for (int col = 0; col < ColumnCount; ++col) {
        auto *item = new QTableWidgetItem(values[col]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, col, item);
    }
}
