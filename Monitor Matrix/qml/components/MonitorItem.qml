// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import "../theme"

Item {
    id: root

    // Monitor Physical Topology & Specifications
    property string deviceName: ""
    property string modelName: ""
    property real rectX: 0
    property real rectY: 0
    property real rectWidth: 1920
    property real rectHeight: 1080
    property real availX: 0
    property real availY: 0
    property real availWidth: 1920
    property real availHeight: 1040
    property bool isPrimary: false
    property real scaleFactor: 1.0
    property real zoomScale: 1.0
    property var placementModel: null
    property string selectedPlacementId: ""
    property int displayIndex: 1

    width: rectWidth
    height: rectHeight

    // Snapping Guide State
    property bool vGuideVisible: false
    property real vGuideX: 0
    property bool hGuideVisible: false
    property real hGuideY: 0

    // Zoom-Independent Visual Tokens
    readonly property real invZoom: Math.max(0.05, root.zoomScale)
    readonly property real bezelBorderWidth: Math.max(2, Math.min(8, Math.round(3 / invZoom)))
    readonly property real cornerRadius: Math.max(4, Math.min(16, Math.round(8 / invZoom)))
    readonly property real headerFontSize: Math.max(10, Math.min(22, Math.round(12 / invZoom)))
    readonly property real badgeFontSize: Math.max(8, Math.min(18, Math.round(10 / invZoom)))
    readonly property real watermarkFontSize: Math.max(36, Math.min(180, Math.round(96 / invZoom)))
    readonly property real pillHeight: Math.max(16, Math.min(36, Math.round(22 / invZoom)))

    // Outer Metallic Frame & Bezel
    Rectangle {
        id: metallicBezel
        anchors.fill: parent
        radius: root.cornerRadius
        color: Theme.isDark ? "#0A0E18" : "#F8FAFC"

        // Metallic frame border
        border.color: root.isPrimary ? Theme.accentCyan : (Theme.isDark ? "#28354D" : "#CBD5E1")
        border.width: root.bezelBorderWidth

        // Subtle Inner Bezel Inset
        Rectangle {
            anchors.fill: parent
            anchors.margins: root.bezelBorderWidth
            radius: Math.max(2, root.cornerRadius - 2)
            color: "transparent"
            border.color: Theme.isDark ? "#131C2E" : "#E2E8F0"
            border.width: 1
        }

        // Center Watermark Display Index Number
        Text {
            anchors.centerIn: parent
            text: root.displayIndex.toString()
            font.family: Theme.fontFamily
            font.pixelSize: root.watermarkFontSize
            font.weight: Theme.fontWeightBold
            color: Theme.isDark ? "#FFFFFF" : "#000000"
            opacity: Theme.isDark ? 0.04 : 0.03
            z: 0
        }

        // Top Status Header (Wallpaper Info Bar)
        Item {
            id: headerArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: root.bezelBorderWidth + Math.max(4, Math.round(8 / root.invZoom))
            anchors.leftMargin: root.bezelBorderWidth + Math.max(6, Math.round(12 / root.invZoom))
            anchors.rightMargin: root.bezelBorderWidth + Math.max(6, Math.round(12 / root.invZoom))
            height: root.pillHeight
            z: 1

            // Left Cluster: Display Name & Primary Pill
            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: Math.max(4, Math.round(8 / root.invZoom))

                // Display Identification Pill
                Rectangle {
                    height: root.pillHeight
                    width: displayTitleText.implicitWidth + Math.max(10, Math.round(16 / root.invZoom))
                    radius: root.pillHeight / 2
                    color: Theme.isDark ? "#141D2D" : "#E2E8F0"
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: Math.max(3, Math.round(6 / root.invZoom))

                        Text {
                            text: "🖥"
                            font.pixelSize: root.badgeFontSize
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            id: displayTitleText
                            text: root.modelName.length > 0 ? root.modelName : root.deviceName
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: root.headerFontSize
                            font.weight: Theme.fontWeightDemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                // Primary Display Badge
                Rectangle {
                    visible: root.isPrimary
                    height: root.pillHeight
                    width: primaryBadgeText.implicitWidth + Math.max(8, Math.round(14 / root.invZoom))
                    radius: root.pillHeight / 2
                    color: Theme.accentAmberMuted
                    border.color: Theme.accentAmber
                    border.width: 1

                    Text {
                        id: primaryBadgeText
                        anchors.centerIn: parent
                        text: "★ PRIMARY"
                        color: Theme.accentAmber
                        font.family: Theme.fontFamily
                        font.pixelSize: root.badgeFontSize
                        font.weight: Theme.fontWeightBold
                    }
                }
            }

            // Right Cluster: Resolution & Scale Factor Tags
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: Math.max(4, Math.round(6 / root.invZoom))

                // Resolution Tag
                Rectangle {
                    height: root.pillHeight
                    width: resText.implicitWidth + Math.max(8, Math.round(14 / root.invZoom))
                    radius: Theme.radiusSm
                    color: Theme.isDark ? "#141D2D" : "#E2E8F0"
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Text {
                        id: resText
                        anchors.centerIn: parent
                        text: Math.round(root.rectWidth) + " × " + Math.round(root.rectHeight)
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: root.badgeFontSize
                    }
                }

                // DPI Scale Tag
                Rectangle {
                    height: root.pillHeight
                    width: dpiText.implicitWidth + Math.max(8, Math.round(12 / root.invZoom))
                    radius: Theme.radiusSm
                    color: Theme.isDark ? "#141D2D" : "#E2E8F0"
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Text {
                        id: dpiText
                        anchors.centerIn: parent
                        text: Math.round(root.scaleFactor * 100) + "% DPI"
                        color: Theme.textMuted
                        font.family: Theme.fontMono
                        font.pixelSize: root.badgeFontSize
                    }
                }
            }
        }
    }

    // Placed Applications Layer
    Repeater {
        id: placementsRepeater
        model: root.placementModel

        delegate: PlacedAppItem {
            id: placedApp
            required property var model

            placementId: model.placementId
            appId: model.appId
            appName: model.appName
            appType: model.appType
            rectX: model.rectX
            rectY: model.rectY
            rectW: model.rectW
            rectH: model.rectH
            hasOverlap: model.hasOverlap

            visible: model.targetDeviceName === root.deviceName
            enabled: visible

            monitorWidth: root.rectWidth
            monitorHeight: root.rectHeight
            zoomScale: root.zoomScale
            isSelected: root.selectedPlacementId.length > 0 && root.selectedPlacementId === placedApp.placementId

            onSelectRequested: root.selectedPlacementId = placedApp.placementId

            onSnapGuidesUpdated: (vVis, vX, hVis, hY) => {
                root.vGuideVisible = vVis;
                root.vGuideX = vX;
                root.hGuideVisible = hVis;
                root.hGuideY = hY;
            }

            onRemoveRequested: (id) => {
                if (root.placementModel) {
                    root.placementModel.removePlacement(id);
                }
            }

            onRectCommitted: (id, newRect) => {
                if (root.placementModel) {
                    root.placementModel.updatePlacementRect(id, newRect);
                }
            }
        }
    }

    // Magnetic Snapping Guides
    Rectangle {
        id: vGuideLine
        visible: root.vGuideVisible
        x: root.vGuideX - width / 2
        y: 0
        width: Math.max(1, Math.round(2 / root.invZoom))
        height: parent.height
        color: Theme.accentCyan
        opacity: 0.85
        z: 40
    }

    Rectangle {
        id: hGuideLine
        visible: root.hGuideVisible
        x: 0
        y: root.hGuideY - height / 2
        width: parent.width
        height: Math.max(1, Math.round(2 / root.invZoom))
        color: Theme.accentCyan
        opacity: 0.85
        z: 40
    }

    // Drop Area for Ingesting Catalog Items or Repositioning
    DropArea {
        id: monitorDropArea
        anchors.fill: parent
        z: 30
        keys: ["application", "app", "Executable", "URL"]

        property int activePositionEnum: 0
        property string activeZoneName: "Full Screen"
        property rect activeZoneRect: Qt.rect(0, 0, root.rectWidth, root.rectHeight)

        onEntered: (drag) => {
            updateDropZone(drag.x, drag.y);
        }

        onPositionChanged: (drag) => {
            updateDropZone(drag.x, drag.y);
        }

        onExited: {
            activePositionEnum = 0;
            activeZoneName = "Full Screen";
            activeZoneRect = Qt.rect(0, 0, root.rectWidth, root.rectHeight);
        }

        onDropped: (drop) => {
            var id = "";
            var name = "";
            var type = "Application";

            if (drop.formats && drop.formats.indexOf("application/x-app-id") !== -1) {
                id = drop.getDataAsString("application/x-app-id");
                name = drop.getDataAsString("application/x-app-name");
                type = drop.getDataAsString("application/x-app-type");
            } else if (drop.source) {
                var src = drop.source;
                id = src["appId"] ? src["appId"].toString() : "";
                name = src["appName"] ? src["appName"].toString() : "";
                type = src["appType"] ? src["appType"].toString() : "Application";
            }

            var targetDev = (root.deviceName && root.deviceName.length > 0) ? root.deviceName : "";
            if (id && id.length > 0 && targetDev.length > 0 && root.placementModel) {
                root.placementModel.placeApp(
                    id,
                    name.length > 0 ? name : id,
                    targetDev,
                    activePositionEnum,
                    activeZoneRect,
                    type.length > 0 ? type : "Application"
                );
            }

            drop.accept();
        }

        function updateDropZone(dragX, dragY) {
            var relX = dragX / root.rectWidth;
            var relY = dragY / root.rectHeight;

            // Center: Full Screen
            if (relX >= 0.28 && relX <= 0.72 && relY >= 0.28 && relY <= 0.72) {
                activePositionEnum = 0;
                activeZoneName = "Full Screen";
                activeZoneRect = Qt.rect(0, 0, root.rectWidth, root.rectHeight);
                return;
            }

            // Left Side
            if (relX < 0.28) {
                if (relY < 0.25) {
                    activePositionEnum = 5;
                    activeZoneName = "Top-Left";
                    activeZoneRect = Qt.rect(0, 0, root.rectWidth / 2, root.rectHeight / 2);
                } else if (relY > 0.75) {
                    activePositionEnum = 7;
                    activeZoneName = "Bottom-Left";
                    activeZoneRect = Qt.rect(0, root.rectHeight / 2, root.rectWidth / 2, root.rectHeight / 2);
                } else {
                    activePositionEnum = 1;
                    activeZoneName = "Left Half";
                    activeZoneRect = Qt.rect(0, 0, root.rectWidth / 2, root.rectHeight);
                }
                return;
            }

            // Right Side
            if (relX > 0.72) {
                if (relY < 0.25) {
                    activePositionEnum = 6;
                    activeZoneName = "Top-Right";
                    activeZoneRect = Qt.rect(root.rectWidth / 2, 0, root.rectWidth / 2, root.rectHeight / 2);
                } else if (relY > 0.75) {
                    activePositionEnum = 8;
                    activeZoneName = "Bottom-Right";
                    activeZoneRect = Qt.rect(root.rectWidth / 2, root.rectHeight / 2, root.rectWidth / 2, root.rectHeight / 2);
                } else {
                    activePositionEnum = 2;
                    activeZoneName = "Right Half";
                    activeZoneRect = Qt.rect(root.rectWidth / 2, 0, root.rectWidth / 2, root.rectHeight);
                }
                return;
            }

            // Top / Bottom Edges
            if (relY < 0.28) {
                activePositionEnum = 3;
                activeZoneName = "Top Half";
                activeZoneRect = Qt.rect(0, 0, root.rectWidth, root.rectHeight / 2);
            } else if (relY > 0.72) {
                activePositionEnum = 4;
                activeZoneName = "Bottom Half";
                activeZoneRect = Qt.rect(0, root.rectHeight / 2, root.rectWidth, root.rectHeight / 2);
            } else {
                activePositionEnum = 0;
                activeZoneName = "Full Screen";
                activeZoneRect = Qt.rect(0, 0, root.rectWidth, root.rectHeight);
            }
        }
    }

    // Drop Preview Hologram Overlay
    Rectangle {
        id: dropPreviewOverlay
        visible: monitorDropArea.containsDrag
        x: monitorDropArea.activeZoneRect.x
        y: monitorDropArea.activeZoneRect.y
        width: monitorDropArea.activeZoneRect.width
        height: monitorDropArea.activeZoneRect.height
        radius: root.cornerRadius
        z: 35

        color: Theme.accentCyanGlow
        border.color: Theme.accentCyan
        border.width: Math.max(2, Math.round(3 / root.invZoom))

        // Center Zone Indicator Badge
        Rectangle {
            anchors.centerIn: parent
            height: Math.max(22, Math.min(50, Math.round(32 / root.invZoom)))
            width: zoneLabel.implicitWidth + Math.max(16, Math.round(24 / root.invZoom))
            radius: height / 2
            color: Theme.isDark ? "#101B2E" : "#FFFFFF"
            border.color: Theme.accentCyan
            border.width: Math.max(1, Math.round(1.5 / root.invZoom))

            Text {
                id: zoneLabel
                anchors.centerIn: parent
                text: monitorDropArea.activeZoneName
                color: Theme.accentCyan
                font.family: Theme.fontFamily
                font.pixelSize: Math.max(11, Math.min(24, Math.round(14 / root.invZoom)))
                font.weight: Theme.fontWeightBold
            }
        }
    }
}
