import QtQuick
import QtQuick.Controls
import "../theme"

Rectangle {
    id: root

    property string currentFileName: ""
    property bool isDirty: false
    property bool isRunning: false
    property bool hasOverlaps: false
    property int invalidCount: 0
    property bool darkTheme: true

    signal runLayoutClicked()
    signal newLayoutClicked()
    signal openLayoutClicked()
    signal saveLayoutClicked()
    signal saveAsLayoutClicked()
    signal screenSettingsClicked()
    signal openLogsClicked()
    signal themeToggleClicked()

    implicitWidth: 1280
    implicitHeight: 52
    color: Theme.bgSurface
    border.color: Theme.borderSubtle
    border.width: 0

    // Bottom separator line
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.borderSubtle
    }

    // Inline Reusable Header Button
    component HeaderButton: Rectangle {
        id: btn
        property alias text: label.text
        property alias tooltipText: toolTip.text
        property string iconSource: ""
        signal clicked()

        implicitWidth: contentRow.implicitWidth + (Theme.spacingMd * 2)
        implicitHeight: 32
        radius: Theme.radiusSm
        color: btnMouse.pressed ? Theme.bgSurfaceActive : (btnMouse.containsMouse ? Theme.bgSurfaceHover : "transparent")
        border.color: btnMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle
        border.width: btnMouse.containsMouse ? 1 : 0

        Behavior on color { ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easingType } }
        Behavior on border.color { ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easingType } }

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: Theme.spacingXs

            Image {
                id: btnIcon
                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter
                source: btn.iconSource
                visible: btn.iconSource.length > 0
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Text {
                id: label
                anchors.verticalCenter: parent.verticalCenter
                color: btnMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Theme.fontWeightMedium
            }
        }

        ToolTip {
            id: toolTip
            visible: btnMouse.containsMouse && text.length > 0
            delay: 400
        }

        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }
    }

    // Left Navigation & Actions Group
    Row {
        id: leftGroup
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingLg
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacingMd

        // Brand Logo & Title
        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.spacingSm

            // Matrix Icon Mark
            Grid {
                anchors.verticalCenter: parent.verticalCenter
                columns: 2
                spacing: 2
                Repeater {
                    model: 4
                    Rectangle {
                        required property int index
                        width: 6
                        height: 6
                        radius: 1
                        color: index === 0 ? Theme.accentCyan : Theme.textMuted
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Screen Drop"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Theme.fontWeightDemiBold
            }
        }

        // Divider
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 20
            color: Theme.borderSubtle
        }

        // Active File Badge
        Rectangle {
            id: fileBadge
            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: fileBadgeRow.implicitWidth + (Theme.spacingSm * 2)
            implicitHeight: 28
            radius: Theme.radiusSm
            color: Theme.bgSurfaceHover
            border.color: root.isDirty ? Qt.rgba(Theme.accentAmber.r, Theme.accentAmber.g, Theme.accentAmber.b, 0.4) : Theme.borderSubtle
            border.width: 1

            Row {
                id: fileBadgeRow
                anchors.centerIn: parent
                spacing: Theme.spacingXs

                Text {
                    id: fileNameText
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.currentFileName.length > 0 ? root.currentFileName : "Untitled Layout"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightMedium
                }

                // Unsaved changes indicator dot
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 6
                    height: 6
                    radius: 3
                    color: Theme.accentAmber
                    visible: root.isDirty
                }
            }

            ToolTip {
                id: fileBadgeTooltip
                visible: fileBadgeMouse.containsMouse
                delay: 350
                text: root.isDirty
                      ? "Layout has unsaved modifications (Ctrl+S to save)"
                      : "Current layout file is saved"
            }

            MouseArea {
                id: fileBadgeMouse
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        // Divider
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 20
            color: Theme.borderSubtle
        }

        // File Operations
        HeaderButton {
            text: "New"
            tooltipText: "Create new empty layout (Ctrl+N)"
            onClicked: root.newLayoutClicked()
        }

        HeaderButton {
            text: "Open"
            tooltipText: "Open layout file (.sav) (Ctrl+O)"
            onClicked: root.openLayoutClicked()
        }

        HeaderButton {
            text: "Save"
            tooltipText: "Save current layout (Ctrl+S)"
            onClicked: root.saveLayoutClicked()
        }

        HeaderButton {
            text: "Save As"
            tooltipText: "Save layout to a new file (Ctrl+Shift+S)"
            onClicked: root.saveAsLayoutClicked()
        }
    }

    // Center Overlap Status Pill
    OverlapStatusPill {
        id: overlapPill
        anchors.centerIn: parent
        hasOverlaps: root.hasOverlaps
        invalidCount: root.invalidCount
    }

    // Right Action & Tools Group
    Row {
        id: rightGroup
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLg
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacingMd

        // Screen Topology Settings Button
        HeaderButton {
            text: "Displays"
            tooltipText: "Inspect and configure connected display topology"
            onClicked: root.screenSettingsClicked()
        }

        // Logs Diagnostics Button
        HeaderButton {
            text: "Logs"
            tooltipText: "Open diagnostic application logs directory"
            onClicked: root.openLogsClicked()
        }

        // Theme Mode Toggle Button
        HeaderButton {
            text: root.darkTheme ? "Dark" : "Light"
            tooltipText: "Toggle interface dark/light theme"
            onClicked: root.themeToggleClicked()
        }

        // Divider
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 24
            color: Theme.borderSubtle
        }

        // Prominent Glowing "Run Layout" Button
        Rectangle {
            id: runButton
            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: runButtonRow.implicitWidth + (Theme.spacingLg * 2)
            implicitHeight: 34
            radius: Theme.radiusMd

            // Glow aura background
            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                radius: Theme.radiusMd + 2
                color: "transparent"
                border.color: Theme.accentCyanGlow
                border.width: runMouseArea.containsMouse ? 2 : 1
                opacity: root.isRunning ? 0.3 : (runMouseArea.containsMouse ? 1.0 : 0.6)
                Behavior on opacity { NumberAnimation { duration: Theme.durationFast } }
            }

            color: root.isRunning
                   ? Qt.darker(Theme.accentCyan, 1.4)
                   : (runMouseArea.pressed ? Qt.darker(Theme.accentCyan, 1.1)
                                           : (runMouseArea.containsMouse ? Theme.accentCyanHover : Theme.accentCyan))

            Behavior on color { ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easingType } }

            Row {
                id: runButtonRow
                anchors.centerIn: parent
                spacing: Theme.spacingSm

                Image {
                    id: runIcon
                    width: 13
                    height: 13
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/icons/action-run.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: root.isRunning ? 0.5 : 1.0
                }

                Text {
                    id: runText
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.isRunning ? "Running..." : "Run Layout"
                    color: "#080B11" // Deep Obsidian for maximum contrast on Electric Cyan
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightBold
                }
            }

            ToolTip {
                id: runToolTip
                visible: runMouseArea.containsMouse
                delay: 350
                text: "Launch all placed applications and position windows (Ctrl+R)"
            }

            MouseArea {
                id: runMouseArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: !root.isRunning
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: root.runLayoutClicked()
            }
        }
    }
}
