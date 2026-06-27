#pragma once

#include <QColor>
#include <QString>

class QApplication;

enum class AppThemeMode {
    Dark,
    Light
};

struct AppThemePalette {
    QColor text;
    QColor windowBg;
    QColor menuBarBg;
    QColor menuBorder;
    QColor mutedText;
    QColor menuItemSelectedBg;
    QColor menuPopupBg;
    QColor menuPopupBorder;
    QColor menuText;
    QColor menuSelectedBg;
    QColor menuSelectedText;
    QColor separator;
    QColor disabledText;
    QColor splitterHandle;
    QColor splitterHandleHover;

    QColor panelBg;
    QColor panelBorder;
    QColor sectionHeaderText;

    QColor listItemBg;
    QColor listItemBorder;
    QColor listItemText;
    QColor listItemHoverBg;
    QColor listItemHoverBorder;
    QColor listItemHoverText;
    QColor listItemSelectedBg;
    QColor listItemSelectedBorder;
    QColor listItemSelectedText;
    QColor listItemDisabledBg;
    QColor listItemDisabledBorder;
    QColor listItemDisabledText;

    QColor primaryButtonStart;
    QColor primaryButtonEnd;
    QColor primaryButtonHoverStart;
    QColor primaryButtonHoverEnd;
    QColor primaryButtonPressedStart;
    QColor primaryButtonPressedEnd;
    QColor primaryButtonText;
    QColor primaryButtonDisabledBg;
    QColor primaryButtonDisabledText;

    QColor secondaryButtonBg;
    QColor secondaryButtonBorder;
    QColor secondaryButtonText;
    QColor secondaryButtonHoverBg;
    QColor secondaryButtonHoverBorder;
    QColor secondaryButtonHoverText;
    QColor secondaryButtonPressedBg;

    QColor graphicsViewBg;
    QColor scrollHandle;
    QColor scrollHandleHover;

    QColor dialogBg;
    QColor dialogText;
    QColor dialogButtonBg;
    QColor dialogButtonBorder;
    QColor dialogButtonText;
    QColor dialogButtonHoverBg;
    QColor dialogButtonHoverText;
    QColor dialogButtonDefaultBg;

    QColor inputBg;
    QColor inputBorder;
    QColor inputText;
    QColor inputSelectionBg;
    QColor inputFocusBorder;

    QColor tooltipBg;
    QColor tooltipText;
    QColor tooltipBorder;

    QColor danger;
    QColor success;

    QColor sceneBackground;
    QColor monitorBorder;
    QColor monitorFill;
    QColor badgeFill;
    QColor badgeText;
    QColor dropHighlight;
    QColor dockBorder;
    QColor dockHoverBorder;
    QColor dockFill;
    QColor dockHoverFill;
    QColor dockScreenOutline;
    QColor dockPreviewFill;
    QColor dockPreviewHoverFill;
    QColor placementBorder;
    QColor placementFill;
    QColor placementText;
    QColor placementOverlapBorder;
    QColor placementOverlapFill;
    QColor guide;
    QColor primaryDisplayBorder;
    QColor primaryDisplayFill;
    QColor primaryDisplayText;
};

class AppTheme {
public:
    static AppThemeMode mode();
    static const AppThemePalette& palette();

    static void apply(QApplication& app, AppThemeMode mode);
    static void setMode(AppThemeMode mode);

    static QString fontFamily();
    static QString styleSheet();
    static QString statusLabelStyle(bool isWarning);
    static QString cssColor(const QColor& color);
};
