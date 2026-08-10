// PatchOrchestrator — Phase 9 schedule-definition UI.
//
// Collects schedule, maintenance-window, and rollout-stage data from the user
// and POSTs it to POST /api/schedules. The API base URL is configurable via
// the PATCHORCH_API_URL env var (default http://localhost:5000).

#include "schedule_editor.hpp"
#include "demo_app_context.hpp"
#include "log.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextEdit>
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

ScheduleEditorWindow::ScheduleEditorWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scheduleId(nullptr)
    , m_package(nullptr)
    , m_groupId(nullptr)
    , m_windowId(nullptr)
    , m_windowStart(nullptr)
    , m_windowEnd(nullptr)
    , m_stageTable(nullptr)
    , m_addStageButton(nullptr)
    , m_removeStageButton(nullptr)
    , m_createButton(nullptr)
    , m_result(nullptr)
    , m_baseUrl(envOr("PATCHORCH_API_URL", QStringLiteral("http://localhost:5000")))
    , m_context(nullptr)
{
    setWindowTitle(QStringLiteral("PatchOrchestrator — Schedule Editor"));
    resize(680, 560);

    buildUi();
    setStatusMessage(QStringLiteral("API base URL: %1").arg(m_baseUrl));
}

void ScheduleEditorWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);

    // --- Schedule fields ---
    auto *scheduleBox = new QGroupBox(QStringLiteral("Schedule"), central);
    auto *scheduleLayout = new QVBoxLayout(scheduleBox);
    m_scheduleId = makeLineEdit(QStringLiteral("e.g. sch-1"));
    m_package = makeLineEdit(QStringLiteral("e.g. pkg-v2"));
    m_groupId = makeLineEdit(QStringLiteral("e.g. grp-1"));
    scheduleLayout->addWidget(new QLabel(QStringLiteral("Schedule ID")));
    scheduleLayout->addWidget(m_scheduleId);
    scheduleLayout->addWidget(new QLabel(QStringLiteral("Package")));
    scheduleLayout->addWidget(m_package);
    scheduleLayout->addWidget(new QLabel(QStringLiteral("Group ID")));
    scheduleLayout->addWidget(m_groupId);
    root->addWidget(scheduleBox);

    // --- Maintenance-window fields ---
    auto *windowBox = new QGroupBox(QStringLiteral("Maintenance Window"), central);
    auto *windowLayout = new QVBoxLayout(windowBox);
    m_windowId = makeLineEdit(QStringLiteral("e.g. mw-1"));
    m_windowStart = makeLineEdit(QStringLiteral("e.g. 2025-01-01T02:00:00Z"));
    m_windowEnd = makeLineEdit(QStringLiteral("e.g. 2025-01-01T04:00:00Z"));
    windowLayout->addWidget(new QLabel(QStringLiteral("Window ID")));
    windowLayout->addWidget(m_windowId);
    windowLayout->addWidget(new QLabel(QStringLiteral("Start (ISO-8601)")));
    windowLayout->addWidget(m_windowStart);
    windowLayout->addWidget(new QLabel(QStringLiteral("End (ISO-8601)")));
    windowLayout->addWidget(m_windowEnd);
    root->addWidget(windowBox);

    // --- Rollout-stage editor ---
    auto *stageBox = new QGroupBox(QStringLiteral("Rollout Stages"), central);
    auto *stageLayout = new QVBoxLayout(stageBox);

    m_stageTable = new QTableWidget(0, 3, stageBox);
    m_stageTable->setHorizontalHeaderLabels(
        {QStringLiteral("Stage ID"), QStringLiteral("Order"), QStringLiteral("Group IDs (comma-sep)")});
    m_stageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_stageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stageTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stageLayout->addWidget(m_stageTable);

    auto *stageButtons = new QHBoxLayout;
    m_addStageButton = new QPushButton(QStringLiteral("Add Stage"), stageBox);
    m_removeStageButton = new QPushButton(QStringLiteral("Remove Stage"), stageBox);
    stageButtons->addWidget(m_addStageButton);
    stageButtons->addWidget(m_removeStageButton);
    stageButtons->addStretch(1);
    stageLayout->addLayout(stageButtons);
    root->addWidget(stageBox);

    // --- Create + result ---
    m_createButton = new QPushButton(QStringLiteral("Create Schedule"), central);
    root->addWidget(m_createButton);

    m_result = new QTextEdit(central);
    m_result->setReadOnly(true);
    m_result->setPlaceholderText(QStringLiteral("API response will appear here."));
    root->addWidget(m_result, 1);

    setCentralWidget(central);

    connect(m_addStageButton, &QPushButton::clicked, this, &ScheduleEditorWindow::onAddStage);
    connect(m_removeStageButton, &QPushButton::clicked, this,
            &ScheduleEditorWindow::onRemoveStage);
    connect(m_createButton, &QPushButton::clicked, this,
            &ScheduleEditorWindow::onCreateSchedule);
}

void ScheduleEditorWindow::setContext(DemoAppContext *context)
{
    m_context = context;
    if (m_context == nullptr)
        return;

    // Adopt the shared schedule id and API base URL.
    m_baseUrl = m_context->apiBaseUrl();
    m_scheduleId->setText(m_context->scheduleId());
    setStatusMessage(QStringLiteral("API base URL: %1").arg(m_baseUrl));

    // Propagate shared-state changes into this panel.
    connect(m_context, &DemoAppContext::apiBaseUrlChanged, this,
            [this](const QString &url) { m_baseUrl = url; });
    connect(m_context, &DemoAppContext::scheduleIdChanged, this,
            [this](const QString &id) { m_scheduleId->setText(id); });

    // Write local edits back into the shared context (change-only setters make
    // the echo from scheduleIdChanged a no-op, so there is no feedback loop).
    connect(m_scheduleId, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_context->setScheduleId(text.trimmed()); });
}

void ScheduleEditorWindow::onAddStage()
{
    const int row = m_stageTable->rowCount();
    m_stageTable->insertRow(row);
    m_stageTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("stage-%1").arg(row + 1)));
    m_stageTable->setItem(row, 1, new QTableWidgetItem(QString::number(row + 1)));
    m_stageTable->setItem(row, 2, new QTableWidgetItem(QStringLiteral("grp-1")));
}

void ScheduleEditorWindow::onRemoveStage()
{
    const int row = m_stageTable->currentRow();
    if (row >= 0) {
        m_stageTable->removeRow(row);
    }
}

void ScheduleEditorWindow::onCreateSchedule()
{
    const QString id = m_scheduleId->text().trimmed();
    const QString package = m_package->text().trimmed();
    const QString groupId = m_groupId->text().trimmed();

    if (id.isEmpty() || package.isEmpty() || groupId.isEmpty()) {
        setStatusMessage(QStringLiteral("Schedule ID, Package, and Group ID are required."));
        return;
    }

    // Build the request body. The API contract consumes id/package/group_id;
    // the maintenance window and rollout stages are included for completeness.
    QJsonObject body;
    body["id"] = id;
    body["package"] = package;
    body["group_id"] = groupId;

    QJsonObject window;
    window["id"] = m_windowId->text().trimmed();
    window["start"] = m_windowStart->text().trimmed();
    window["end"] = m_windowEnd->text().trimmed();
    body["window"] = window;

    QJsonArray stages;
    for (int row = 0; row < m_stageTable->rowCount(); ++row) {
        QJsonObject stage;
        stage["id"] = m_stageTable->item(row, 0)
                          ? m_stageTable->item(row, 0)->text().trimmed()
                          : QString();
        stage["order"] = m_stageTable->item(row, 1)
                             ? m_stageTable->item(row, 1)->text().trimmed().toInt()
                             : 0;

        QJsonArray groupIds;
        const QString groups = m_stageTable->item(row, 2)
                                   ? m_stageTable->item(row, 2)->text().trimmed()
                                   : QString();
        const QStringList parts = groups.split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QString &g : parts) {
            groupIds.append(g.trimmed());
        }
        stage["group_ids"] = groupIds;
        stages.append(stage);
    }
    body["stages"] = stages;

    QNetworkRequest request(QUrl(m_baseUrl + QStringLiteral("/api/schedules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = m_net.post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCreateReply(reply);
    });

    setStatusMessage(QStringLiteral("Creating schedule %1 ...").arg(id));
    m_createButton->setEnabled(false);
}

void ScheduleEditorWindow::onCreateReply(QNetworkReply *reply)
{
    reply->deleteLater();
    m_createButton->setEnabled(true);

    const QByteArray payload = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        const QString message = QStringLiteral("Error (%1): %2")
                                    .arg(reply->error())
                                    .arg(reply->errorString());
        PATCHORCH_LOG_ERROR(QStringLiteral("Create schedule failed: %1").arg(message));
        m_result->setPlainText(message);
        setStatusMessage(QStringLiteral("Create schedule failed."));
        return;
    }

    PATCHORCH_LOG_INFO(QStringLiteral("Schedule created successfully."));
    m_result->setPlainText(QString::fromUtf8(payload));
    setStatusMessage(QStringLiteral("Schedule created successfully."));
}

void ScheduleEditorWindow::setStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
