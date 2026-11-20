// qmllint disable unqualified
import QtQuick
import QtQuick.Controls
import "../theme"

Rectangle {
    id: root

    property var catalogModel: typeof appCatalogModel !== "undefined" ? appCatalogModel : null
    property bool isCollapsed: false
    property int expandedWidth: 290
    property int collapsedWidth: 52

    signal addAppClicked()
    signal appRemoved(string id, string name)

    implicitWidth: isCollapsed ? collapsedWidth : expandedWidth
    color: Theme.bgSurface
    border.color: Theme.borderSubtle
    border.width: 0

    Behavior on implicitWidth {
        NumberAnimation {
            duration: Theme.durationNormal
            easing.type: Theme.easingType
        }
    }

    // Right border separator
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: Theme.borderSubtle
    }

    // Collapsed Vertical Bar
    Item {
        id: collapsedBar
        anchors.fill: parent
        visible: root.isCollapsed

        Column {
            anchors.top: parent.top
            anchors.topMargin: Theme.spacingMd
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.spacingMd

            // Expand Toggle Button
            Rectangle {
                width: 36
                height: 36
                radius: Theme.radiusSm
                color: expandMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"
                border.color: expandMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "▶"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                ToolTip.visible: expandMouse.containsMouse
                ToolTip.delay: 350
                ToolTip.text: "Expand App Catalog"

                MouseArea {
                    id: expandMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.isCollapsed = false
                }
            }

            // Add App Quick Button
            Rectangle {
                width: 36
                height: 36
                radius: Theme.radiusSm
                color: quickAddMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"
                border.color: quickAddMouse.containsMouse ? Theme.accentCyan : Theme.borderSubtle
                border.width: 1

                Image {
                    width: 14
                    height: 14
                    anchors.centerIn: parent
                    source: "qrc:/icons/action-add.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                ToolTip.visible: quickAddMouse.containsMouse
                ToolTip.delay: 350
                ToolTip.text: "Add New Application"

                MouseArea {
                    id: quickAddMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.addAppClicked()
                }
            }

            // Divider
            Rectangle {
                width: 24
                height: 1
                color: Theme.borderSubtle
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Compact App Count Badge
            Rectangle {
                width: 32
                height: 20
                radius: Theme.radiusPill
                color: Theme.bgCanvas
                border.color: Theme.borderSubtle
                border.width: 1
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    anchors.centerIn: parent
                    text: root.catalogModel ? root.catalogModel.count.toString() : "0"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                    font.weight: Theme.fontWeightMedium
                }
            }
        }
    }

    // Expanded Full Content Container
    Item {
        id: expandedContent
        anchors.fill: parent
        visible: !root.isCollapsed
        clip: true

        // Top Header Area
        Item {
            id: headerArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 98

            // Title Row
            Row {
                anchors.top: parent.top
                anchors.topMargin: Theme.spacingMd
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingMd
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingMd
                spacing: Theme.spacingSm

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Catalog"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeRegular
                    font.weight: Theme.fontWeightDemiBold
                }

                // Count Pill
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    implicitWidth: countText.implicitWidth + 12
                    implicitHeight: 20
                    radius: Theme.radiusPill
                    color: Theme.bgCanvas
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Text {
                        id: countText
                        anchors.centerIn: parent
                        text: root.catalogModel ? root.catalogModel.count.toString() : "0"
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeTiny
                        font.weight: Theme.fontWeightMedium
                    }
                }

                Item {
                    // Spacer
                    width: Math.max(0, headerArea.width - 240)
                    height: 1
                }

                // "+ Add" Button
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    implicitWidth: addBtnRow.implicitWidth + (Theme.spacingSm * 2)
                    implicitHeight: 28
                    radius: Theme.radiusSm
                    color: addBtnMouse.containsMouse ? Theme.bgSurfaceHover : Theme.bgCanvas
                    border.color: addBtnMouse.containsMouse ? Theme.accentCyan : Theme.borderSubtle
                    border.width: 1

                    Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }
                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Row {
                        id: addBtnRow
                        anchors.centerIn: parent
                        spacing: 4

                        Image {
                            width: 11
                            height: 11
                            anchors.verticalCenter: parent.verticalCenter
                            source: "qrc:/icons/action-add.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Add"
                            color: addBtnMouse.containsMouse ? Theme.accentCyan : Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSmall
                            font.weight: Theme.fontWeightMedium
                        }
                    }

                    ToolTip.visible: addBtnMouse.containsMouse
                    ToolTip.delay: 350
                    ToolTip.text: "Add new application or URL to catalog"

                    MouseArea {
                        id: addBtnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.addAppClicked()
                    }
                }

                // Collapse Button
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 24
                    height: 24
                    radius: Theme.radiusSm
                    color: collapseMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "◀"
                        color: Theme.textMuted
                        font.pixelSize: 10
                    }

                    ToolTip.visible: collapseMouse.containsMouse
                    ToolTip.delay: 350
                    ToolTip.text: "Collapse catalog sidebar"

                    MouseArea {
                        id: collapseMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.isCollapsed = true
                    }
                }
            }

            // Search Bar
            Rectangle {
                id: searchBarContainer
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingMd
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingMd
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.spacingSm
                height: 32
                radius: Theme.radiusSm
                color: Theme.bgCanvas
                border.color: searchInput.activeFocus ? Theme.borderFocus : Theme.borderSubtle
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.spacingSm
                    anchors.rightMargin: Theme.spacingSm
                    spacing: Theme.spacingXs

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "⌕"
                        color: Theme.textMuted
                        font.pixelSize: 15
                    }

                    TextInput {
                        id: searchInput
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 40
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        selectByMouse: true
                        selectionColor: Theme.accentCyanGlow
                        selectedTextColor: Theme.textPrimary

                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            text: "Search apps..."
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSmall
                            visible: searchInput.text.length === 0 && !searchInput.activeFocus
                        }

                        onTextChanged: {
                            if (root.catalogModel) {
                                root.catalogModel.setSearchFilter(text);
                            }
                        }
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 16
                        height: 16
                        radius: 8
                        color: clearMouse.containsMouse ? Theme.bgSurfaceActive : "transparent"
                        visible: searchInput.text.length > 0

                        Text {
                            anchors.centerIn: parent
                            text: "×"
                            color: Theme.textMuted
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: clearMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                searchInput.text = "";
                                searchInput.forceActiveFocus();
                            }
                        }
                    }
                }
            }

            // Divider
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.borderSubtle
            }
        }

        // Applications List View
        ListView {
            id: appListView
            anchors.top: headerArea.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: Theme.spacingSm
            clip: true
            spacing: Theme.spacingXs
            model: root.catalogModel

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Item {
                id: delegateItem
                width: appListView.width
                height: 58

                // Public properties accessible to DropArea via drop.source
                readonly property string appId: model.id
                readonly property string appName: model.name
                readonly property string appType: model.type
                readonly property string appTarget: model.target
                readonly property string appIconSource: model.iconSource
                readonly property bool appIsPlaced: model.isPlaced

                // Drag Proxy Item
                Item {
                    id: dragProxy
                    width: 1
                    height: 1
                }

                Drag.active: dragMouseArea.drag.active
                Drag.source: delegateItem
                Drag.dragType: Drag.Automatic
                Drag.hotSpot.x: delegateCard.width / 2
                Drag.hotSpot.y: delegateCard.height / 2
                Drag.keys: ["application", "app", model.type]
                Drag.mimeData: {
                    "application/x-app-id": model.id,
                    "application/x-app-name": model.name,
                    "application/x-app-type": model.type,
                    "appId": model.id,
                    "appName": model.name,
                    "appType": model.type,
                    "target": model.target,
                    "text/plain": model.name
                }

                // Context Menu for Removal
                Menu {
                    id: contextMenu

                    MenuItem {
                        text: "Remove from Catalog"
                        onTriggered: {
                            if (root.catalogModel) {
                                root.catalogModel.removeApplication(model.id);
                            }
                            root.appRemoved(model.id, model.name);
                        }
                    }
                }

                // Visual Card
                Rectangle {
                    id: delegateCard
                    anchors.fill: parent
                    radius: Theme.radiusMd
                    color: dragMouseArea.drag.active
                           ? Theme.bgSurfaceActive
                           : (delegateMouse.containsMouse ? Theme.bgSurfaceHover : Theme.bgSurface)
                    border.color: dragMouseArea.drag.active
                                  ? Theme.accentCyan
                                  : (delegateMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle)
                    border.width: 1
                    opacity: dragMouseArea.drag.active ? 0.65 : 1.0

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                    Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: Theme.spacingSm
                        anchors.rightMargin: Theme.spacingSm
                        spacing: Theme.spacingSm

                        // Icon Container
                        Rectangle {
                            id: iconBox
                            anchors.verticalCenter: parent.verticalCenter
                            width: 36
                            height: 36
                            radius: Theme.radiusSm
                            color: Theme.bgCanvas
                            border.color: Theme.borderSubtle
                            border.width: 1

                            Image {
                                id: appIconImg
                                anchors.fill: parent
                                anchors.margins: 4
                                source: model.iconSource
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                visible: status === Image.Ready
                            }

                            // Fallback glyph/badge
                            Rectangle {
                                anchors.fill: parent
                                radius: Theme.radiusSm
                                visible: !appIconImg.visible
                                color: model.type === "URL"
                                       ? Qt.rgba(Theme.tagUrl.r, Theme.tagUrl.g, Theme.tagUrl.b, 0.15)
                                       : Qt.rgba(Theme.tagExecutable.r, Theme.tagExecutable.g, Theme.tagExecutable.b, 0.15)

                                Text {
                                    anchors.centerIn: parent
                                    text: model.type === "URL" ? "URL" : (model.name.length > 0 ? model.name.charAt(0).toUpperCase() : "A")
                                    color: model.type === "URL" ? Theme.tagUrl : Theme.tagExecutable
                                    font.family: Theme.fontFamily
                                    font.pixelSize: Theme.fontSizeSmall
                                    font.weight: Theme.fontWeightBold
                                }
                            }
                        }

                        // Text Metadata
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - iconBox.width - rightActionCol.width - (Theme.spacingSm * 2)
                            spacing: 2

                            Text {
                                width: parent.width
                                text: model.name
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSizeSmall
                                font.weight: Theme.fontWeightMedium
                                elide: Text.ElideRight
                            }

                            Row {
                                width: parent.width
                                spacing: 4

                                // Type Tag
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    implicitWidth: typeTagText.implicitWidth + 6
                                    implicitHeight: 14
                                    radius: 3
                                    color: model.type === "URL"
                                           ? Qt.rgba(Theme.tagUrl.r, Theme.tagUrl.g, Theme.tagUrl.b, 0.15)
                                           : Qt.rgba(Theme.tagExecutable.r, Theme.tagExecutable.g, Theme.tagExecutable.b, 0.15)

                                    Text {
                                        id: typeTagText
                                        anchors.centerIn: parent
                                        text: model.type === "URL" ? "URL" : "EXE"
                                        color: model.type === "URL" ? Theme.tagUrl : Theme.tagExecutable
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeTiny
                                        font.weight: Theme.fontWeightBold
                                    }
                                }

                                // Target Path / URL
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - typeTagText.implicitWidth - 10
                                    text: model.target
                                    color: Theme.textMuted
                                    font.family: Theme.fontMono
                                    font.pixelSize: Theme.fontSizeTiny
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        // Right Status & Actions Area
                        Row {
                            id: rightActionCol
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            // Placed Indicator Pill
                            Rectangle {
                                id: placedPill
                                anchors.verticalCenter: parent.verticalCenter
                                implicitWidth: placedText.implicitWidth + 10
                                implicitHeight: 18
                                radius: Theme.radiusPill
                                color: Theme.accentEmeraldMuted
                                border.color: Qt.rgba(Theme.accentEmerald.r, Theme.accentEmerald.g, Theme.accentEmerald.b, 0.4)
                                border.width: 1
                                visible: model.isPlaced

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 3

                                    Rectangle {
                                        width: 4
                                        height: 4
                                        radius: 2
                                        color: Theme.accentEmerald
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        id: placedText
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "Placed"
                                        color: Theme.accentEmerald
                                        font.family: Theme.fontFamily
                                        font.pixelSize: Theme.fontSizeTiny
                                        font.weight: Theme.fontWeightMedium
                                    }
                                }
                            }

                            // Context Menu Button (Hover visible)
                            Rectangle {
                                id: menuBtn
                                anchors.verticalCenter: parent.verticalCenter
                                width: 20
                                height: 20
                                radius: Theme.radiusSm
                                color: menuBtnMouse.containsMouse ? Theme.bgSurfaceActive : "transparent"
                                visible: delegateMouse.containsMouse

                                Text {
                                    anchors.centerIn: parent
                                    text: "⋮"
                                    color: menuBtnMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                                    font.pixelSize: 13
                                }

                                MouseArea {
                                    id: menuBtnMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: contextMenu.popup()
                                }
                            }

                            // Drag Grip Handle Icon
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "⋮⋮"
                                color: Theme.textMuted
                                font.pixelSize: 11
                                visible: !model.isPlaced && delegateMouse.containsMouse && !menuBtn.visible
                                opacity: 0.6
                            }
                        }
                    }

                    // Card Mouse Area for Hover, Context Menu, and Dragging
                    MouseArea {
                        id: delegateMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.RightButton
                        cursorShape: dragMouseArea.drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup();
                            }
                        }
                    }

                    MouseArea {
                        id: dragMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                        drag.target: dragProxy
                        drag.axis: Drag.XAndYAxis
                        drag.threshold: 5

                        onReleased: {
                            dragProxy.x = 0;
                            dragProxy.y = 0;
                        }
                    }
                }
            }
        }

        // Empty Search Results State
        Column {
            anchors.centerIn: appListView
            width: parent.width - (Theme.spacingLg * 2)
            spacing: Theme.spacingSm
            visible: root.catalogModel && root.catalogModel.count === 0 && searchInput.text.trim().length > 0

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "No applications found"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeRegular
                font.weight: Theme.fontWeightMedium
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "No entries match \"" + searchInput.text + "\""
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                implicitWidth: clearBtnText.implicitWidth + 24
                implicitHeight: 28
                radius: Theme.radiusSm
                color: clearFilterMouse.containsMouse ? Theme.bgSurfaceHover : Theme.bgCanvas
                border.color: Theme.borderSubtle
                border.width: 1

                Text {
                    id: clearBtnText
                    anchors.centerIn: parent
                    text: "Clear Search"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                }

                MouseArea {
                    id: clearFilterMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: searchInput.text = ""
                }
            }
        }

        // Entirely Empty Catalog State
        Column {
            anchors.centerIn: appListView
            width: parent.width - (Theme.spacingLg * 2)
            spacing: Theme.spacingSm
            visible: root.catalogModel && root.catalogModel.count === 0 && searchInput.text.trim().length === 0

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "No Applications Added"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeRegular
                font.weight: Theme.fontWeightMedium
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "Add programs or web URLs to organize them onto your monitors."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                implicitWidth: addFirstText.implicitWidth + 24
                implicitHeight: 32
                radius: Theme.radiusSm
                color: addFirstMouse.containsMouse ? Theme.accentCyanHover : Theme.accentCyan

                Text {
                    id: addFirstText
                    anchors.centerIn: parent
                    text: "+ Add Application"
                    color: "#080B11"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightBold
                }

                MouseArea {
                    id: addFirstMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.addAppClicked()
                }
            }
        }
    }
}
