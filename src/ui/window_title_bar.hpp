// PatchOrchestrator — shared frameless-window title bar.
//
// Both desktop apps run frameless (no OS-drawn title bar), so this widget is
// the *only* window chrome they get: brand mark, title/subtitle, an optional
// slot for app-specific content (the Control Panel's API-endpoint pill), and
// the minimize/close buttons a frameless window otherwise loses. It also
// supplies the drag-to-move behavior a native title bar would normally give
// for free, via QWindow::startSystemMove() — which still gets native Aero
// Snap on Windows, since the OS (not this widget) drives the actual move.
//
// Resizing is not handled here: both windows already have a QStatusBar, and
// QStatusBar's size grip resizes the top-level window by mouse delta rather
// than relying on native chrome, so it keeps working unchanged in frameless
// mode.
//
// Header-only (no signals/slots of its own, so no Q_OBJECT/moc needed): both
// dashboard.cpp and control_panel.cpp are compiled directly into ~30 test
// targets across the suite, and a plain #include here is far simpler than
// adding a new .cpp to every one of them.

#ifndef PATCHORCHESTRATOR_UI_WINDOW_TITLE_BAR_HPP
#define PATCHORCHESTRATOR_UI_WINDOW_TITLE_BAR_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <QWindow>

class WindowTitleBar : public QWidget
{
public:
    // title/subtitle are shown as "<bold title>  <muted subtitle>", e.g.
    // "PatchOrchestrator" / "Control Panel".
    explicit WindowTitleBar(const QString &title, const QString &subtitle,
                             QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("windowTitleBar"));
        setFixedHeight(40);

        auto *root = new QHBoxLayout(this);
        root->setContentsMargins(12, 0, 8, 0);
        root->setSpacing(8);

        auto *mark = new QLabel(this);
        mark->setFixedSize(10, 10);
        mark->setStyleSheet(QStringLiteral("background: #2dd4bf; border-radius: 5px;"));
        root->addWidget(mark);

        auto *titleLabel = new QLabel(title, this);
        titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 700;"));
        root->addWidget(titleLabel);

        if (!subtitle.isEmpty()) {
            auto *subtitleLabel = new QLabel(subtitle, this);
            subtitleLabel->setStyleSheet(QStringLiteral("color: #97a1b3;"));
            root->addWidget(subtitleLabel);
        }

        root->addStretch(1);

        // App-specific trailing content (e.g. the Control Panel's API pill)
        // is added here by the caller, ahead of the window buttons.
        m_trailingLayout = new QHBoxLayout;
        m_trailingLayout->setSpacing(8);
        root->addLayout(m_trailingLayout);

        m_minimizeButton = new QPushButton(QStringLiteral("—"), this);
        m_minimizeButton->setObjectName(QStringLiteral("windowMinimizeButton"));
        m_minimizeButton->setFixedSize(32, 28);
        root->addWidget(m_minimizeButton);

        m_closeButton = new QPushButton(QStringLiteral("✕"), this);
        m_closeButton->setObjectName(QStringLiteral("windowCloseButton"));
        m_closeButton->setFixedSize(32, 28);
        root->addWidget(m_closeButton);

        connect(m_minimizeButton, &QPushButton::clicked, this, [this]() {
            if (window() != nullptr)
                window()->showMinimized();
        });
        connect(m_closeButton, &QPushButton::clicked, this, [this]() {
            if (window() != nullptr)
                window()->close();
        });
    }

    // Layout that app-specific trailing content (e.g. the API-endpoint pill)
    // is added to, positioned after the title and before the window buttons.
    QHBoxLayout *trailingLayout() const { return m_trailingLayout; }

    QPushButton *minimizeButton() const { return m_minimizeButton; }
    QPushButton *closeButton() const { return m_closeButton; }

protected:
    // Dragging anywhere on the bar's empty background moves the window,
    // matching a native title bar. Clicks on child widgets (the trailing
    // content, the window buttons) are handled by those widgets first and
    // never reach here.
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            QWindow *handle = window() != nullptr ? window()->windowHandle() : nullptr;
            if (handle != nullptr) {
                handle->startSystemMove();
                event->accept();
                return;
            }
        }
        QWidget::mousePressEvent(event);
    }

private:
    QHBoxLayout *m_trailingLayout;
    QPushButton *m_minimizeButton;
    QPushButton *m_closeButton;
};

#endif // PATCHORCHESTRATOR_UI_WINDOW_TITLE_BAR_HPP
