// qmllint disable unqualified
import QtQuick
import "../theme"

Rectangle {
    id: root

    // Zoom Level & Boundary Constraints
    property real zoomScale: 1.0
    property real minZoom: 0.05
    property real maxZoom: 3.0
    property real zoomStep: 0.1

    // Action Signals
    signal zoomInRequested()
    signal zoomOutRequested()
    signal resetZoomRequested()
    signal fitToViewRequested()

    implicitWidth: controlsRow.implicitWidth + 24
    implicitHeight: 38
    radius: Theme.radiusPill
    color: Theme.bgSurfaceElevated
    opacity: 0.96

    border.color: Theme.borderStrong
    border.width: 1

    // Drop shadow glow
    Rectangle {
        anchors.fill: parent
        anchors.margins: -1
        radius: root.radius + 1
        color: "transparent"
        border.color: Theme.isDark ? "#0A101D" : "#E2E8F0"
        border.width: 1
        z: -1
    }

    Row {
        id: controlsRow
        anchors.centerIn: parent
        spacing: Theme.spacingXs

        // 1. Zoom Out Button ("−")
        Rectangle {
            id: zoomOutBtn
            width: 28
            height: 28
            radius: width / 2
            anchors.verticalCenter: parent.verticalCenter
            color: zoomOutMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"

            Text {
                anchors.centerIn: parent
                text: "−"
                color: zoomOutMouse.containsMouse ? Theme.accentCyan : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeRegular
                font.weight: Theme.fontWeightBold
            }

            MouseArea {
                id: zoomOutMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.zoomOutRequested()
            }
        }

        // 2. Zoom Percentage Label & Reset to 100% Button
        Rectangle {
            id: zoomPercentBtn
            width: zoomLabel.implicitWidth + 14
            height: 26
            radius: Theme.radiusSm
            anchors.verticalCenter: parent.verticalCenter
            color: zoomPercentMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"
            border.color: zoomPercentMouse.containsMouse ? Theme.borderSubtle : "transparent"
            border.width: 1

            Text {
                id: zoomLabel
                anchors.centerIn: parent
                text: Math.round(root.zoomScale * 100) + "%"
                color: zoomPercentMouse.containsMouse ? Theme.accentCyan : Theme.textPrimary
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Theme.fontWeightDemiBold
            }

            MouseArea {
                id: zoomPercentMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.resetZoomRequested()
            }
        }

        // 3. Zoom In Button ("+")
        Rectangle {
            id: zoomInBtn
            width: 28
            height: 28
            radius: width / 2
            anchors.verticalCenter: parent.verticalCenter
            color: zoomInMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"

            Text {
                anchors.centerIn: parent
                text: "+"
                color: zoomInMouse.containsMouse ? Theme.accentCyan : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeRegular
                font.weight: Theme.fontWeightBold
            }

            MouseArea {
                id: zoomInMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.zoomInRequested()
            }
        }

        // Subtle Vertical Divider Line
        Rectangle {
            width: 1
            height: 18
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.borderSubtle
        }

        // 4. Fit All Monitors / Fit to View Button
        Rectangle {
            id: fitBtn
            width: fitRow.implicitWidth + 12
            height: 26
            radius: Theme.radiusSm
            anchors.verticalCenter: parent.verticalCenter
            color: fitMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"
            border.color: fitMouse.containsMouse ? Theme.borderSubtle : "transparent"
            border.width: 1

            Row {
                id: fitRow
                anchors.centerIn: parent
                spacing: 4

                Text {
                    text: "⛶"
                    anchors.verticalCenter: parent.verticalCenter
                    color: fitMouse.containsMouse ? Theme.accentCyan : Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                Text {
                    text: "Fit to View"
                    anchors.verticalCenter: parent.verticalCenter
                    color: fitMouse.containsMouse ? Theme.accentCyan : Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightMedium
                }
            }

            MouseArea {
                id: fitMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.fitToViewRequested()
            }
        }
    }
}
