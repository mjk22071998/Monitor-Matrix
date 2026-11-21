// qmllint disable unqualified
import QtQuick
import "../theme"

Item {
    id: root

    // Placement Model Identifiers & Data
    property string placementId: ""
    property string appId: ""
    property string appName: ""
    property string appType: "Application"
    property bool hasOverlap: false
    property real rectX: 0
    property real rectY: 0
    property real rectW: 400
    property real rectH: 300
    property real monitorWidth: 1920
    property real monitorHeight: 1080
    property real zoomScale: 1.0
    property bool isSelected: false
    property real minWidth: 40
    property real minHeight: 40
    property real snapThreshold: 14

    // Interaction State
    property bool isDragging: false
    property bool isResizing: false
    readonly property bool isInteracting: isDragging || isResizing

    // Signals
    signal removeRequested(string id)
    signal rectCommitted(string id, rect r)
    signal selectRequested()
    signal snapGuidesUpdated(bool vVisible, real vX, bool hVisible, real hY)

    // Two-way model binding guards: Pause bindings while interacting
    Binding {
        target: root
        property: "x"
        value: root.rectX
        when: !root.isInteracting
    }
    Binding {
        target: root
        property: "y"
        value: root.rectY
        when: !root.isInteracting
    }
    Binding {
        target: root
        property: "width"
        value: root.rectW
        when: !root.isInteracting
    }
    Binding {
        target: root
        property: "height"
        value: root.rectH
        when: !root.isInteracting
    }

    z: root.isSelected || root.isInteracting ? 10 : 2

    // Dynamic Sizing Tokens for Zoom-Independent Visuals
    readonly property real invZoom: Math.max(0.05, root.zoomScale)
    readonly property real handleSize: Math.max(8, Math.min(28, Math.round(11 / invZoom)))
    readonly property real handleBorderWidth: Math.max(1, Math.round(1.5 / invZoom))
    readonly property real cardBorderWidth: root.hasOverlap
        ? Math.max(2, Math.round(2.5 / invZoom))
        : (root.isInteracting || root.isSelected ? Math.max(1.5, Math.round(2 / invZoom)) : 1)
    readonly property real closeBtnSize: Math.max(18, Math.min(36, Math.round(22 / invZoom)))
    readonly property real headerFontSize: Math.max(10, Math.min(22, Math.round(12 / invZoom)))
    readonly property real badgeFontSize: Math.max(8, Math.min(18, Math.round(9 / invZoom)))
    readonly property real cornerRadius: Math.max(3, Math.round(6 / invZoom))

    // Placed App Card Body
    Rectangle {
        id: cardBody
        anchors.fill: parent
        radius: root.cornerRadius
        color: Theme.isDark ? "#172033" : "#FFFFFF"
        opacity: 0.94

        border.color: {
            if (root.hasOverlap) return Theme.accentCrimson;
            if (root.isInteracting || root.isSelected) return Theme.accentCyan;
            if (cardMouseArea.containsMouse) return Theme.borderStrong;
            return Theme.borderSubtle;
        }
        border.width: root.cardBorderWidth

        // Collision Glow Shadow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -root.cardBorderWidth
            radius: cardBody.radius + 1
            color: "transparent"
            border.color: Theme.accentCrimson
            border.width: Math.max(1, Math.round(2 / root.invZoom))
            opacity: root.hasOverlap ? 0.45 : 0.0
            visible: root.hasOverlap
            z: -1
        }

        // Inner Content Layout
        Item {
            anchors.fill: parent
            anchors.margins: Math.max(4, Math.round(6 / root.invZoom))
            clip: true

            // Header Bar inside Card
            Row {
                id: headerRow
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: root.closeBtnSize + Math.max(4, Math.round(6 / root.invZoom))
                height: root.closeBtnSize
                spacing: Math.max(4, Math.round(6 / root.invZoom))

                // App Type Badge Pill
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    height: Math.max(12, Math.round(16 / root.invZoom))
                    width: typeText.implicitWidth + Math.max(6, Math.round(8 / root.invZoom))
                    radius: height / 2
                    color: root.appType === "URL" ? Theme.tagUrl : Theme.tagExecutable

                    Text {
                        id: typeText
                        anchors.centerIn: parent
                        text: root.appType === "URL" ? "URL" : "EXE"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: root.badgeFontSize
                        font.weight: Theme.fontWeightBold
                    }
                }

                // App Name Label
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.min(implicitWidth, headerRow.width - typeText.width - (root.hasOverlap ? overlapPill.width : 0) - 20)
                    text: root.appName.length > 0 ? root.appName : "Application"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: root.headerFontSize
                    font.weight: Theme.fontWeightDemiBold
                    elide: Text.ElideRight
                }

                // Overlap Warning Badge
                Rectangle {
                    id: overlapPill
                    visible: root.hasOverlap
                    anchors.verticalCenter: parent.verticalCenter
                    height: Math.max(12, Math.round(16 / root.invZoom))
                    width: overlapText.implicitWidth + Math.max(6, Math.round(8 / root.invZoom))
                    radius: height / 2
                    color: Theme.accentCrimsonMuted
                    border.color: Theme.accentCrimson
                    border.width: 1

                    Text {
                        id: overlapText
                        anchors.centerIn: parent
                        text: "⚠ COLLISION"
                        color: Theme.accentCrimson
                        font.family: Theme.fontFamily
                        font.pixelSize: root.badgeFontSize
                        font.weight: Theme.fontWeightBold
                    }
                }
            }

            // Dimension Badge (Bottom Right)
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                height: Math.max(12, Math.round(16 / root.invZoom))
                width: dimText.implicitWidth + Math.max(6, Math.round(8 / root.invZoom))
                radius: Theme.radiusSm
                color: Theme.isDark ? "#0D131F" : "#F1F5F9"
                border.color: Theme.borderSubtle
                border.width: 1

                Text {
                    id: dimText
                    anchors.centerIn: parent
                    text: Math.round(root.width) + " × " + Math.round(root.height)
                    color: Theme.textMuted
                    font.family: Theme.fontMono
                    font.pixelSize: root.badgeFontSize
                }
            }
        }
    }

    // Close Button (Top-Right, high z-order)
    Rectangle {
        id: closeButton
        x: root.width - width - Math.max(3, Math.round(4 / root.invZoom))
        y: Math.max(3, Math.round(4 / root.invZoom))
        width: root.closeBtnSize
        height: root.closeBtnSize
        radius: width / 2
        z: 20
        color: closeMouseArea.containsMouse ? Theme.accentCrimsonMuted : "transparent"
        border.color: closeMouseArea.containsMouse ? Theme.accentCrimson : "transparent"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "✕"
            color: closeMouseArea.containsMouse ? Theme.accentCrimson : Theme.textMuted
            font.pixelSize: Math.max(9, Math.round(11 / root.invZoom))
            font.weight: Theme.fontWeightBold
        }

        MouseArea {
            id: closeMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            preventStealing: true
            onClicked: {
                root.removeRequested(root.placementId);
                if (typeof placementModel !== "undefined" && placementModel) {
                    placementModel.removePlacement(root.placementId);
                }
            }
        }
    }

    // Central Drag Area for Repositioning with Magnetic Snapping
    MouseArea {
        id: cardMouseArea
        anchors.fill: parent
        z: 1
        hoverEnabled: true
        cursorShape: root.isDragging ? Qt.ClosedHandCursor : (containsMouse ? Qt.OpenHandCursor : Qt.ArrowCursor)

        property real dragStartX: 0
        property real dragStartY: 0
        property real dragOriginX: 0
        property real dragOriginY: 0

        onPressed: (mouse) => {
            if (mouse.button !== Qt.LeftButton) return;
            dragStartX = root.x;
            dragStartY = root.y;
            var mapped = cardMouseArea.mapToItem(root.parent, mouse.x, mouse.y);
            dragOriginX = mapped.x;
            dragOriginY = mapped.y;
            root.isDragging = true;
            root.selectRequested();
        }

        onPositionChanged: (mouse) => {
            if (!root.isDragging) return;

            var current = cardMouseArea.mapToItem(root.parent, mouse.x, mouse.y);
            var dx = current.x - dragOriginX;
            var dy = current.y - dragOriginY;

            var proposedX = dragStartX + dx;
            var proposedY = dragStartY + dy;

            // Clamping inside monitor boundaries
            proposedX = Math.max(0, Math.min(root.monitorWidth - root.width, proposedX));
            proposedY = Math.max(0, Math.min(root.monitorHeight - root.height, proposedY));

            var showV = false;
            var vX = 0;
            var showH = false;
            var hY = 0;

            // Magnetic Snapping X: 50% Centerline & Edges
            var centerX = proposedX + root.width / 2;
            var monCenterX = root.monitorWidth / 2;
            if (Math.abs(centerX - monCenterX) <= root.snapThreshold) {
                proposedX = monCenterX - root.width / 2;
                showV = true;
                vX = monCenterX;
            } else if (Math.abs(proposedX) <= root.snapThreshold) {
                proposedX = 0;
                showV = true;
                vX = 0;
            } else if (Math.abs(proposedX + root.width - root.monitorWidth) <= root.snapThreshold) {
                proposedX = root.monitorWidth - root.width;
                showV = true;
                vX = root.monitorWidth;
            }

            // Magnetic Snapping Y: 50% Centerline & Edges
            var centerY = proposedY + root.height / 2;
            var monCenterY = root.monitorHeight / 2;
            if (Math.abs(centerY - monCenterY) <= root.snapThreshold) {
                proposedY = monCenterY - root.height / 2;
                showH = true;
                hY = monCenterY;
            } else if (Math.abs(proposedY) <= root.snapThreshold) {
                proposedY = 0;
                showH = true;
                hY = 0;
            } else if (Math.abs(proposedY + root.height - root.monitorHeight) <= root.snapThreshold) {
                proposedY = root.monitorHeight - root.height;
                showH = true;
                hY = root.monitorHeight;
            }

            root.x = proposedX;
            root.y = proposedY;
            root.snapGuidesUpdated(showV, vX, showH, hY);
        }

        onReleased: (mouse) => {
            if (root.isDragging) {
                root.isDragging = false;
                root.snapGuidesUpdated(false, 0, false, 0);
                commitCurrentRect();
            }
        }
    }

    // Helper to Commit Rect to PlacementModel
    function commitCurrentRect() {
        var r = Qt.rect(root.x, root.y, root.width, root.height);
        root.rectCommitted(root.placementId, r);
        if (typeof placementModel !== "undefined" && placementModel) {
            placementModel.updatePlacementRect(root.placementId, r);
        }
    }

    // Resize Handles Visibility
    readonly property bool handlesVisible: root.isSelected || cardMouseArea.containsMouse || root.isDragging || root.isResizing

    // 8 Resize Handles Component Functionality
    function startResize(mouse, handleItem) {
        root.isResizing = true;
        root.selectRequested();
        var globalPos = handleItem.mapToItem(root.parent, mouse.x, mouse.y);
        resizeOriginX = globalPos.x;
        resizeOriginY = globalPos.y;
        resizeStartX = root.x;
        resizeStartY = root.y;
        resizeStartW = root.width;
        resizeStartH = root.height;
    }

    property real resizeOriginX: 0
    property real resizeOriginY: 0
    property real resizeStartX: 0
    property real resizeStartY: 0
    property real resizeStartW: 0
    property real resizeStartH: 0

    function handleResizeDelta(mouse, handleItem, movesLeft, movesRight, movesTop, movesBottom) {
        if (!root.isResizing) return;

        var globalPos = handleItem.mapToItem(root.parent, mouse.x, mouse.y);
        var dx = globalPos.x - resizeOriginX;
        var dy = globalPos.y - resizeOriginY;

        var newX = resizeStartX;
        var newY = resizeStartY;
        var newW = resizeStartW;
        var newH = resizeStartH;

        var showV = false;
        var vX = 0;
        var showH = false;
        var hY = 0;

        // Horizontal resizing
        if (movesRight) {
            newW = Math.max(root.minWidth, resizeStartW + dx);
            if (newX + newW > root.monitorWidth) {
                newW = root.monitorWidth - newX;
            }
            // Snap right edge to 50% or monitor edge
            if (Math.abs(newX + newW - root.monitorWidth) <= root.snapThreshold) {
                newW = root.monitorWidth - newX;
                showV = true;
                vX = root.monitorWidth;
            } else if (Math.abs(newX + newW - root.monitorWidth / 2) <= root.snapThreshold) {
                newW = root.monitorWidth / 2 - newX;
                showV = true;
                vX = root.monitorWidth / 2;
            }
        } else if (movesLeft) {
            var proposedX = resizeStartX + dx;
            var proposedW = resizeStartW - dx;
            if (proposedW < root.minWidth) {
                proposedX = resizeStartX + resizeStartW - root.minWidth;
                proposedW = root.minWidth;
            }
            if (proposedX < 0) {
                proposedW += proposedX;
                proposedX = 0;
            }
            // Snap left edge to 0 or 50%
            if (Math.abs(proposedX) <= root.snapThreshold) {
                proposedW += proposedX;
                proposedX = 0;
                showV = true;
                vX = 0;
            } else if (Math.abs(proposedX - root.monitorWidth / 2) <= root.snapThreshold) {
                proposedW += (proposedX - root.monitorWidth / 2);
                proposedX = root.monitorWidth / 2;
                showV = true;
                vX = root.monitorWidth / 2;
            }
            newX = proposedX;
            newW = proposedW;
        }

        // Vertical resizing
        if (movesBottom) {
            newH = Math.max(root.minHeight, resizeStartH + dy);
            if (newY + newH > root.monitorHeight) {
                newH = root.monitorHeight - newY;
            }
            // Snap bottom edge to 50% or monitor edge
            if (Math.abs(newY + newH - root.monitorHeight) <= root.snapThreshold) {
                newH = root.monitorHeight - newY;
                showH = true;
                hY = root.monitorHeight;
            } else if (Math.abs(newY + newH - root.monitorHeight / 2) <= root.snapThreshold) {
                newH = root.monitorHeight / 2 - newY;
                showH = true;
                hY = root.monitorHeight / 2;
            }
        } else if (movesTop) {
            var proposedY = resizeStartY + dy;
            var proposedH = resizeStartH - dy;
            if (proposedH < root.minHeight) {
                proposedY = resizeStartY + resizeStartH - root.minHeight;
                proposedH = root.minHeight;
            }
            if (proposedY < 0) {
                proposedH += proposedY;
                proposedY = 0;
            }
            // Snap top edge to 0 or 50%
            if (Math.abs(proposedY) <= root.snapThreshold) {
                proposedH += proposedY;
                proposedY = 0;
                showH = true;
                hY = 0;
            } else if (Math.abs(proposedY - root.monitorHeight / 2) <= root.snapThreshold) {
                proposedH += (proposedY - root.monitorHeight / 2);
                proposedY = root.monitorHeight / 2;
                showH = true;
                hY = root.monitorHeight / 2;
            }
            newY = proposedY;
            newH = proposedH;
        }

        newW = Math.max(root.minWidth, newW);
        newH = Math.max(root.minHeight, newH);

        root.x = newX;
        root.y = newY;
        root.width = newW;
        root.height = newH;
        root.snapGuidesUpdated(showV, vX, showH, hY);
    }

    function endResize() {
        if (root.isResizing) {
            root.isResizing = false;
            root.snapGuidesUpdated(false, 0, false, 0);
            commitCurrentRect();
        }
    }

    // 1. TopLeft Handle
    Rectangle {
        id: handleTopLeft
        x: -root.handleSize / 2
        y: -root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleTopLeftMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleTopLeftMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeFDiagCursor
            onPressed: (mouse) => root.startResize(mouse, handleTopLeft)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleTopLeft, true, false, true, false)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 2. TopMid Handle
    Rectangle {
        id: handleTopMid
        x: (root.width - root.handleSize) / 2
        y: -root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleTopMidMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleTopMidMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeVerCursor
            onPressed: (mouse) => root.startResize(mouse, handleTopMid)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleTopMid, false, false, true, false)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 3. TopRight Handle
    Rectangle {
        id: handleTopRight
        x: root.width - root.handleSize / 2
        y: -root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleTopRightMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleTopRightMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeBDiagCursor
            onPressed: (mouse) => root.startResize(mouse, handleTopRight)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleTopRight, false, true, true, false)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 4. MidLeft Handle
    Rectangle {
        id: handleMidLeft
        x: -root.handleSize / 2
        y: (root.height - root.handleSize) / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleMidLeftMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleMidLeftMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            onPressed: (mouse) => root.startResize(mouse, handleMidLeft)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleMidLeft, true, false, false, false)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 5. MidRight Handle
    Rectangle {
        id: handleMidRight
        x: root.width - root.handleSize / 2
        y: (root.height - root.handleSize) / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleMidRightMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleMidRightMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            onPressed: (mouse) => root.startResize(mouse, handleMidRight)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleMidRight, false, true, false, false)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 6. BotLeft Handle
    Rectangle {
        id: handleBotLeft
        x: -root.handleSize / 2
        y: root.height - root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleBotLeftMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleBotLeftMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeBDiagCursor
            onPressed: (mouse) => root.startResize(mouse, handleBotLeft)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleBotLeft, true, false, false, true)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 7. BotMid Handle
    Rectangle {
        id: handleBotMid
        x: (root.width - root.handleSize) / 2
        y: root.height - root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleBotMidMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleBotMidMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeVerCursor
            onPressed: (mouse) => root.startResize(mouse, handleBotMid)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleBotMid, false, false, false, true)
            onReleased: (mouse) => root.endResize()
        }
    }

    // 8. BotRight Handle
    Rectangle {
        id: handleBotRight
        x: root.width - root.handleSize / 2
        y: root.height - root.handleSize / 2
        width: root.handleSize
        height: root.handleSize
        radius: width / 2
        z: 15
        visible: root.handlesVisible
        color: handleBotRightMouse.containsMouse ? Theme.accentCyan : (Theme.isDark ? "#0D131F" : "#FFFFFF")
        border.color: Theme.accentCyan
        border.width: root.handleBorderWidth

        MouseArea {
            id: handleBotRightMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeFDiagCursor
            onPressed: (mouse) => root.startResize(mouse, handleBotRight)
            onPositionChanged: (mouse) => root.handleResizeDelta(mouse, handleBotRight, false, true, false, true)
            onReleased: (mouse) => root.endResize()
        }
    }
}
