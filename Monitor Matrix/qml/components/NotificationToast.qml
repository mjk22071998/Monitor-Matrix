import QtQuick
import "../theme"

Item {
    id: root

    property string toastTitle: ""
    property string toastMessage: ""
    property bool isError: false
    property bool isDisplayed: false

    implicitWidth: Math.min(380, cardRect.implicitWidth)
    implicitHeight: cardRect.implicitHeight
    visible: opacity > 0.0
    opacity: isDisplayed ? 1.0 : 0.0
    y: isDisplayed ? 0 : -16

    Behavior on opacity {
        NumberAnimation { duration: Theme.durationNormal; easing.type: Theme.easingType }
    }
    Behavior on y {
        NumberAnimation { duration: Theme.durationNormal; easing.type: Theme.easingType }
    }

    function show(title, message, error) {
        toastTitle = title || "";
        toastMessage = message || "";
        isError = error === true;
        isDisplayed = true;
        dismissTimer.restart();
    }

    function dismiss() {
        dismissTimer.stop();
        isDisplayed = false;
    }

    Timer {
        id: dismissTimer
        interval: 4000
        repeat: false
        onTriggered: root.dismiss()
    }

    Rectangle {
        id: cardRect
        anchors.fill: parent
        implicitWidth: 340
        implicitHeight: contentColumn.implicitHeight + (Theme.spacingMd * 2)
        radius: Theme.radiusMd
        color: Theme.bgSurface
        border.color: root.isError
                      ? Qt.rgba(Theme.accentCrimson.r, Theme.accentCrimson.g, Theme.accentCrimson.b, 0.45)
                      : Theme.borderSubtle
        border.width: 1
        clip: true

        // Left accent indicator stripe
        Rectangle {
            id: accentBar
            width: 4
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: root.isError ? Theme.accentCrimson : Theme.accentCyan
        }

        Row {
            id: contentRow
            anchors.fill: parent
            anchors.leftMargin: Theme.spacingMd + 4
            anchors.rightMargin: Theme.spacingSm
            anchors.topMargin: Theme.spacingMd
            anchors.bottomMargin: Theme.spacingMd
            spacing: Theme.spacingMd

            Image {
                id: iconImage
                width: 18
                height: 18
                anchors.top: parent.top
                anchors.topMargin: 2
                source: root.isError ? "qrc:/icons/status-warning.svg" : "qrc:/icons/status-check.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Column {
                id: contentColumn
                width: parent.width - iconImage.width - closeButton.width - (Theme.spacingMd * 2)
                spacing: Theme.spacing2Xs

                Text {
                    id: titleText
                    width: parent.width
                    text: root.toastTitle
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeRegular
                    font.weight: Theme.fontWeightMedium
                    elide: Text.ElideRight
                    visible: text.length > 0
                }

                Text {
                    id: messageText
                    width: parent.width
                    text: root.toastMessage
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }
            }

            // Dismiss Button
            Item {
                id: closeButton
                width: 24
                height: 24
                anchors.top: parent.top

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusSm
                    color: closeMouseArea.containsMouse ? Theme.bgSurfaceHover : "transparent"
                }

                Text {
                    anchors.centerIn: parent
                    text: "×"
                    color: closeMouseArea.containsMouse ? Theme.textPrimary : Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeLarge
                    font.weight: Theme.fontWeightNormal
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.dismiss()
                }
            }
        }
    }
}
