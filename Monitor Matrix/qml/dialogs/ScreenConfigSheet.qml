// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

Item {
    id: root

    property var displayBridge: null
    property bool isOpen: false

    property int selectedDisplayIndex: displayBridge ? displayBridge.selectedIndex : -1
    property var displaysList: displayBridge ? displayBridge.displays : []
    property var selectedDisplay: (selectedDisplayIndex >= 0 && selectedDisplayIndex < displaysList.length)
                                  ? displaysList[selectedDisplayIndex]
                                  : null
    property var availableResolutions: (displayBridge && selectedDisplayIndex >= 0)
                                       ? displayBridge.getResolutions(selectedDisplayIndex)
                                       : []

    property real drawerWidth: Math.min(640, Math.max(480, root.width * 0.55))

    signal closed()
    signal notificationRequested(string title, string message, bool isError)

    anchors.fill: parent
    visible: opacity > 0.0 || drawerPanel.x < root.width
    opacity: isOpen ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.durationNormal
            easing.type: Theme.easingType
        }
    }

    function open() {
        if (displayBridge) {
            displayBridge.refreshDisplays();
        }
        isOpen = true;
    }

    function close() {
        isOpen = false;
        root.closed();
    }

    Shortcut {
        sequence: "Escape"
        enabled: root.isOpen
        onActivated: root.close()
    }

    Connections {
        target: displayBridge
        function onApplySuccess() {
            root.notificationRequested("Display Settings Applied", "Display topology and resolution have been updated successfully.", false);
        }
        function onApplyFailed(errorMessage: string) {
            root.notificationRequested("Failed to Apply Settings", errorMessage ? errorMessage : "Windows rejected the display configuration.", true);
        }
    }

    // Dimmed Frosted Backdrop
    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: Qt.rgba(8 / 255, 11 / 255, 17 / 255, 0.72)
        opacity: root.isOpen ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.durationNormal
                easing.type: Theme.easingType
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    // Slide-out Drawer Panel
    Rectangle {
        id: drawerPanel
        width: root.drawerWidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        x: root.isOpen ? (parent.width - width) : parent.width

        Behavior on x {
            NumberAnimation {
                duration: Theme.durationNormal
                easing.type: Theme.easingType
            }
        }

        color: Theme.bgSurface
        border.color: Theme.borderSubtle
        border.width: 1

        // Left Edge Highlight Glow Stripe
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 2
            color: Theme.accentCyan
            opacity: 0.6
        }

        // Prevent clicks inside drawer from dismissing backdrop
        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        // Drawer Header Bar
        Item {
            id: drawerHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 64

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.borderSubtle
            }

            Column {
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacing2Xs

                Row {
                    spacing: Theme.spacingSm

                    Text {
                        text: "Display Topology & Settings"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeLarge
                        font.weight: Theme.fontWeightDemiBold
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        height: 18
                        width: countLabel.implicitWidth + 10
                        radius: Theme.radiusPill
                        color: Theme.accentCyanMuted
                        border.color: Theme.accentCyan
                        border.width: 1

                        Text {
                            id: countLabel
                            anchors.centerIn: parent
                            text: (root.displaysList ? root.displaysList.length : 0) + " Active"
                            color: Theme.accentCyan
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeTiny
                            font.weight: Theme.fontWeightBold
                        }
                    }
                }

                Text {
                    text: "Rearrange monitors, configure resolutions, and assign primary display"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacingSm

                // Refresh Button
                Rectangle {
                    width: 32
                    height: 32
                    radius: Theme.radiusSm
                    color: refreshMouse.pressed ? Theme.bgSurfaceActive : (refreshMouse.containsMouse ? Theme.bgSurfaceHover : "transparent")
                    border.color: refreshMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "↺"
                        color: refreshMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeMedium
                    }

                    ToolTip.visible: refreshMouse.containsMouse
                    ToolTip.text: "Re-query Windows display topology"
                    ToolTip.delay: 400

                    MouseArea {
                        id: refreshMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (root.displayBridge) {
                                root.displayBridge.refreshDisplays();
                            }
                        }
                    }
                }

                // Close Button
                Rectangle {
                    width: 32
                    height: 32
                    radius: Theme.radiusSm
                    color: closeMouse.pressed ? Theme.bgSurfaceActive : (closeMouse.containsMouse ? Theme.bgSurfaceHover : "transparent")
                    border.color: closeMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: closeMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeHeader
                    }

                    MouseArea {
                        id: closeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.close()
                    }
                }
            }
        }

        // Scrollable Body Area
        Flickable {
            id: scrollArea
            anchors.top: drawerHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: drawerFooter.top
            clip: true
            contentWidth: width
            contentHeight: bodyColumn.implicitHeight + (Theme.spacingXl * 2)

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                active: true
            }

            Column {
                id: bodyColumn
                anchors.top: parent.top
                anchors.topMargin: Theme.spacingLg
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLg
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingLg
                spacing: Theme.spacingXl

                // SECTION 1: TOPOLOGY CANVAS
                Column {
                    width: parent.width
                    spacing: Theme.spacingSm

                    Row {
                        width: parent.width
                        spacing: Theme.spacingSm

                        Text {
                            text: "MONITOR TOPOLOGY"
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeTiny
                            font.weight: Theme.fontWeightBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "• Drag displays to reorder virtual desktop coordinates"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSmall
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Rectangle {
                        id: canvasContainer
                        width: parent.width
                        height: 270
                        radius: Theme.radiusMd
                        color: Theme.bgCanvas
                        border.color: Theme.borderSubtle
                        border.width: 1
                        clip: true

                        // Blueprint Dotted Grid Background
                        Canvas {
                            id: gridBackground
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                ctx.fillStyle = Theme.isDark ? "#1C273C" : "#CBD5E1";
                                var spacing = 20;
                                var dotSize = 1.5;
                                for (var x = spacing / 2; x < width; x += spacing) {
                                    for (var y = spacing / 2; y < height; y += spacing) {
                                        ctx.fillRect(x, y, dotSize, dotSize);
                                    }
                                }
                            }
                            onWidthChanged: requestPaint()
                            onHeightChanged: requestPaint()
                        }

                        // Topology Preview Area
                        Item {
                            id: topoCanvasArea
                            anchors.fill: parent
                            anchors.margins: 16

                            property real minX: 0
                            property real minY: 0
                            property real totalW: 1920
                            property real totalH: 1080
                            property real zoomScale: 1.0
                            property real offsetX: 0
                            property real offsetY: 0
                            property bool isDragging: false

                            function computeBounds() {
                                if (isDragging) {
                                    return;
                                }
                                var list = root.displaysList;
                                if (!list || list.length === 0) {
                                    minX = 0;
                                    minY = 0;
                                    totalW = 1920;
                                    totalH = 1080;
                                    zoomScale = 0.1;
                                    offsetX = 0;
                                    offsetY = 0;
                                    return;
                                }
                                var mX = list[0].x;
                                var mY = list[0].y;
                                var maxX = list[0].x + list[0].width;
                                var maxY = list[0].y + list[0].height;

                                for (var i = 1; i < list.length; ++i) {
                                    var d = list[i];
                                    if (d.x < mX) mX = d.x;
                                    if (d.y < mY) mY = d.y;
                                    if (d.x + d.width > maxX) maxX = d.x + d.width;
                                    if (d.y + d.height > maxY) maxY = d.y + d.height;
                                }

                                minX = mX;
                                minY = mY;
                                totalW = Math.max(200, maxX - mX);
                                totalH = Math.max(200, maxY - mY);

                                var s = Math.min((width - 48) / totalW, (height - 48) / totalH);
                                zoomScale = Math.max(0.01, s);
                                offsetX = (width - totalW * zoomScale) / 2 - minX * zoomScale;
                                offsetY = (height - totalH * zoomScale) / 2 - minY * zoomScale;
                            }

                            onWidthChanged: computeBounds()
                            onHeightChanged: computeBounds()

                            Connections {
                                target: root
                                function onDisplaysListChanged() {
                                    topoCanvasArea.computeBounds();
                                }
                            }

                            Repeater {
                                model: root.displaysList
                                delegate: Item {
                                    id: monitorDelegate
                                    property var disp: modelData
                                    property int dispIndex: index
                                    property bool isSelected: root.selectedDisplayIndex === index

                                    x: disp.x * topoCanvasArea.zoomScale + topoCanvasArea.offsetX
                                    y: disp.y * topoCanvasArea.zoomScale + topoCanvasArea.offsetY
                                    width: Math.max(70, disp.width * topoCanvasArea.zoomScale)
                                    height: Math.max(45, disp.height * topoCanvasArea.zoomScale)
                                    z: isSelected ? 10 : 1

                                    Rectangle {
                                        id: monitorFrame
                                        anchors.fill: parent
                                        radius: Theme.radiusSm
                                        color: isSelected
                                               ? (Theme.isDark ? "#16233B" : "#E0F2FE")
                                               : (monitorMouse.containsMouse
                                                  ? (Theme.isDark ? "#121C2F" : "#F1F5F9")
                                                  : (Theme.isDark ? "#0A0E18" : "#F8FAFC"))
                                        border.color: isSelected
                                                      ? Theme.accentCyan
                                                      : (monitorMouse.containsMouse ? Theme.accentCyanHover : Theme.borderStrong)
                                        border.width: isSelected ? 2 : 1

                                        // Glow outline when selected
                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: -3
                                            radius: Theme.radiusSm + 2
                                            color: "transparent"
                                            border.color: Theme.accentCyan
                                            border.width: 1
                                            opacity: isSelected ? 0.4 : 0.0
                                            visible: opacity > 0.0
                                        }

                                        // Center display index watermark
                                        Text {
                                            anchors.centerIn: parent
                                            text: (dispIndex + 1).toString()
                                            color: isSelected ? Theme.accentCyan : Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Math.max(14, Math.min(32, parent.height * 0.45))
                                            font.weight: Theme.fontWeightBold
                                            opacity: isSelected ? 0.35 : 0.12
                                        }

                                        // Top Information Badges Row
                                        Row {
                                            anchors.top: parent.top
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.margins: 4
                                            spacing: 4

                                            // Display number pill
                                            Rectangle {
                                                height: 14
                                                width: numTagText.implicitWidth + 6
                                                radius: 2
                                                color: isSelected ? Theme.accentCyan : (Theme.isDark ? "#1E293B" : "#CBD5E1")

                                                Text {
                                                    id: numTagText
                                                    anchors.centerIn: parent
                                                    text: (dispIndex + 1).toString()
                                                    color: isSelected ? "#080B11" : Theme.textPrimary
                                                    font.family: Theme.fontFamily
                                                    font.pixelSize: 9
                                                    font.weight: Theme.fontWeightBold
                                                }
                                            }

                                            // Primary Display Badge
                                            Rectangle {
                                                visible: disp.isPrimary === true
                                                height: 14
                                                width: primaryTagText.implicitWidth + 6
                                                radius: 2
                                                color: Theme.accentAmberMuted
                                                border.color: Theme.accentAmber
                                                border.width: 1

                                                Text {
                                                    id: primaryTagText
                                                    anchors.centerIn: parent
                                                    text: "★ PRIMARY"
                                                    color: Theme.accentAmber
                                                    font.family: Theme.fontFamily
                                                    font.pixelSize: 8
                                                    font.weight: Theme.fontWeightBold
                                                }
                                            }
                                        }

                                        // Bottom Information: Resolution
                                        Text {
                                            anchors.bottom: parent.bottom
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottomMargin: 3
                                            anchors.leftMargin: 4
                                            anchors.rightMargin: 4
                                            text: disp.width + " × " + disp.height
                                            color: isSelected ? Theme.accentCyan : Theme.textSecondary
                                            font.family: Theme.fontMono
                                            font.pixelSize: Math.max(8, Math.min(10, parent.width * 0.11))
                                            elide: Text.ElideRight
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                    }

                                    MouseArea {
                                        id: monitorMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                                        property real startMouseX: 0
                                        property real startMouseY: 0
                                        property int origDesktopX: 0
                                        property int origDesktopY: 0

                                        onPressed: (mouse) => {
                                            startMouseX = mouse.x;
                                            startMouseY = mouse.y;
                                            origDesktopX = disp.x;
                                            origDesktopY = disp.y;
                                            if (root.displayBridge) {
                                                root.displayBridge.selectedIndex = dispIndex;
                                            }
                                            topoCanvasArea.isDragging = true;
                                        }

                                        onPositionChanged: (mouse) => {
                                            if (pressed && topoCanvasArea.isDragging) {
                                                var dx = mouse.x - startMouseX;
                                                var dy = mouse.y - startMouseY;
                                                var desktopDx = Math.round(dx / topoCanvasArea.zoomScale);
                                                var desktopDy = Math.round(dy / topoCanvasArea.zoomScale);
                                                var targetX = origDesktopX + desktopDx;
                                                var targetY = origDesktopY + desktopDy;

                                                // Magnetic snapping to (0, 0) and neighbor boundaries
                                                var snapThreshold = 32;
                                                if (Math.abs(targetX) < snapThreshold) targetX = 0;
                                                if (Math.abs(targetY) < snapThreshold) targetY = 0;

                                                for (var j = 0; j < root.displaysList.length; ++j) {
                                                    if (j === dispIndex) continue;
                                                    var other = root.displaysList[j];

                                                    if (Math.abs((targetX + disp.width) - other.x) < snapThreshold) {
                                                        targetX = other.x - disp.width;
                                                    }
                                                    if (Math.abs(targetX - (other.x + other.width)) < snapThreshold) {
                                                        targetX = other.x + other.width;
                                                    }
                                                    if (Math.abs(targetY - other.y) < snapThreshold) {
                                                        targetY = other.y;
                                                    }
                                                    if (Math.abs((targetY + disp.height) - other.y) < snapThreshold) {
                                                        targetY = other.y - disp.height;
                                                    }
                                                    if (Math.abs(targetY - (other.y + other.height)) < snapThreshold) {
                                                        targetY = other.y + other.height;
                                                    }
                                                }

                                                if (root.displayBridge) {
                                                    root.displayBridge.setDisplayPosition(dispIndex, targetX, targetY);
                                                }
                                            }
                                        }

                                        onReleased: {
                                            topoCanvasArea.isDragging = false;
                                            topoCanvasArea.computeBounds();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // SECTION 2: SELECTED DISPLAY SETTINGS PANEL
                Column {
                    width: parent.width
                    spacing: Theme.spacingMd
                    visible: root.selectedDisplay !== null

                    Row {
                        width: parent.width
                        spacing: Theme.spacingSm

                        Text {
                            text: "DISPLAY SETTINGS"
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeTiny
                            font.weight: Theme.fontWeightBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: root.selectedDisplay
                                  ? ("• Display " + (root.selectedDisplayIndex + 1) + ": " + (root.selectedDisplay.displayName ? root.selectedDisplay.displayName : root.selectedDisplay.deviceName))
                                  : ""
                            color: Theme.accentCyan
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSmall
                            font.weight: Theme.fontWeightMedium
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        width: parent.width
                        implicitHeight: settingsCardColumn.implicitHeight + (Theme.spacingLg * 2)
                        radius: Theme.radiusMd
                        color: Theme.bgSurfaceElevated
                        border.color: Theme.borderSubtle
                        border.width: 1

                        Column {
                            id: settingsCardColumn
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: Theme.spacingLg
                            spacing: Theme.spacingLg

                            // Display Header Badge
                            Row {
                                width: parent.width
                                spacing: Theme.spacingMd

                                Rectangle {
                                    width: 36
                                    height: 36
                                    radius: Theme.radiusSm
                                    color: Theme.accentCyanMuted
                                    border.color: Theme.accentCyan
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: (root.selectedDisplayIndex + 1).toString()
                                        color: Theme.accentCyan
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeLarge
                                        font.weight: Theme.fontWeightBold
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: Theme.spacing2Xs

                                    Text {
                                        text: root.selectedDisplay
                                              ? (root.selectedDisplay.displayName ? root.selectedDisplay.displayName : "Generic Monitor")
                                              : ""
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeMedium
                                        font.weight: Theme.fontWeightDemiBold
                                    }

                                    Row {
                                        spacing: Theme.spacingSm

                                        Text {
                                            text: root.selectedDisplay ? root.selectedDisplay.deviceName : ""
                                            color: Theme.textMuted
                                            font.family: Theme.fontMono
                                            font.pixelSize: Theme.fontSizeSmall
                                        }

                                        Text {
                                            visible: root.selectedDisplay ? (root.selectedDisplay.isPrimary === true) : false
                                            text: "• Primary Display"
                                            color: Theme.accentAmber
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeSmall
                                            font.weight: Theme.fontWeightMedium
                                        }
                                    }
                                }
                            }

                            // Separator
                            Rectangle {
                                width: parent.width
                                height: 1
                                color: Theme.borderSubtle
                            }

                            // Resolution ComboBox Row
                            Column {
                                width: parent.width
                                spacing: Theme.spacingSm

                                Row {
                                    spacing: Theme.spacingXs

                                    Text {
                                        text: "Display Resolution & Refresh Rate"
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeRegular
                                        font.weight: Theme.fontWeightMedium
                                    }

                                    Text {
                                        text: "(Recommended modes)"
                                        color: Theme.textMuted
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeSmall
                                    }
                                }

                                ComboBox {
                                    id: resolutionCombo
                                    width: parent.width
                                    model: root.availableResolutions
                                    textRole: "label"

                                    function syncResolutionIndex() {
                                        if (!root.selectedDisplay || !root.availableResolutions || root.availableResolutions.length === 0) {
                                            return;
                                        }
                                        for (var i = 0; i < root.availableResolutions.length; ++i) {
                                            var opt = root.availableResolutions[i];
                                            if (opt.width === root.selectedDisplay.width && opt.height === root.selectedDisplay.height) {
                                                resolutionCombo.currentIndex = i;
                                                return;
                                            }
                                        }
                                    }

                                    Component.onCompleted: syncResolutionIndex()

                                    Connections {
                                        target: root
                                        function onSelectedDisplayChanged() {
                                            resolutionCombo.syncResolutionIndex();
                                        }
                                        function onAvailableResolutionsChanged() {
                                            resolutionCombo.syncResolutionIndex();
                                        }
                                    }

                                    onActivated: (index) => {
                                        if (root.availableResolutions && index >= 0 && index < root.availableResolutions.length) {
                                            var chosen = root.availableResolutions[index];
                                            if (root.displayBridge && root.selectedDisplayIndex >= 0) {
                                                root.displayBridge.setDisplayResolution(root.selectedDisplayIndex, chosen.width, chosen.height);
                                            }
                                        }
                                    }

                                    contentItem: Text {
                                        leftPadding: Theme.spacingMd
                                        rightPadding: resolutionCombo.indicator.width + Theme.spacingMd
                                        text: resolutionCombo.displayText
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeRegular
                                        color: Theme.textPrimary
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }

                                    background: Rectangle {
                                        implicitHeight: 38
                                        radius: Theme.radiusSm
                                        color: resolutionCombo.pressed ? Theme.bgSurfaceActive : Theme.bgSurface
                                        border.color: resolutionCombo.activeFocus ? Theme.borderFocus : Theme.borderStrong
                                        border.width: 1
                                    }

                                    popup: Popup {
                                        y: resolutionCombo.height + 4
                                        width: resolutionCombo.width
                                        implicitHeight: Math.min(260, contentItem.implicitHeight + 8)
                                        padding: 4

                                        contentItem: ListView {
                                            clip: true
                                            implicitHeight: contentHeight
                                            model: resolutionCombo.popup.visible ? resolutionCombo.delegateModel : null
                                            currentIndex: resolutionCombo.highlightedIndex
                                            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                        }

                                        background: Rectangle {
                                            color: Theme.bgSurfaceElevated
                                            border.color: Theme.borderStrong
                                            border.width: 1
                                            radius: Theme.radiusMd
                                        }
                                    }

                                    delegate: ItemDelegate {
                                        width: resolutionCombo.width - 8
                                        height: 34
                                        highlighted: resolutionCombo.highlightedIndex === index

                                        contentItem: Text {
                                            text: modelData.label
                                            color: highlighted ? Theme.accentCyan : Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeRegular
                                            verticalAlignment: Text.AlignVCenter
                                            elide: Text.ElideRight
                                        }

                                        background: Rectangle {
                                            color: highlighted ? Theme.bgSurfaceHover : "transparent"
                                            radius: Theme.radiusSm
                                        }
                                    }
                                }
                            }

                            // Primary Display CheckBox Row
                            Rectangle {
                                width: parent.width
                                implicitHeight: primaryRow.implicitHeight + (Theme.spacingMd * 2)
                                radius: Theme.radiusSm
                                color: Theme.bgSurface
                                border.color: Theme.borderSubtle
                                border.width: 1

                                Row {
                                    id: primaryRow
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.leftMargin: Theme.spacingMd
                                    anchors.rightMargin: Theme.spacingMd
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: Theme.spacingMd

                                    CheckBox {
                                        id: primaryCheckBox
                                        anchors.verticalCenter: parent.verticalCenter
                                        checked: root.selectedDisplay ? (root.selectedDisplay.isPrimary === true) : false
                                        enabled: root.selectedDisplay ? (root.selectedDisplay.isPrimary !== true) : false

                                        onToggled: {
                                            if (root.displayBridge && root.selectedDisplayIndex >= 0) {
                                                root.displayBridge.setPrimaryDisplay(root.selectedDisplayIndex);
                                            }
                                        }
                                    }

                                    Column {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width - primaryCheckBox.width - Theme.spacingMd
                                        spacing: Theme.spacing2Xs

                                        Text {
                                            text: "Make this my primary display"
                                            color: primaryCheckBox.enabled ? Theme.textPrimary : Theme.textMuted
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeRegular
                                            font.weight: Theme.fontWeightMedium
                                        }

                                        Text {
                                            text: "Desktop icons, system notifications, and application launches default to this display."
                                            color: Theme.textSecondary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeSmall
                                            wrapMode: Text.Wrap
                                            width: parent.width
                                        }
                                    }
                                }
                            }

                            // Virtual Coordinates Readout
                            Rectangle {
                                width: parent.width
                                implicitHeight: coordRow.implicitHeight + (Theme.spacingMd * 2)
                                radius: Theme.radiusSm
                                color: Theme.bgSurface
                                border.color: Theme.borderSubtle
                                border.width: 1

                                Row {
                                    id: coordRow
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.leftMargin: Theme.spacingMd
                                    anchors.rightMargin: Theme.spacingMd
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: Theme.spacingLg

                                    Column {
                                        spacing: Theme.spacing2Xs
                                        Text {
                                            text: "Virtual Desktop Coordinates"
                                            color: Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeSmall
                                            font.weight: Theme.fontWeightMedium
                                        }
                                        Text {
                                            text: "Top-left position relative to primary monitor at (0, 0)"
                                            color: Theme.textMuted
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeTiny
                                        }
                                    }

                                    Item { Layout.fillWidth: true; width: 1 }

                                    Row {
                                        spacing: Theme.spacingSm
                                        anchors.verticalCenter: parent.verticalCenter

                                        Rectangle {
                                            height: 26
                                            width: coordXText.implicitWidth + 14
                                            radius: Theme.radiusSm
                                            color: Theme.bgSurfaceElevated
                                            border.color: Theme.borderStrong
                                            border.width: 1

                                            Text {
                                                id: coordXText
                                                anchors.centerIn: parent
                                                text: "X: " + (root.selectedDisplay ? root.selectedDisplay.x : 0) + "px"
                                                color: Theme.accentCyan
                                                font.family: Theme.fontMono
                                                font.pixelSize: Theme.fontSizeSmall
                                            }
                                        }

                                        Rectangle {
                                            height: 26
                                            width: coordYText.implicitWidth + 14
                                            radius: Theme.radiusSm
                                            color: Theme.bgSurfaceElevated
                                            border.color: Theme.borderStrong
                                            border.width: 1

                                            Text {
                                                id: coordYText
                                                anchors.centerIn: parent
                                                text: "Y: " + (root.selectedDisplay ? root.selectedDisplay.y : 0) + "px"
                                                color: Theme.accentCyan
                                                font.family: Theme.fontMono
                                                font.pixelSize: Theme.fontSizeSmall
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Unapplied Changes Warning Callout
                Rectangle {
                    width: parent.width
                    implicitHeight: unappliedRow.implicitHeight + (Theme.spacingMd * 2)
                    radius: Theme.radiusMd
                    color: Theme.accentAmberMuted
                    border.color: Theme.accentAmber
                    border.width: 1
                    visible: root.displayBridge ? (root.displayBridge.hasChanges === true) : false

                    Row {
                        id: unappliedRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: Theme.spacingMd
                        spacing: Theme.spacingMd

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "⚠"
                            color: Theme.accentAmber
                            font.pixelSize: Theme.fontSizeLarge
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 32
                            spacing: Theme.spacing2Xs

                            Text {
                                text: "Unapplied Display Settings"
                                color: Theme.accentAmber
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSizeRegular
                                font.weight: Theme.fontWeightBold
                            }

                            Text {
                                text: "Topology or resolution modifications are currently pending. Click 'Apply Display Settings' below to persist changes to Windows."
                                color: Theme.isDark ? "#FDE68A" : "#92400E"
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSizeSmall
                                wrapMode: Text.Wrap
                                width: parent.width
                            }
                        }
                    }
                }
            }
        }

        // Drawer Footer Action Bar
        Item {
            id: drawerFooter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 68

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: Theme.borderSubtle
            }

            Rectangle {
                anchors.fill: parent
                color: Theme.bgSurface
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacingSm

                // Status Indicator Pill
                Rectangle {
                    height: 26
                    width: statusText.implicitWidth + 20
                    radius: Theme.radiusPill
                    color: (root.displayBridge && root.displayBridge.hasChanges)
                           ? Theme.accentAmberMuted
                           : Theme.accentEmeraldMuted
                    border.color: (root.displayBridge && root.displayBridge.hasChanges)
                                  ? Theme.accentAmber
                                  : Theme.accentEmerald
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: Theme.spacingXs

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 6
                            height: 6
                            radius: 3
                            color: (root.displayBridge && root.displayBridge.hasChanges)
                                   ? Theme.accentAmber
                                   : Theme.accentEmerald
                        }

                        Text {
                            id: statusText
                            anchors.verticalCenter: parent.verticalCenter
                            text: (root.displayBridge && root.displayBridge.hasChanges)
                                  ? "Pending Changes"
                                  : "Windows In Sync"
                            color: (root.displayBridge && root.displayBridge.hasChanges)
                                   ? Theme.accentAmber
                                   : Theme.accentEmerald
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeTiny
                            font.weight: Theme.fontWeightBold
                        }
                    }
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacingSm

                // Revert Changes Button
                Rectangle {
                    implicitWidth: revertText.implicitWidth + (Theme.spacingMd * 2)
                    implicitHeight: 36
                    radius: Theme.radiusSm
                    property bool btnEnabled: root.displayBridge ? (root.displayBridge.hasChanges === true) : false
                    color: btnEnabled
                           ? (revertMouse.pressed ? Theme.bgSurfaceActive : (revertMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"))
                           : "transparent"
                    border.color: btnEnabled ? Theme.borderStrong : Theme.borderSubtle
                    border.width: 1
                    opacity: btnEnabled ? 1.0 : 0.4

                    Text {
                        id: revertText
                        anchors.centerIn: parent
                        text: "Revert Changes"
                        color: parent.btnEnabled ? Theme.textPrimary : Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Theme.fontWeightMedium
                    }

                    MouseArea {
                        id: revertMouse
                        anchors.fill: parent
                        hoverEnabled: parent.btnEnabled
                        cursorShape: parent.btnEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (parent.btnEnabled && root.displayBridge) {
                                root.displayBridge.revertChanges();
                            }
                        }
                    }
                }

                // Apply Display Settings Button
                Rectangle {
                    implicitWidth: applyText.implicitWidth + (Theme.spacingLg * 2)
                    implicitHeight: 36
                    radius: Theme.radiusSm
                    property bool btnEnabled: root.displayBridge ? (root.displayBridge.hasChanges === true) : false
                    color: btnEnabled
                           ? (applyMouse.pressed ? Theme.accentCyanHover : Theme.accentCyan)
                           : (Theme.isDark ? "#1E293B" : "#E2E8F0")
                    opacity: btnEnabled ? 1.0 : 0.45

                    Behavior on color {
                        ColorAnimation {
                            duration: Theme.durationFast
                            easing.type: Theme.easingType
                        }
                    }

                    Text {
                        id: applyText
                        anchors.centerIn: parent
                        text: "Apply Display Settings"
                        color: parent.btnEnabled ? "#080B11" : Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Theme.fontWeightBold
                    }

                    MouseArea {
                        id: applyMouse
                        anchors.fill: parent
                        hoverEnabled: parent.btnEnabled
                        cursorShape: parent.btnEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (parent.btnEnabled && root.displayBridge) {
                                root.displayBridge.applyChanges();
                            }
                        }
                    }
                }

                // Close Button
                Rectangle {
                    implicitWidth: closeBtnText.implicitWidth + (Theme.spacingMd * 2)
                    implicitHeight: 36
                    radius: Theme.radiusSm
                    color: closeBtnMouse.pressed ? Theme.bgSurfaceActive : (closeBtnMouse.containsMouse ? Theme.bgSurfaceHover : "transparent")
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Text {
                        id: closeBtnText
                        anchors.centerIn: parent
                        text: "Close"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Theme.fontWeightMedium
                    }

                    MouseArea {
                        id: closeBtnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.close()
                    }
                }
            }
        }
    }
}
