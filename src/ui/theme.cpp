// PatchOrchestrator — shared application theme implementation.
//
// See theme.hpp for the contract. The design tokens below (colors, radii,
// spacing) are the single source of truth for the product's chrome; change
// them here and every window picks up the update.

#include "theme.hpp"

#include <QApplication>
#include <QPair>
#include <QPalette>
#include <QStyleFactory>

namespace {

// --- Design tokens -----------------------------------------------------
// A small, deliberately narrow palette: a near-black slate background, two
// elevated surface tones for cards/fields, three text tones, one brand
// accent (teal — chosen to stay clearly distinct from the semantic status
// colors StateBadge already uses for running/paused/succeeded/failed/etc.),
// and one danger tone for destructive actions (Rollback).
const char kBg[]            = "#11151c";
const char kBgElevated[]    = "#171c25";
const char kBgElevated2[]   = "#1d2330";
const char kBorder[]        = "#2a3140";
const char kText[]          = "#e7eaf0";
const char kTextMuted[]     = "#97a1b3";
const char kTextFaint[]     = "#5b6577";

const char kAccent[]        = "#2dd4bf";
const char kAccentHover[]   = "#4fe0cd";
const char kAccentPressed[] = "#1fb3a0";
const char kAccentSoft[]    = "rgba(45, 212, 191, 40)";
const char kAccentInk[]     = "#052e2b"; // text painted on top of the accent fill

const char kDanger[]        = "#ef4444";
const char kDangerPressed[] = "#d33636";

QPalette darkPalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(kBg));
    p.setColor(QPalette::WindowText, QColor(kText));
    p.setColor(QPalette::Base, QColor(kBgElevated));
    p.setColor(QPalette::AlternateBase, QColor(kBgElevated2));
    p.setColor(QPalette::ToolTipBase, QColor(kBgElevated2));
    p.setColor(QPalette::ToolTipText, QColor(kText));
    p.setColor(QPalette::Text, QColor(kText));
    p.setColor(QPalette::Button, QColor(kBgElevated2));
    p.setColor(QPalette::ButtonText, QColor(kText));
    p.setColor(QPalette::BrightText, QColor(kDanger));
    p.setColor(QPalette::Link, QColor(kAccent));
    p.setColor(QPalette::Highlight, QColor(kAccent));
    p.setColor(QPalette::HighlightedText, QColor(kAccentInk));
    p.setColor(QPalette::PlaceholderText, QColor(kTextFaint));

    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(kTextFaint));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(kTextFaint));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(kTextFaint));
    return p;
}

// Substitute @token@ placeholders in the template with their design-token
// values. Named tokens (rather than QString::arg's %1, %2, ...) keep the
// stylesheet below readable and safe to reorder/extend without having to
// keep a mental map of argument positions.
QString substitute(QString tpl, const QList<QPair<QString, QString>> &tokens)
{
    for (const auto &kv : tokens)
        tpl.replace(kv.first, kv.second);
    return tpl;
}

QString styleSheet()
{
    const QString tpl = QStringLiteral(R"(
        QWidget {
            color: @text@;
            font-size: 13px;
        }
        QMainWindow, QDialog {
            background: @bg@;
        }

        /* --- Cards (QGroupBox) --------------------------------------- */
        /* The title lives in the "margin box" reserved by margin-top, i.e.
           the band from y=0 (the widget's own top edge) to y=margin-top (the
           border). A negative `top` nudges it further up from there — past
           y=0 — and Qt clips anything painted outside the widget's own rect,
           which is exactly what cut the title off once this card became the
           first thing below the frameless window's title bar (no native
           chrome above it to absorb the overflow). Keeping `top` at 0 (or
           slightly positive) keeps the title inside its own band instead.
        */
        QGroupBox {
            background: @bgElevated@;
            border: 1px solid @border@;
            border-radius: 10px;
            margin-top: 26px;
            padding: 20px 12px 12px 12px;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            top: 4px;
            padding: 0 6px;
            color: @textMuted@;
            background: transparent;
        }

        QLabel {
            background: transparent;
        }

        /* --- Inputs ---------------------------------------------------- */
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background: @bg@;
            border: 1px solid @border@;
            border-radius: 6px;
            padding: 5px 8px;
            selection-background-color: @accent@;
            selection-color: @text@;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1px solid @accent@;
        }
        QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled {
            color: @textFaint@;
        }
        QComboBox::drop-down {
            border: none;
            width: 22px;
        }
        QComboBox QAbstractItemView {
            background: @bgElevated@;
            border: 1px solid @border@;
            selection-background-color: @accentSoft@;
            selection-color: @text@;
        }
        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            width: 16px;
            border: none;
            background: transparent;
        }

        /* --- Buttons ----------------------------------------------------
           Default = secondary (Pause/Resume/Refresh/Cancel). Role-specific
           buttons are targeted by object name below: #scheduleButton is the
           primary call-to-action, #rollbackButton is destructive. */
        QPushButton {
            background: @bgElevated2@;
            color: @text@;
            border: 1px solid @border@;
            border-radius: 6px;
            padding: 7px 16px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: @bgElevated2@;
            border: 1px solid @textMuted@;
        }
        QPushButton:pressed {
            background: @bg@;
        }
        QPushButton:disabled {
            color: @textFaint@;
            border: 1px solid @border@;
        }

        QPushButton#scheduleButton {
            background: @accent@;
            color: @accentInk@;
            border: 1px solid @accent@;
        }
        QPushButton#scheduleButton:hover {
            background: @accentHover@;
            border: 1px solid @accentHover@;
        }
        QPushButton#scheduleButton:pressed {
            background: @accentPressed@;
            border: 1px solid @accentPressed@;
        }

        QPushButton#rollbackButton {
            background: transparent;
            color: @danger@;
            border: 1px solid @danger@;
        }
        QPushButton#rollbackButton:hover {
            background: @danger@;
            color: @text@;
        }
        QPushButton#rollbackButton:pressed {
            background: @dangerPressed@;
            border: 1px solid @dangerPressed@;
            color: @text@;
        }

        /* --- Table / dashboard ------------------------------------------ */
        QTableWidget {
            background: @bg@;
            alternate-background-color: @bgElevated@;
            gridline-color: @border@;
            border: 1px solid @border@;
            border-radius: 8px;
        }
        QTableWidget::item {
            padding: 4px 6px;
            border: none;
        }
        QTableWidget::item:selected {
            background: @accentSoft@;
            color: @text@;
        }
        QHeaderView::section {
            background: @bgElevated2@;
            color: @textMuted@;
            padding: 6px 8px;
            border: none;
            border-bottom: 1px solid @border@;
            font-weight: 600;
        }
        QTableCornerButton::section {
            background: @bgElevated2@;
            border: none;
        }

        /* --- Tool bar / status bar / docks ------------------------------ */
        QToolBar {
            background: @bgElevated@;
            border-bottom: 1px solid @border@;
            spacing: 6px;
            padding: 4px;
        }
        QToolButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 5px 10px;
        }
        QToolButton:hover {
            background: @bgElevated2@;
            border: 1px solid @border@;
        }
        QStatusBar {
            background: @bgElevated@;
            border-top: 1px solid @border@;
            color: @textMuted@;
        }
        QStatusBar::item {
            border: none;
        }
        QDockWidget {
            titlebar-close-icon: none;
            color: @text@;
        }
        QDockWidget::title {
            background: @bgElevated@;
            padding: 6px 10px;
            border-bottom: 1px solid @border@;
            font-weight: 600;
            color: @textMuted@;
        }

        /* --- Tabs -------------------------------------------------------- */
        QTabWidget::pane {
            border: 1px solid @border@;
            border-radius: 8px;
            top: -1px;
        }
        QTabBar::tab {
            background: @bg@;
            color: @textMuted@;
            border: 1px solid @border@;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 7px 14px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background: @bgElevated@;
            color: @text@;
        }
        QTabBar::tab:hover {
            color: @text@;
        }

        /* --- Scroll bars --------------------------------------------- */
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: @border@;
            border-radius: 4px;
            min-height: 24px;
        }
        QScrollBar::handle:vertical:hover {
            background: @textMuted@;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 10px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal {
            background: @border@;
            border-radius: 4px;
            min-width: 24px;
        }
        QScrollBar::handle:horizontal:hover {
            background: @textMuted@;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            height: 0px;
            width: 0px;
            border: none;
            background: none;
        }

        QToolTip {
            background: @bgElevated@;
            color: @text@;
            border: 1px solid @border@;
            padding: 4px 6px;
        }

        /* --- Frameless-window title bar (WindowTitleBar) ----------------- */
        #windowTitleBar {
            background: @bgElevated@;
            border-bottom: 1px solid @border@;
        }
        #windowMinimizeButton, #windowCloseButton {
            background: transparent;
            border: none;
            border-radius: 4px;
            padding: 0px;
            font-weight: 400;
        }
        #windowMinimizeButton:hover {
            background: @bgElevated2@;
            border: none;
        }
        #windowCloseButton:hover {
            background: @danger@;
            color: @text@;
            border: none;
        }
        #windowCloseButton:pressed {
            background: @dangerPressed@;
        }
    )");

    return substitute(tpl, {
        {QStringLiteral("@text@"), QLatin1String(kText)},
        {QStringLiteral("@bg@"), QLatin1String(kBg)},
        {QStringLiteral("@bgElevated2@"), QLatin1String(kBgElevated2)},
        {QStringLiteral("@bgElevated@"), QLatin1String(kBgElevated)},
        {QStringLiteral("@border@"), QLatin1String(kBorder)},
        {QStringLiteral("@textMuted@"), QLatin1String(kTextMuted)},
        {QStringLiteral("@textFaint@"), QLatin1String(kTextFaint)},
        {QStringLiteral("@accentSoft@"), QLatin1String(kAccentSoft)},
        {QStringLiteral("@accentHover@"), QLatin1String(kAccentHover)},
        {QStringLiteral("@accentPressed@"), QLatin1String(kAccentPressed)},
        {QStringLiteral("@accentInk@"), QLatin1String(kAccentInk)},
        {QStringLiteral("@accent@"), QLatin1String(kAccent)},
        {QStringLiteral("@dangerPressed@"), QLatin1String(kDangerPressed)},
        {QStringLiteral("@danger@"), QLatin1String(kDanger)},
    });
}

} // namespace

void Theme::apply(QApplication &app)
{
    // Fusion renders QSS-driven rounded corners, custom borders, and hover
    // states consistently across Windows/macOS/Linux; the native Windows
    // style ignores most of the stylesheet below.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    app.setPalette(darkPalette());
    app.setStyleSheet(styleSheet());
}
