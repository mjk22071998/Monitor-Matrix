#include "AppTheme.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>

namespace {
AppThemeMode s_mode = AppThemeMode::Dark;
bool s_fontsLoaded = false;

void loadBundledFonts()
{
    if (s_fontsLoaded) {
        return;
    }

    s_fontsLoaded = true;

    const QStringList fontPaths = {
        QStringLiteral(":/fonts/DMSans.ttf"),
        QStringLiteral(":/fonts/DMSans-Italic.ttf")
    };

    for (const QString& fontPath : fontPaths) {
        QFontDatabase::addApplicationFont(fontPath);
    }
}

QColor rgba(int r, int g, int b, int a)
{
    return QColor(r, g, b, a);
}

const AppThemePalette& darkPalette()
{
    static const AppThemePalette palette = [] {
        AppThemePalette p;

        p.text = QColor(236, 239, 244);
        p.windowBg = QColor(17, 18, 20);
        p.menuBarBg = QColor(21, 22, 25);
        p.menuBorder = rgba(255, 255, 255, 18);
        p.mutedText = QColor(154, 161, 170);
        p.menuItemSelectedBg = rgba(45, 212, 191, 24);
        p.menuPopupBg = QColor(26, 27, 31);
        p.menuPopupBorder = rgba(255, 255, 255, 30);
        p.menuText = QColor(221, 225, 231);
        p.menuSelectedBg = rgba(45, 212, 191, 42);
        p.menuSelectedText = QColor(255, 255, 255);
        p.separator = rgba(255, 255, 255, 18);
        p.disabledText = QColor(102, 109, 118);
        p.splitterHandle = rgba(255, 255, 255, 18);
        p.splitterHandleHover = rgba(45, 212, 191, 130);

        p.panelBg = QColor(23, 24, 28);
        p.panelBorder = rgba(255, 255, 255, 22);
        p.sectionHeaderText = QColor(165, 173, 184);

        p.listItemBg = rgba(255, 255, 255, 7);
        p.listItemBorder = rgba(255, 255, 255, 13);
        p.listItemText = QColor(228, 232, 238);
        p.listItemHoverBg = rgba(45, 212, 191, 17);
        p.listItemHoverBorder = rgba(45, 212, 191, 92);
        p.listItemHoverText = QColor(255, 255, 255);
        p.listItemSelectedBg = rgba(45, 212, 191, 30);
        p.listItemSelectedBorder = rgba(45, 212, 191, 150);
        p.listItemSelectedText = QColor(255, 255, 255);
        p.listItemDisabledBg = rgba(255, 255, 255, 5);
        p.listItemDisabledBorder = rgba(255, 255, 255, 8);
        p.listItemDisabledText = QColor(113, 120, 130);

        p.primaryButtonStart = QColor(20, 184, 166);
        p.primaryButtonEnd = QColor(22, 163, 74);
        p.primaryButtonHoverStart = QColor(45, 212, 191);
        p.primaryButtonHoverEnd = QColor(34, 197, 94);
        p.primaryButtonPressedStart = QColor(13, 148, 136);
        p.primaryButtonPressedEnd = QColor(21, 128, 61);
        p.primaryButtonText = QColor(255, 255, 255);
        p.primaryButtonDisabledBg = rgba(45, 212, 191, 40);
        p.primaryButtonDisabledText = rgba(255, 255, 255, 70);

        p.secondaryButtonBg = rgba(255, 255, 255, 8);
        p.secondaryButtonBorder = rgba(255, 255, 255, 28);
        p.secondaryButtonText = QColor(221, 225, 231);
        p.secondaryButtonHoverBg = rgba(255, 255, 255, 14);
        p.secondaryButtonHoverBorder = rgba(45, 212, 191, 90);
        p.secondaryButtonHoverText = QColor(255, 255, 255);
        p.secondaryButtonPressedBg = rgba(255, 255, 255, 10);

        p.graphicsViewBg = QColor(15, 16, 18);
        p.scrollHandle = rgba(255, 255, 255, 55);
        p.scrollHandleHover = rgba(45, 212, 191, 160);

        p.dialogBg = QColor(24, 26, 30);
        p.dialogText = QColor(228, 232, 238);
        p.dialogButtonBg = rgba(255, 255, 255, 9);
        p.dialogButtonBorder = rgba(255, 255, 255, 30);
        p.dialogButtonText = QColor(221, 225, 231);
        p.dialogButtonHoverBg = rgba(255, 255, 255, 15);
        p.dialogButtonHoverText = QColor(255, 255, 255);
        p.dialogButtonDefaultBg = rgba(45, 212, 191, 42);

        p.inputBg = QColor(18, 19, 22);
        p.inputBorder = rgba(255, 255, 255, 28);
        p.inputText = QColor(236, 239, 244);
        p.inputSelectionBg = rgba(45, 212, 191, 150);
        p.inputFocusBorder = QColor(45, 212, 191);

        p.tooltipBg = QColor(29, 31, 36);
        p.tooltipText = QColor(236, 239, 244);
        p.tooltipBorder = rgba(255, 255, 255, 38);

        p.danger = QColor(248, 113, 113);
        p.success = QColor(52, 211, 153);

        p.sceneBackground = QColor(14, 15, 17);
        p.monitorBorder = rgba(45, 212, 191, 210);
        p.monitorFill = rgba(24, 26, 30, 235);
        p.badgeFill = rgba(45, 212, 191, 36);
        p.badgeText = rgba(236, 239, 244, 230);
        p.dropHighlight = rgba(245, 158, 11, 76);
        p.dockBorder = rgba(148, 163, 184, 185);
        p.dockHoverBorder = rgba(52, 211, 153, 245);
        p.dockFill = rgba(20, 22, 26, 235);
        p.dockHoverFill = rgba(52, 211, 153, 58);
        p.dockScreenOutline = rgba(236, 239, 244, 220);
        p.dockPreviewFill = rgba(52, 211, 153, 120);
        p.dockPreviewHoverFill = rgba(52, 211, 153, 210);
        p.placementBorder = rgba(56, 189, 248, 230);
        p.placementFill = rgba(56, 189, 248, 34);
        p.placementText = QColor(236, 239, 244);
        p.placementOverlapBorder = rgba(248, 113, 113, 255);
        p.placementOverlapFill = rgba(248, 113, 113, 45);
        p.guide = rgba(245, 158, 11, 230);
        p.primaryDisplayBorder = rgba(251, 191, 36, 245);
        p.primaryDisplayFill = rgba(251, 191, 36, 42);
        p.primaryDisplayText = rgba(255, 247, 237, 240);

        return p;
    }();

    return palette;
}

const AppThemePalette& lightPalette()
{
    static const AppThemePalette palette = [] {
        AppThemePalette p;

        p.text = QColor(31, 41, 55);
        p.windowBg = QColor(245, 247, 250);
        p.menuBarBg = QColor(255, 255, 255);
        p.menuBorder = rgba(15, 23, 42, 18);
        p.mutedText = QColor(104, 115, 133);
        p.menuItemSelectedBg = rgba(20, 184, 166, 24);
        p.menuPopupBg = QColor(255, 255, 255);
        p.menuPopupBorder = rgba(15, 23, 42, 28);
        p.menuText = QColor(47, 57, 76);
        p.menuSelectedBg = rgba(20, 184, 166, 36);
        p.menuSelectedText = QColor(17, 24, 39);
        p.separator = rgba(15, 23, 42, 18);
        p.disabledText = QColor(148, 156, 170);
        p.splitterHandle = rgba(15, 23, 42, 18);
        p.splitterHandleHover = rgba(20, 184, 166, 120);

        p.panelBg = QColor(255, 255, 255);
        p.panelBorder = rgba(15, 23, 42, 20);
        p.sectionHeaderText = QColor(100, 111, 128);

        p.listItemBg = QColor(255, 255, 255);
        p.listItemBorder = rgba(15, 23, 42, 18);
        p.listItemText = QColor(31, 41, 55);
        p.listItemHoverBg = rgba(20, 184, 166, 14);
        p.listItemHoverBorder = rgba(20, 184, 166, 88);
        p.listItemHoverText = QColor(15, 23, 42);
        p.listItemSelectedBg = rgba(20, 184, 166, 24);
        p.listItemSelectedBorder = rgba(20, 184, 166, 130);
        p.listItemSelectedText = QColor(15, 23, 42);
        p.listItemDisabledBg = rgba(15, 23, 42, 4);
        p.listItemDisabledBorder = rgba(15, 23, 42, 8);
        p.listItemDisabledText = QColor(151, 159, 172);

        p.primaryButtonStart = QColor(20, 184, 166);
        p.primaryButtonEnd = QColor(22, 163, 74);
        p.primaryButtonHoverStart = QColor(13, 148, 136);
        p.primaryButtonHoverEnd = QColor(21, 128, 61);
        p.primaryButtonPressedStart = QColor(15, 118, 110);
        p.primaryButtonPressedEnd = QColor(22, 101, 52);
        p.primaryButtonText = QColor(255, 255, 255);
        p.primaryButtonDisabledBg = rgba(20, 184, 166, 45);
        p.primaryButtonDisabledText = rgba(31, 41, 55, 90);

        p.secondaryButtonBg = QColor(255, 255, 255);
        p.secondaryButtonBorder = rgba(15, 23, 42, 28);
        p.secondaryButtonText = QColor(31, 41, 55);
        p.secondaryButtonHoverBg = QColor(248, 250, 252);
        p.secondaryButtonHoverBorder = rgba(20, 184, 166, 95);
        p.secondaryButtonHoverText = QColor(15, 23, 42);
        p.secondaryButtonPressedBg = rgba(15, 23, 42, 5);

        p.graphicsViewBg = QColor(239, 242, 247);
        p.scrollHandle = rgba(15, 23, 42, 48);
        p.scrollHandleHover = rgba(20, 184, 166, 145);

        p.dialogBg = QColor(255, 255, 255);
        p.dialogText = QColor(31, 41, 55);
        p.dialogButtonBg = QColor(255, 255, 255);
        p.dialogButtonBorder = rgba(15, 23, 42, 28);
        p.dialogButtonText = QColor(31, 41, 55);
        p.dialogButtonHoverBg = QColor(248, 250, 252);
        p.dialogButtonHoverText = QColor(15, 23, 42);
        p.dialogButtonDefaultBg = rgba(20, 184, 166, 38);

        p.inputBg = QColor(255, 255, 255);
        p.inputBorder = rgba(15, 23, 42, 28);
        p.inputText = QColor(31, 41, 55);
        p.inputSelectionBg = rgba(20, 184, 166, 140);
        p.inputFocusBorder = QColor(20, 184, 166);

        p.tooltipBg = QColor(31, 41, 55);
        p.tooltipText = QColor(255, 255, 255);
        p.tooltipBorder = rgba(15, 23, 42, 48);

        p.danger = QColor(220, 38, 38);
        p.success = QColor(5, 150, 105);

        p.sceneBackground = QColor(239, 242, 247);
        p.monitorBorder = rgba(20, 184, 166, 205);
        p.monitorFill = rgba(255, 255, 255, 240);
        p.badgeFill = rgba(20, 184, 166, 30);
        p.badgeText = QColor(31, 41, 55);
        p.dropHighlight = rgba(245, 158, 11, 66);
        p.dockBorder = rgba(100, 116, 139, 160);
        p.dockHoverBorder = rgba(5, 150, 105, 235);
        p.dockFill = rgba(255, 255, 255, 240);
        p.dockHoverFill = rgba(5, 150, 105, 48);
        p.dockScreenOutline = rgba(31, 41, 55, 205);
        p.dockPreviewFill = rgba(5, 150, 105, 100);
        p.dockPreviewHoverFill = rgba(5, 150, 105, 185);
        p.placementBorder = rgba(14, 165, 233, 225);
        p.placementFill = rgba(14, 165, 233, 32);
        p.placementText = QColor(31, 41, 55);
        p.placementOverlapBorder = rgba(220, 38, 38, 255);
        p.placementOverlapFill = rgba(220, 38, 38, 42);
        p.guide = rgba(245, 158, 11, 220);
        p.primaryDisplayBorder = rgba(217, 119, 6, 235);
        p.primaryDisplayFill = rgba(245, 158, 11, 34);
        p.primaryDisplayText = QColor(31, 41, 55);

        return p;
    }();

    return palette;
}

QString gradient(const QColor& start, const QColor& end)
{
    return QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2)")
        .arg(AppTheme::cssColor(start), AppTheme::cssColor(end));
}
}

AppThemeMode AppTheme::mode()
{
    return s_mode;
}

const AppThemePalette& AppTheme::palette()
{
    return s_mode == AppThemeMode::Light ? lightPalette() : darkPalette();
}

void AppTheme::apply(QApplication& app, AppThemeMode mode)
{
    loadBundledFonts();
    app.setFont(QFont(fontFamily(), 10));

    setMode(mode);
    app.setStyleSheet(styleSheet());
}

void AppTheme::setMode(AppThemeMode mode)
{
    s_mode = mode;
}

QString AppTheme::fontFamily()
{
    return QStringLiteral("DM Sans");
}

QString AppTheme::cssColor(const QColor& color)
{
    if (color.alpha() == 255) {
        return color.name(QColor::HexRgb);
    }

    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(QString::number(color.alphaF(), 'f', 3));
}

QString AppTheme::statusLabelStyle(bool isWarning)
{
    const AppThemePalette& p = palette();

    return QStringLiteral("QLabel { color: %1; font-weight: 700; padding: 2px 0; background: transparent; }")
        .arg(cssColor(isWarning ? p.danger : p.success));
}

QString AppTheme::styleSheet()
{
    const AppThemePalette& p = palette();
    QString css;

    css += QStringLiteral(R"(
* {
    font-family: "DM Sans", "Segoe UI Variable", "Segoe UI", sans-serif;
    font-size: 13px;
    color: %1;
    outline: none;
}

QMainWindow, QWidget {
    background-color: %2;
}

QMenuBar {
    background-color: %3;
    border-bottom: 1px solid %4;
    padding: 3px 8px;
    spacing: 4px;
}

QMenuBar::item {
    background: transparent;
    padding: 6px 12px;
    border-radius: 6px;
    color: %5;
    font-size: 12px;
    font-weight: 500;
}

QMenuBar::item:selected,
QMenuBar::item:pressed {
    background: %6;
    color: %1;
}

QMenu {
    background-color: %7;
    border: 1px solid %8;
    border-radius: 8px;
    padding: 6px;
}

QMenu::item {
    padding: 7px 22px 7px 14px;
    border-radius: 6px;
    margin: 1px 4px;
    color: %9;
    font-size: 12px;
}

QMenu::item:selected {
    background: %10;
    color: %11;
}

QMenu::separator {
    height: 1px;
    background: %12;
    margin: 5px 10px;
}

QMenu::item:disabled {
    color: %13;
}

QSplitter::handle {
    background: %14;
    width: 1px;
}

QSplitter::handle:hover {
    background: %15;
}
)")
        .arg(cssColor(p.text))
        .arg(cssColor(p.windowBg))
        .arg(cssColor(p.menuBarBg))
        .arg(cssColor(p.menuBorder))
        .arg(cssColor(p.mutedText))
        .arg(cssColor(p.menuItemSelectedBg))
        .arg(cssColor(p.menuPopupBg))
        .arg(cssColor(p.menuPopupBorder))
        .arg(cssColor(p.menuText))
        .arg(cssColor(p.menuSelectedBg))
        .arg(cssColor(p.menuSelectedText))
        .arg(cssColor(p.separator))
        .arg(cssColor(p.disabledText))
        .arg(cssColor(p.splitterHandle))
        .arg(cssColor(p.splitterHandleHover));

    css += QStringLiteral(R"(
QWidget#leftPanel {
    background-color: %1;
    border-right: 1px solid %2;
}

QLabel#sectionHeader {
    background: transparent;
    font-size: 11px;
    font-weight: 700;
    color: %3;
    padding: 2px 4px 6px 4px;
}

QListWidget {
    background: transparent;
    border: none;
    padding: 3px 0;
    outline: none;
    show-decoration-selected: 1;
}

QListWidget::item {
    background: %4;
    border: 1px solid %5;
    border-radius: 7px;
    padding: 9px 10px;
    margin: 3px 0;
    color: %6;
}

QListWidget::item:hover {
    background: %7;
    border: 1px solid %8;
    color: %9;
}

QListWidget::item:selected {
    background: %10;
    border: 1px solid %11;
    color: %12;
}

QListWidget::item:disabled {
    background: %13;
    border: 1px solid %14;
    color: %15;
}
)")
        .arg(cssColor(p.panelBg))
        .arg(cssColor(p.panelBorder))
        .arg(cssColor(p.sectionHeaderText))
        .arg(cssColor(p.listItemBg))
        .arg(cssColor(p.listItemBorder))
        .arg(cssColor(p.listItemText))
        .arg(cssColor(p.listItemHoverBg))
        .arg(cssColor(p.listItemHoverBorder))
        .arg(cssColor(p.listItemHoverText))
        .arg(cssColor(p.listItemSelectedBg))
        .arg(cssColor(p.listItemSelectedBorder))
        .arg(cssColor(p.listItemSelectedText))
        .arg(cssColor(p.listItemDisabledBg))
        .arg(cssColor(p.listItemDisabledBorder))
        .arg(cssColor(p.listItemDisabledText));

    css += QStringLiteral(R"(
QPushButton {
    background: %1;
    border: 1px solid %2;
    border-radius: 8px;
    color: %3;
    font-weight: 600;
    padding: 8px 14px;
    min-height: 22px;
}

QPushButton:hover {
    background: %4;
    border-color: %5;
    color: %6;
}

QPushButton:pressed {
    background: %7;
}

QPushButton:disabled {
    background: %7;
    border-color: %2;
    color: %13;
}

QPushButton#runButton {
    background: %8;
    color: %9;
    font-weight: 700;
    font-size: 13px;
    border: none;
    border-radius: 8px;
    padding: 11px 18px;
    margin: 8px 6px 10px 6px;
    min-height: 24px;
}

QPushButton#runButton:hover {
    background: %10;
}

QPushButton#runButton:pressed {
    background: %11;
}

QPushButton#runButton:disabled {
    background: %12;
    color: %13;
}

QPushButton#secondaryButton {
    background: %1;
    border-color: %2;
    color: %3;
}

QPushButton#secondaryButton:hover {
    background: %4;
    border-color: %5;
    color: %6;
}

QPushButton#secondaryButton:pressed {
    background: %7;
}

QPushButton#dangerButton {
    background: transparent;
    border-color: %14;
    color: %14;
}

QPushButton#dangerButton:hover {
    background: %15;
    border-color: %14;
    color: %14;
}
)")
        .arg(cssColor(p.secondaryButtonBg))
        .arg(cssColor(p.secondaryButtonBorder))
        .arg(cssColor(p.secondaryButtonText))
        .arg(cssColor(p.secondaryButtonHoverBg))
        .arg(cssColor(p.secondaryButtonHoverBorder))
        .arg(cssColor(p.secondaryButtonHoverText))
        .arg(cssColor(p.secondaryButtonPressedBg))
        .arg(gradient(p.primaryButtonStart, p.primaryButtonEnd))
        .arg(cssColor(p.primaryButtonText))
        .arg(gradient(p.primaryButtonHoverStart, p.primaryButtonHoverEnd))
        .arg(gradient(p.primaryButtonPressedStart, p.primaryButtonPressedEnd))
        .arg(cssColor(p.primaryButtonDisabledBg))
        .arg(cssColor(p.primaryButtonDisabledText))
        .arg(cssColor(p.danger))
        .arg(cssColor(p.placementOverlapFill));

    css += QStringLiteral(R"(
QGraphicsView {
    background-color: %1;
    border: none;
}

QStatusBar {
    background-color: %4;
    border-top: 1px solid %5;
    color: %6;
    padding: 4px 10px;
}

QStatusBar QLabel {
    background: transparent;
}

QScrollBar:vertical, QScrollBar:horizontal {
    background: transparent;
    width: 8px;
    height: 8px;
    margin: 0;
}

QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
    background: %2;
    border-radius: 4px;
    min-height: 24px;
    min-width: 24px;
}

QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
    background: %3;
}

QScrollBar::add-line, QScrollBar::sub-line,
QScrollBar::add-page, QScrollBar::sub-page {
    background: transparent;
    border: none;
}
)")
        .arg(cssColor(p.graphicsViewBg))
        .arg(cssColor(p.scrollHandle))
        .arg(cssColor(p.scrollHandleHover))
        .arg(cssColor(p.panelBg))
        .arg(cssColor(p.panelBorder))
        .arg(cssColor(p.mutedText));

    css += QStringLiteral(R"(
QDialog, QMessageBox, QFileDialog {
    background-color: %1;
}

QMessageBox QLabel, QDialog QLabel, QGroupBox, QCheckBox {
    color: %2;
    font-size: 13px;
}

QGroupBox {
    background: transparent;
    border: 1px solid %3;
    border-radius: 8px;
    margin-top: 12px;
    padding: 14px 10px 10px 10px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: %4;
    font-weight: 600;
}

QFileDialog QListView,
QFileDialog QTreeView,
QComboBox QAbstractItemView {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 8px;
    color: %8;
    selection-background-color: %9;
}

QComboBox QAbstractItemView::item {
    padding: 6px 10px;
    min-height: 22px;
}

QFileDialog QLineEdit,
QLineEdit,
QComboBox {
    background: %7;
    border: 1px solid %6;
    border-radius: 8px;
    padding: 6px 10px;
    color: %8;
    min-height: 22px;
    selection-background-color: %9;
}

QFileDialog QLineEdit:hover,
QLineEdit:hover,
QComboBox:hover {
    border-color: %16;
}

QLineEdit:focus,
QComboBox:focus {
    border-color: %10;
}

QComboBox::drop-down {
    border: none;
    width: 28px;
}

QCheckBox {
    spacing: 8px;
    background: transparent;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border-radius: 4px;
    border: 1px solid %6;
    background: %7;
}

QCheckBox::indicator:hover {
    border-color: %10;
}

QCheckBox::indicator:checked {
    background: %17;
    border-color: %17;
}

QCheckBox::indicator:disabled {
    background: %14;
    border-color: %16;
}

QToolTip {
    background-color: %11;
    color: %12;
    border: 1px solid %13;
    border-radius: 6px;
    padding: 5px 9px;
    font-size: 12px;
}
)")
        .arg(cssColor(p.dialogBg))
        .arg(cssColor(p.dialogText))
        .arg(cssColor(p.menuPopupBorder))
        .arg(cssColor(p.mutedText))
        .arg(cssColor(p.panelBg))
        .arg(cssColor(p.inputBorder))
        .arg(cssColor(p.inputBg))
        .arg(cssColor(p.inputText))
        .arg(cssColor(p.inputSelectionBg))
        .arg(cssColor(p.inputFocusBorder))
        .arg(cssColor(p.tooltipBg))
        .arg(cssColor(p.tooltipText))
        .arg(cssColor(p.tooltipBorder))
        .arg(cssColor(p.secondaryButtonHoverBg))
        .arg(cssColor(p.secondaryButtonBorder))
        .arg(cssColor(p.secondaryButtonHoverBorder))
        .arg(cssColor(p.success));

    return css;
}
