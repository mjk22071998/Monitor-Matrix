// qmllint disable unqualified
import QtQuick
import "../theme"
import "../components"

Rectangle {
    id: root

    // Models & Configuration
    property var canvasModel: null
    property var placementModel: null
    property real zoomScale: 0.35
    property real minZoom: 0.05
    property real maxZoom: 3.0
    property string selectedPlacementId: ""

    // Desktop Bounds Geometry
    property real minDeskX: 0
    property real minDeskY: 0
    property real deskWidth: 1920
    property real deskHeight: 1080
    readonly property real canvasPadding: 200

    color: Theme.bgCanvas
    clip: true

    // Dotted Blueprint Grid Background
    Canvas {
        id: dottedCanvas
        anchors.fill: parent
        z: 0

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = Theme.isDark ? "#1C273C" : "#CBD5E1";
            var spacing = 28;
            var dotSize = 2;
            for (var x = spacing / 2; x < width; x += spacing) {
                for (var y = spacing / 2; y < height; y += spacing) {
                    ctx.fillRect(x, y, dotSize, dotSize);
                }
            }
        }

        Connections {
            target: Theme
            function onIsDarkChanged() {
                dottedCanvas.requestPaint();
            }
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    // Interactive Zoomable & Pannable Viewport
    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: Math.max(width, (root.deskWidth + root.canvasPadding * 2) * root.zoomScale)
        contentHeight: Math.max(height, (root.deskHeight + root.canvasPadding * 2) * root.zoomScale)

        // Mouse Drag Panning on Empty Canvas
        MouseArea {
            id: panMouseArea
            anchors.fill: parent
            z: 0
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

            property real lastPressX: 0
            property real lastPressY: 0

            onPressed: (mouse) => {
                lastPressX = mouse.x;
                lastPressY = mouse.y;
                root.selectedPlacementId = "";
            }

            onPositionChanged: (mouse) => {
                if (pressed) {
                    var dx = mouse.x - lastPressX;
                    var dy = mouse.y - lastPressY;
                    flickable.contentX = Math.max(0, Math.min(flickable.contentWidth - flickable.width, flickable.contentX - dx));
                    flickable.contentY = Math.max(0, Math.min(flickable.contentHeight - flickable.height, flickable.contentY - dy));
                    lastPressX = mouse.x;
                    lastPressY = mouse.y;
                }
            }
        }

        // Virtual Desktop World Container
        Item {
            id: contentCenteringItem
            width: flickable.contentWidth
            height: flickable.contentHeight

            Item {
                id: scaledWorld
                x: (parent.width - root.deskWidth * root.zoomScale) / 2
                y: (parent.height - root.deskHeight * root.zoomScale) / 2
                width: root.deskWidth
                height: root.deskHeight
                scale: root.zoomScale
                transformOrigin: Item.TopLeft
                z: 1

                // Multi-Monitor Displays
                Repeater {
                    id: monitorRepeater
                    model: root.canvasModel

                    delegate: MonitorItem {
                        required property int index
                        required property var model

                        x: model.rectX - root.minDeskX
                        y: model.rectY - root.minDeskY
                        width: model.rectWidth
                        height: model.rectHeight

                        deviceName: model.deviceName
                        modelName: model.modelName
                        rectX: model.rectX
                        rectY: model.rectY
                        rectWidth: model.rectWidth
                        rectHeight: model.rectHeight
                        availX: model.availX
                        availY: model.availY
                        availWidth: model.availWidth
                        availHeight: model.availHeight
                        isPrimary: model.isPrimary
                        scaleFactor: model.scaleFactor

                        displayIndex: index + 1
                        zoomScale: root.zoomScale
                        placementModel: root.placementModel
                        selectedPlacementId: root.selectedPlacementId
                    }
                }
            }
        }
    }

    // Mouse Wheel Zoom Handler
    WheelHandler {
        id: wheelHandler
        target: null
        onWheel: (event) => {
            if (event.angleDelta.y > 0) {
                root.zoomIn();
            } else if (event.angleDelta.y < 0) {
                root.zoomOut();
            }
        }
    }

    // Empty State When No Displays are Detected
    Item {
        anchors.centerIn: parent
        visible: !root.canvasModel || root.canvasModel.count === 0
        z: 50

        Column {
            anchors.centerIn: parent
            spacing: Theme.spacingMd

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "🖥 No Displays Detected"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeHeader
                font.weight: Theme.fontWeightBold
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Connect displays or configure display topology in Windows settings."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeRegular
            }
        }
    }

    // Floating Canvas Navigation Controls
    CanvasControls {
        id: canvasControls
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.spacingLg
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLg
        z: 100

        zoomScale: root.zoomScale
        minZoom: root.minZoom
        maxZoom: root.maxZoom

        onZoomInRequested: root.zoomIn()
        onZoomOutRequested: root.zoomOut()
        onResetZoomRequested: root.resetZoom()
        onFitToViewRequested: root.fitToView()
    }

    // Zoom & Pan Controller Functions
    function zoomIn() {
        var newScale = Math.min(root.maxZoom, root.zoomScale * 1.2);
        adjustZoomCentered(newScale);
    }

    function zoomOut() {
        var newScale = Math.max(root.minZoom, root.zoomScale / 1.2);
        adjustZoomCentered(newScale);
    }

    function resetZoom() {
        adjustZoomCentered(1.0);
    }

    function adjustZoomCentered(newScale) {
        var oldScale = root.zoomScale;
        if (Math.abs(oldScale - newScale) < 0.001) return;

        // Keep viewport center stable while zooming
        var viewCenterX = flickable.contentX + flickable.width / 2;
        var viewCenterY = flickable.contentY + flickable.height / 2;

        root.zoomScale = newScale;

        var scaleRatio = newScale / oldScale;
        flickable.contentX = Math.max(0, Math.min(flickable.contentWidth - flickable.width, viewCenterX * scaleRatio - flickable.width / 2));
        flickable.contentY = Math.max(0, Math.min(flickable.contentHeight - flickable.height, viewCenterY * scaleRatio - flickable.height / 2));
    }

    function fitToView() {
        updateBounds();
        if (flickable.width <= 0 || flickable.height <= 0) return;

        var pad = 80;
        var availW = Math.max(100, flickable.width - pad * 2);
        var availH = Math.max(100, flickable.height - pad * 2);

        var targetScale = Math.min(availW / root.deskWidth, availH / root.deskHeight);
        targetScale = Math.max(root.minZoom, Math.min(1.0, targetScale));
        root.zoomScale = targetScale;

        flickable.contentX = (flickable.contentWidth - flickable.width) / 2;
        flickable.contentY = (flickable.contentHeight - flickable.height) / 2;
    }

    function updateBounds() {
        if (!root.canvasModel || root.canvasModel.count === 0) {
            root.minDeskX = 0;
            root.minDeskY = 0;
            root.deskWidth = 1920;
            root.deskHeight = 1080;
            return;
        }

        var minX = Infinity;
        var minY = Infinity;
        var maxX = -Infinity;
        var maxY = -Infinity;

        for (var i = 0; i < root.canvasModel.count; ++i) {
            var m = root.canvasModel.getMonitor(i);
            var x = (typeof m.rectX !== "undefined") ? m.rectX : 0;
            var y = (typeof m.rectY !== "undefined") ? m.rectY : 0;
            var w = (typeof m.rectWidth !== "undefined") ? m.rectWidth : 1920;
            var h = (typeof m.rectHeight !== "undefined") ? m.rectHeight : 1080;

            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (x + w > maxX) maxX = x + w;
            if (y + h > maxY) maxY = y + h;
        }

        if (minX === Infinity) {
            minX = 0;
            minY = 0;
            maxX = 1920;
            maxY = 1080;
        }

        root.minDeskX = minX;
        root.minDeskY = minY;
        root.deskWidth = Math.max(200, maxX - minX);
        root.deskHeight = Math.max(200, maxY - minY);
    }

    Component.onCompleted: {
        updateBounds();
        fitToView();
    }

    Connections {
        target: root.canvasModel
        function onMonitorsChanged() {
            root.updateBounds();
            root.fitToView();
        }
    }
}
