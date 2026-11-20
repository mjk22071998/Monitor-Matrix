// qmllint disable unqualified
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import "../theme"

Dialog {
    id: root

    property var catalogModel: null
    property int activeTab: 0 // 0: Executable, 1: Web URL
    property string errorMessage: ""
    property bool hasError: false

    signal appAdded(string name, string type)

    modal: true
    focus: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: parent

    width: Math.min(500, parent ? parent.width - 32 : 500)
    padding: 0
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    background: Rectangle {
        color: Theme.bgSurface
        radius: Theme.radiusLg
        border.color: Theme.borderStrong
        border.width: 1

        // Subtle top glow highlight
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.accentCyan
            opacity: 0.3
        }
    }

    Overlay.modal: Rectangle {
        color: Qt.rgba(8 / 255, 11 / 255, 17 / 255, 0.72)
    }

    function openNew(tabIndex) {
        resetFields();
        if (typeof tabIndex === "number") {
            activeTab = tabIndex;
        }
        root.open();
        Qt.callLater(() => {
            appNameField.forceActiveFocus();
        });
    }

    function resetFields() {
        appNameField.text = "";
        appPathField.text = "";
        appUrlField.text = "";
        errorMessage = "";
        hasError = false;
    }

    function urlToPath(fileUrl) {
        let s = fileUrl.toString();
        if (s.indexOf("file:///") === 0) {
            s = s.substring(8);
        } else if (s.indexOf("file://") === 0) {
            s = s.substring(7);
        }
        return decodeURIComponent(s);
    }

    function validateInputs() {
        hasError = false;
        errorMessage = "";

        const name = appNameField.text.trim();
        if (name.length === 0) {
            errorMessage = "Application name is required.";
            hasError = true;
            return false;
        }

        if (activeTab === 0) {
            const path = appPathField.text.trim();
            if (path.length === 0) {
                errorMessage = "Executable file path is required.";
                hasError = true;
                return false;
            }
            const lower = path.toLowerCase();
            if (!lower.endsWith(".exe") && !lower.endsWith(".lnk")) {
                errorMessage = "Target must be a Windows executable (.exe) or shortcut (.lnk).";
                hasError = true;
                return false;
            }
        } else {
            const url = appUrlField.text.trim();
            if (url.length === 0) {
                errorMessage = "Web address (URL) is required.";
                hasError = true;
                return false;
            }
            const lower = url.toLowerCase();
            if (!lower.startsWith("http://") && !lower.startsWith("https://")) {
                errorMessage = "URL must start with http:// or https://";
                hasError = true;
                return false;
            }
            if (!url.includes(".") && !lower.includes("localhost")) {
                errorMessage = "Please enter a valid domain or localhost address.";
                hasError = true;
                return false;
            }
        }

        return true;
    }

    function submit() {
        if (!validateInputs()) {
            return;
        }

        const name = appNameField.text.trim();
        let success = false;

        if (activeTab === 0) {
            const path = appPathField.text.trim();
            if (catalogModel) {
                success = catalogModel.addApplication(path, name);
            }
        } else {
            const url = appUrlField.text.trim();
            if (catalogModel) {
                success = catalogModel.addUrl(url, name);
            }
        }

        if (success) {
            const typeStr = activeTab === 0 ? "Application" : "URL";
            root.appAdded(name, typeStr);
            root.close();
            resetFields();
        } else {
            hasError = true;
            errorMessage = activeTab === 0
                ? "Could not add application. Verify file exists and is not already in catalog."
                : "Could not add URL. Verify link format and that it is not already in catalog.";
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Application Executable"
        nameFilters: ["Executable & Shortcut Files (*.exe *.lnk)", "Executables (*.exe)", "Shortcuts (*.lnk)", "All Files (*.*)"]
        onAccepted: {
            const resolved = root.urlToPath(selectedFile);
            appPathField.text = resolved;

            if (appNameField.text.trim().length === 0) {
                let lastSlash = Math.max(resolved.lastIndexOf("/"), resolved.lastIndexOf("\\"));
                let baseName = (lastSlash >= 0) ? resolved.substring(lastSlash + 1) : resolved;
                let dotIdx = baseName.lastIndexOf(".");
                if (dotIdx > 0) {
                    baseName = baseName.substring(0, dotIdx);
                }
                appNameField.text = baseName;
            }
            root.validateInputs();
        }
    }

    contentItem: Column {
        spacing: 0
        width: parent.width

        // Modal Header
        Item {
            width: parent.width
            height: 56

            Row {
                anchors.left: parent.left
                anchors.leftMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacingSm

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Add Application"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeLarge
                    font.weight: Theme.fontWeightDemiBold
                }
            }

            // Close button
            Rectangle {
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingMd
                anchors.verticalCenter: parent.verticalCenter
                width: 28
                height: 28
                radius: Theme.radiusSm
                color: closeMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "×"
                    color: closeMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeLarge
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.close();
                        root.resetFields();
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.borderSubtle
            }
        }

        // Modal Body
        Column {
            width: parent.width
            topPadding: Theme.spacingLg
            bottomPadding: Theme.spacingLg
            leftPadding: Theme.spacingLg
            rightPadding: Theme.spacingLg
            spacing: Theme.spacingMd

            // Type Segmented Switcher
            Rectangle {
                width: parent.width - (Theme.spacingLg * 2)
                height: 38
                radius: Theme.radiusSm
                color: Theme.bgCanvas
                border.color: Theme.borderSubtle
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 4

                    // Executable Tab
                    Rectangle {
                        width: (parent.width - 4) / 2
                        height: parent.height
                        radius: Theme.radiusSm - 1
                        color: root.activeTab === 0 ? Theme.bgSurfaceActive : "transparent"
                        border.color: root.activeTab === 0 ? Theme.borderStrong : "transparent"
                        border.width: root.activeTab === 0 ? 1 : 0

                        Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                        Row {
                            anchors.centerIn: parent
                            spacing: Theme.spacingXs

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: Theme.tagExecutable
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Executable Program"
                                color: root.activeTab === 0 ? Theme.textPrimary : Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSizeSmall
                                font.weight: root.activeTab === 0 ? Theme.fontWeightMedium : Theme.fontWeightNormal
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.activeTab = 0;
                                root.validateInputs();
                            }
                        }
                    }

                    // Web URL Tab
                    Rectangle {
                        width: (parent.width - 4) / 2
                        height: parent.height
                        radius: Theme.radiusSm - 1
                        color: root.activeTab === 1 ? Theme.bgSurfaceActive : "transparent"
                        border.color: root.activeTab === 1 ? Theme.borderStrong : "transparent"
                        border.width: root.activeTab === 1 ? 1 : 0

                        Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                        Row {
                            anchors.centerIn: parent
                            spacing: Theme.spacingXs

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: Theme.tagUrl
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Web Application / URL"
                                color: root.activeTab === 1 ? Theme.textPrimary : Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSizeSmall
                                font.weight: root.activeTab === 1 ? Theme.fontWeightMedium : Theme.fontWeightNormal
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.activeTab = 1;
                                root.validateInputs();
                            }
                        }
                    }
                }
            }

            // App Name Field
            Column {
                width: parent.width - (Theme.spacingLg * 2)
                spacing: Theme.spacingXs

                Text {
                    text: "Application Name"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightMedium
                }

                TextField {
                    id: appNameField
                    width: parent.width
                    height: 36
                    placeholderText: root.activeTab === 0 ? "e.g. Visual Studio Code" : "e.g. GitHub Dashboard"
                    placeholderTextColor: Theme.textMuted
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    selectByMouse: true
                    selectionColor: Theme.accentCyanGlow
                    selectedTextColor: Theme.textPrimary

                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: Theme.bgCanvas
                        border.color: appNameField.activeFocus
                                      ? Theme.borderFocus
                                      : (root.hasError && appNameField.text.trim().length === 0 ? Theme.accentCrimson : Theme.borderSubtle)
                        border.width: 1
                    }

                    onTextEdited: root.validateInputs()
                    onAccepted: root.submit()
                }
            }

            // Executable Path Field (Tab 0)
            Column {
                width: parent.width - (Theme.spacingLg * 2)
                spacing: Theme.spacingXs
                visible: root.activeTab === 0

                Text {
                    text: "Executable Path (.exe or .lnk)"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightMedium
                }

                Row {
                    width: parent.width
                    spacing: Theme.spacingSm

                    TextField {
                        id: appPathField
                        width: parent.width - browseButton.width - Theme.spacingSm
                        height: 36
                        placeholderText: "C:\\Program Files\\..."
                        placeholderTextColor: Theme.textMuted
                        color: Theme.textPrimary
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontSizeSmall
                        selectByMouse: true
                        selectionColor: Theme.accentCyanGlow
                        selectedTextColor: Theme.textPrimary

                        background: Rectangle {
                            radius: Theme.radiusSm
                            color: Theme.bgCanvas
                            border.color: appPathField.activeFocus
                                          ? Theme.borderFocus
                                          : (root.hasError && appPathField.text.trim().length === 0 ? Theme.accentCrimson : Theme.borderSubtle)
                            border.width: 1
                        }

                        onTextEdited: root.validateInputs()
                        onAccepted: root.submit()
                    }

                    Rectangle {
                        id: browseButton
                        width: 86
                        height: 36
                        radius: Theme.radiusSm
                        color: browseMouse.pressed ? Theme.bgSurfaceActive : (browseMouse.containsMouse ? Theme.bgSurfaceHover : Theme.bgCanvas)
                        border.color: browseMouse.containsMouse ? Theme.borderStrong : Theme.borderSubtle
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "Browse..."
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSmall
                            font.weight: Theme.fontWeightMedium
                        }

                        MouseArea {
                            id: browseMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: fileDialog.open()
                        }
                    }
                }

                Text {
                    text: "Select a native Windows application executable or shortcut file."
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                }
            }

            // Web URL Field (Tab 1)
            Column {
                width: parent.width - (Theme.spacingLg * 2)
                spacing: Theme.spacingXs
                visible: root.activeTab === 1

                Text {
                    text: "Target URL"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Theme.fontWeightMedium
                }

                TextField {
                    id: appUrlField
                    width: parent.width
                    height: 36
                    placeholderText: "https://example.com"
                    placeholderTextColor: Theme.textMuted
                    color: Theme.textPrimary
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontSizeSmall
                    selectByMouse: true
                    selectionColor: Theme.accentCyanGlow
                    selectedTextColor: Theme.textPrimary

                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: Theme.bgCanvas
                        border.color: appUrlField.activeFocus
                                      ? Theme.borderFocus
                                      : (root.hasError && appUrlField.text.trim().length === 0 ? Theme.accentCrimson : Theme.borderSubtle)
                        border.width: 1
                    }

                    onTextEdited: root.validateInputs()
                    onAccepted: root.submit()
                }

                Text {
                    text: "Target will be opened in your default system browser when launched."
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTiny
                }
            }

            // Error Callout Banner
            Rectangle {
                width: parent.width - (Theme.spacingLg * 2)
                height: errorRow.implicitHeight + (Theme.spacingSm * 2)
                radius: Theme.radiusSm
                visible: root.hasError && root.errorMessage.length > 0
                color: Qt.rgba(Theme.accentCrimson.r, Theme.accentCrimson.g, Theme.accentCrimson.b, 0.12)
                border.color: Qt.rgba(Theme.accentCrimson.r, Theme.accentCrimson.g, Theme.accentCrimson.b, 0.4)
                border.width: 1

                Row {
                    id: errorRow
                    anchors.fill: parent
                    anchors.margins: Theme.spacingSm
                    spacing: Theme.spacingSm

                    Image {
                        width: 14
                        height: 14
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/icons/status-warning.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    Text {
                        width: parent.width - 24
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.errorMessage
                        color: Theme.accentCrimson
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        // Modal Footer
        Item {
            width: parent.width
            height: 60

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: Theme.borderSubtle
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: Theme.spacingLg
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.spacingSm

                // Cancel Button
                Rectangle {
                    width: 82
                    height: 34
                    radius: Theme.radiusSm
                    color: cancelMouse.containsMouse ? Theme.bgSurfaceHover : "transparent"
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Theme.fontWeightMedium
                    }

                    MouseArea {
                        id: cancelMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.close();
                            root.resetFields();
                        }
                    }
                }

                // Add Button
                Rectangle {
                    width: 120
                    height: 34
                    radius: Theme.radiusSm
                    color: addMouse.pressed ? Qt.darker(Theme.accentCyan, 1.1) : (addMouse.containsMouse ? Theme.accentCyanHover : Theme.accentCyan)

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Text {
                        anchors.centerIn: parent
                        text: "Add Application"
                        color: "#080B11"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Theme.fontWeightBold
                    }

                    MouseArea {
                        id: addMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.submit()
                    }
                }
            }
        }
    }
}
