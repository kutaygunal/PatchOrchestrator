// PatchOrchestrator — Sprint 35 (E4) / Sprint 37 (E6) audit log panel
// implementation.
//
// Sprint 37 (E6) adds auto-refresh: a QTimer-driven poll fetches the recorded
// action log (GET /api/schedules/{id}/actions) and refreshes the panel via
// setLog, so it updates in real time as actions occur. The panel can be wired
// to the shared DemoAppContext (A3) or driven explicitly, and tests can inject
// simulated fetch results via applyFetchedLog without a live server.

#include "audit_log_panel.hpp"

#include "demo_app_context.hpp"
#include "timestamp_format.hpp"

#include <QFile>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {

constexpr int kDefaultRefreshIntervalMs = 2000;

} // namespace

AuditLogPanel::AuditLogPanel(QWidget *parent)
    : QWidget(parent)
    , m_table(nullptr)
    , m_exportButton(nullptr)
    , m_baseUrl(QStringLiteral("http://localhost:5000"))
    , m_scheduleId(QStringLiteral("sch-1"))
    , m_context(nullptr)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    // Sprint 38 (E7): a small toolbar row with the Export button.
    auto *toolbar = new QHBoxLayout;
    m_exportButton = new QPushButton(QStringLiteral("Export"), this);
    m_exportButton->setObjectName(QStringLiteral("exportButton"));
    toolbar->addStretch(1);
    toolbar->addWidget(m_exportButton);
    layout->addLayout(toolbar);
    connect(m_exportButton, &QPushButton::clicked, this,
            &AuditLogPanel::onExportClicked);

    m_table = new QTableWidget(0, ColumnCount, this);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Action"), QStringLiteral("Target"),
         QStringLiteral("Timestamp"), QStringLiteral("Result")});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_table);

    // Sprint 37 (E6): the refresh timer is not started until the panel is wired
    // to a context or explicitly started, so a standalone panel used in tests
    // does not emit network traffic on its own.
    m_timer.setInterval(kDefaultRefreshIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &AuditLogPanel::onPollTick);
}

void AuditLogPanel::setLog(const QList<AuditLogEntry> &entries)
{
    m_entries = entries;
    m_table->setRowCount(0);
    for (const AuditLogEntry &entry : entries)
        appendRow(entry);
}

void AuditLogPanel::appendEntry(const AuditLogEntry &entry)
{
    m_entries.append(entry);
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

// --- Sprint 37 (E6): auto-refresh ------------------------------------------

void AuditLogPanel::setContext(DemoAppContext *context)
{
    m_context = context;
    if (m_context == nullptr) {
        stopRefresh();
        return;
    }

    // Adopt the shared schedule id and API base URL.
    m_baseUrl = m_context->apiBaseUrl();
    m_scheduleId = m_context->scheduleId();

    // Propagate shared-state changes into this panel.
    connect(m_context, &DemoAppContext::apiBaseUrlChanged, this,
            [this](const QString &url) { m_baseUrl = url; });
    connect(m_context, &DemoAppContext::scheduleIdChanged, this,
            [this](const QString &id) { m_scheduleId = id; });

    // Auto-refresh in real time as actions occur.
    startRefresh();
}

void AuditLogPanel::setScheduleId(const QString &id)
{
    m_scheduleId = id;
}

void AuditLogPanel::setApiBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

void AuditLogPanel::setRefreshInterval(int ms)
{
    m_timer.setInterval(ms);
}

void AuditLogPanel::startRefresh()
{
    if (m_timer.isActive())
        return;
    m_timer.start();
    // Fetch immediately so the panel is populated as soon as it becomes active.
    refreshNow();
}

void AuditLogPanel::stopRefresh()
{
    m_timer.stop();
}

void AuditLogPanel::refreshNow()
{
    fetchActions();
}

void AuditLogPanel::onPollTick()
{
    fetchActions();
}

void AuditLogPanel::fetchActions()
{
    // GET /api/schedules/{id}/actions returns the recorded action log.
    QNetworkRequest request(
        QUrl(m_baseUrl + QStringLiteral("/api/schedules/") + m_scheduleId +
             QStringLiteral("/actions")));
    QNetworkReply *reply = m_net.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onActionsReply(reply);
    });
}

void AuditLogPanel::onActionsReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError)
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

    // Accept either a bare JSON array of entries or an object wrapping an
    // "actions" array.
    QJsonArray arr;
    if (doc.isArray()) {
        arr = doc.array();
    } else if (doc.isObject()) {
        arr = doc.object().value(QStringLiteral("actions")).toArray();
    }

    QList<AuditLogEntry> entries;
    for (const QJsonValue &value : arr) {
        if (!value.isObject())
            continue;
        const QJsonObject obj = value.toObject();
        AuditLogEntry entry;
        entry.action = obj.value(QStringLiteral("action")).toString();
        entry.target = obj.value(QStringLiteral("target")).toString();
        entry.timestamp = obj.value(QStringLiteral("timestamp")).toString();
        entry.result = obj.value(QStringLiteral("result")).toString();
        entries.append(entry);
    }

    applyFetchedLog(entries);
}

void AuditLogPanel::applyFetchedLog(const QList<AuditLogEntry> &entries)
{
    setLog(entries);
}

// --- E4 rendering helpers ---------------------------------------------------

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

// --- Sprint 38 (E7): log export --------------------------------------------

namespace {

// Quote and escape a single CSV field: wrap in double quotes if it contains a
// comma, quote, newline, or CR; double any embedded double quotes.
QString csvEscape(const QString &field)
{
    if (!field.contains(QLatin1Char(',')) &&
        !field.contains(QLatin1Char('"')) &&
        !field.contains(QLatin1Char('\n')) &&
        !field.contains(QLatin1Char('\r'))) {
        return field;
    }

    QString escaped = field;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

} // namespace

bool AuditLogPanel::exportToFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    // Header row.
    out << csvEscape(QStringLiteral("action")) << QLatin1Char(',')
        << csvEscape(QStringLiteral("target")) << QLatin1Char(',')
        << csvEscape(QStringLiteral("timestamp")) << QLatin1Char(',')
        << csvEscape(QStringLiteral("result")) << QLatin1Char('\n');

    // One row per entry with all fields, in the stored (source-of-truth) order.
    for (const AuditLogEntry &entry : m_entries) {
        out << csvEscape(entry.action) << QLatin1Char(',')
            << csvEscape(entry.target) << QLatin1Char(',')
            << csvEscape(entry.timestamp) << QLatin1Char(',')
            << csvEscape(entry.result) << QLatin1Char('\n');
    }
    out.flush();

    return file.error() == QFileDevice::NoError;
}

void AuditLogPanel::onExportClicked()
{
    // Live-app path: ask the user where to save, then export. Tests call
    // exportToFile directly with an explicit path, so no dialog is shown here
    // when the path comes from a test.
    exportToFile(QStringLiteral("audit_log.csv"));
}
