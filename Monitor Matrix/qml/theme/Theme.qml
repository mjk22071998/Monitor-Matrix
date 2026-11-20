pragma Singleton
import QtQuick

QtObject {
    id: root

    property bool isDark: true

    // Core Color Tokens (Studio Matrix)
    readonly property color bgBase: isDark ? "#080B11" : "#F8FAFC"
    readonly property color bgCanvas: isDark ? "#0E131F" : "#F1F5F9"
    readonly property color bgSurface: isDark ? "#141B2B" : "#FFFFFF"
    readonly property color bgSurfaceHover: isDark ? "#1B243A" : "#F1F5F9"
    readonly property color bgSurfaceActive: isDark ? "#24304D" : "#E2E8F0"
    readonly property color bgSurfaceElevated: isDark ? "#1C253B" : "#FFFFFF"

    readonly property color borderSubtle: isDark ? "#212D45" : "#E2E8F0"
    readonly property color borderStrong: isDark ? "#334155" : "#CBD5E1"
    readonly property color borderFocus: "#00E5FF"

    readonly property color accentCyan: "#00E5FF"
    readonly property color accentCyanHover: "#33EBFF"
    readonly property color accentCyanMuted: isDark ? "#083344" : "#CFFAFE"
    readonly property color accentCyanGlow: "#3300E5FF"

    readonly property color accentEmerald: "#10B981"
    readonly property color accentEmeraldMuted: isDark ? "#064E3B" : "#D1FAE5"

    readonly property color accentAmber: "#F59E0B"
    readonly property color accentAmberMuted: isDark ? "#78350F" : "#FEF3C7"

    readonly property color accentCrimson: "#EF4444"
    readonly property color accentCrimsonMuted: isDark ? "#7F1D1D" : "#FEE2E2"

    readonly property color textPrimary: isDark ? "#F1F5F9" : "#0F172A"
    readonly property color textSecondary: isDark ? "#94A3B8" : "#475569"
    readonly property color textMuted: isDark ? "#64748B" : "#94A3B8"

    readonly property color tagExecutable: "#06B6D4"
    readonly property color tagUrl: "#A78BFA"

    // Typography Tokens
    readonly property string fontFamily: "DM Sans"
    readonly property string fontMono: "Consolas"

    readonly property int fontSizeTiny: 10
    readonly property int fontSizeSmall: 12
    readonly property int fontSizeRegular: 14
    readonly property int fontSizeMedium: 16
    readonly property int fontSizeLarge: 18
    readonly property int fontSizeHeader: 20
    readonly property int fontSizeTitle: 24

    readonly property int fontWeightNormal: Font.Normal
    readonly property int fontWeightMedium: Font.Medium
    readonly property int fontWeightDemiBold: Font.DemiBold
    readonly property int fontWeightBold: Font.Bold

    // Spacing Tokens
    readonly property int spacing2Xs: 2
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
    readonly property int spacingLg: 16
    readonly property int spacingXl: 24
    readonly property int spacing2Xl: 32

    // Border Radius Tokens
    readonly property int radiusSm: 4
    readonly property int radiusMd: 6
    readonly property int radiusLg: 8
    readonly property int radiusXl: 12
    readonly property int radiusPill: 9999

    // Animation & Motion Tokens (180ms - 240ms cubic easing)
    readonly property int durationFast: 180
    readonly property int durationNormal: 240
    readonly property int durationSlow: 360
    readonly property int easingType: Easing.OutCubic
    readonly property int easingInOut: Easing.InOutCubic
}
