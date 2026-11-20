import QtQuick
import QtQuick.Controls
import "../theme"

Rectangle {
    id: root

    property bool hasOverlaps: false
    property int invalidCount: 0

    implicitWidth: contentRow.implicitWidth + Theme.spacingLg
    implicitHeight: 28
    radius: Theme.radiusPill

    color: root.hasOverlaps
           ? Qt.rgba(Theme.accentCrimson.r, Theme.accentCrimson.g, Theme.accentCrimson.b, 0.16)
           : Qt.rgba(Theme.accentEmerald.r, Theme.accentEmerald.g, Theme.accentEmerald.b, 0.16)

    border.color: root.hasOverlaps
                  ? Qt.rgba(Theme.accentCrimson.r, Theme.accentCrimson.g, Theme.accentCrimson.b, 0.5)
                  : Qt.rgba(Theme.accentEmerald.r, Theme.accentEmerald.g, Theme.accentEmerald.b, 0.45)
    border.width: 1

    Behavior on color {
        ColorAnimation { duration: Theme.durationNormal; easing.type: Theme.easingType }
    }
    Behavior on border.color {
        ColorAnimation { duration: Theme.durationNormal; easing.type: Theme.easingType }
    }

    SequentialAnimation {
        id: pulseAnim
        running: root.hasOverlaps
        loops: Animation.Infinite
        NumberAnimation { target: root; property: "opacity"; to: 0.55; duration: 700; easing.type: Easing.InOutSine }
        NumberAnimation { target: root; property: "opacity"; to: 1.0; duration: 700; easing.type: Easing.InOutSine }
    }

    onHasOverlapsChanged: {
        if (!hasOverlaps) {
            pulseAnim.stop();
            root.opacity = 1.0;
        }
    }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: Theme.spacingSm

        Image {
            id: statusIcon
            width: 14
            height: 14
            anchors.verticalCenter: parent.verticalCenter
            source: root.hasOverlaps ? "qrc:/icons/status-warning.svg" : "qrc:/icons/status-check.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Text {
            id: statusText
            anchors.verticalCenter: parent.verticalCenter
            text: root.hasOverlaps
                  ? (root.invalidCount > 0
                     ? root.invalidCount + (root.invalidCount === 1 ? " Overlap" : " Overlaps")
                     : "Overlaps Detected")
                  : "Layout Valid"
            color: root.hasOverlaps ? Theme.accentCrimson : Theme.accentEmerald
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeSmall
            font.weight: Theme.fontWeightMedium

            Behavior on color {
                ColorAnimation { duration: Theme.durationNormal; easing.type: Theme.easingType }
            }
        }
    }

    ToolTip.visible: mouseArea.containsMouse
    ToolTip.delay: 350
    ToolTip.text: root.hasOverlaps
                  ? "Colliding app boundaries detected on canvas. Adjust placements to prevent overlapping windows."
                  : "All placed application boundaries are cleanly separated."

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
