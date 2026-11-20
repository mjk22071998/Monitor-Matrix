// qmllint disable unqualified
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import "theme"
import "components"
import "views"
import "dialogs"

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    minimumWidth: 960
    minimumHeight: 540
    visible: true
    color: Theme.bgBase

    title: (screenDropEngine && screenDropEngine.currentFileName.length > 0
            ? screenDropEngine.currentFileName
            : "Untitled Layout")
           + (screenDropEngine && screenDropEngine.isDirty ? " * - " : " - ")
           + "Screen Drop"

    Component.onCompleted: {
        if (screenDropEngine) {
            Theme.isDark = screenDropEngine.darkTheme;
        }
    }

    Connections {
        target: screenDropEngine
        function onNotificationRequested(title: string, message: string, isError: bool) {
            notificationToast.show(title, message, isError);
        }
        function onMonitorMismatchDetected(savedCount: int, currentCount: int) {
            notificationToast.show(
                "Monitor Mismatch",
                "Saved layout was designed for " + savedCount + " display(s), but " + currentCount + " are currently detected.",
                true
            );
        }
        function onDarkThemeChanged() {
            if (screenDropEngine) {
                Theme.isDark = screenDropEngine.darkTheme;
            }
        }
    }

    // Keyboard Shortcuts
    Shortcut {
        sequence: StandardKey.Save
        onActivated: headerBar.saveLayoutClicked()
    }

    Shortcut {
        sequence: "Ctrl+Shift+S"
        onActivated: headerBar.saveAsLayoutClicked()
    }

    Shortcut {
        sequence: StandardKey.Open
        onActivated: headerBar.openLayoutClicked()
    }

    Shortcut {
        sequence: StandardKey.New
        onActivated: headerBar.newLayoutClicked()
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: headerBar.runLayoutClicked()
    }

    // File Dialogs
    FileDialog {
        id: openFileDialog
        title: "Open Screen Drop Layout"
        nameFilters: ["Screen Drop Layouts (*.sav)", "All Files (*.*)"]
        onAccepted: {
            if (screenDropEngine) {
                screenDropEngine.loadLayout(selectedFile);
            }
        }
    }

    FileDialog {
        id: saveFileDialog
        title: "Save Screen Drop Layout"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Screen Drop Layouts (*.sav)", "All Files (*.*)"]
        defaultSuffix: "sav"
        onAccepted: {
            if (screenDropEngine) {
                screenDropEngine.saveLayout(selectedFile);
            }
        }
    }

    // Top Navigation Header Bar
    HeaderBar {
        id: headerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentFileName: screenDropEngine ? screenDropEngine.currentFileName : ""
        isDirty: screenDropEngine ? screenDropEngine.isDirty : false
        isRunning: screenDropEngine ? screenDropEngine.isRunning : false
        hasOverlaps: screenDropEngine ? screenDropEngine.hasOverlaps : false
        invalidCount: screenDropEngine ? screenDropEngine.invalidCount : 0
        darkTheme: screenDropEngine ? screenDropEngine.darkTheme : true

        onRunLayoutClicked: {
            if (screenDropEngine) {
                screenDropEngine.runLayout();
            }
        }

        onNewLayoutClicked: {
            if (screenDropEngine) {
                screenDropEngine.newLayout();
            }
        }

        onOpenLayoutClicked: {
            openFileDialog.open();
        }

        onSaveLayoutClicked: {
            if (screenDropEngine) {
                if (screenDropEngine.currentFilePath && screenDropEngine.currentFilePath.length > 0) {
                    screenDropEngine.saveCurrentLayout();
                } else {
                    saveFileDialog.open();
                }
            }
        }

        onSaveAsLayoutClicked: {
            saveFileDialog.open();
        }

        onScreenSettingsClicked: {
            if (screenDropEngine) {
                screenDropEngine.notify("Display Setup", "Display arrangement drawer will open here.", false);
            }
        }

        onOpenLogsClicked: {
            if (screenDropEngine) {
                screenDropEngine.openLogsFolder();
            }
        }

        onThemeToggleClicked: {
            if (screenDropEngine) {
                screenDropEngine.darkTheme = !screenDropEngine.darkTheme;
            }
        }
    }

    // Central Application Area
    Item {
        id: mainContentArea
        anchors.top: headerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Left App Catalog Sidebar
        AppSidebar {
            id: appSidebar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            catalogModel: typeof appCatalogModel !== "undefined" ? appCatalogModel : (typeof catalogModel !== "undefined" ? catalogModel : null)

            onAddAppClicked: {
                appEditModal.openNew(0);
            }

            onAppRemoved: (id, name) => {
                notificationToast.show("App Removed", "'" + name + "' was removed from the catalog.", false);
            }
        }

        // Central Canvas Area
        CanvasView {
            id: canvasView
            anchors.top: parent.top
            anchors.left: appSidebar.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            canvasModel: typeof canvasModel !== "undefined" ? canvasModel : (typeof monitorCanvasModel !== "undefined" ? monitorCanvasModel : null)
            placementModel: typeof placementModel !== "undefined" ? placementModel : null
        }
    }

    // Add / Edit Application Modal Dialog
    AppEditModal {
        id: appEditModal
        catalogModel: typeof appCatalogModel !== "undefined" ? appCatalogModel : (typeof catalogModel !== "undefined" ? catalogModel : null)

        onAppAdded: (name, type) => {
            notificationToast.show(
                type === "URL" ? "Web Link Added" : "Application Added",
                "'" + name + "' was successfully added to your catalog.",
                false
            );
        }
    }

    // Floating Notification Toast HUD
    NotificationToast {
        id: notificationToast
        anchors.top: headerBar.bottom
        anchors.topMargin: Theme.spacingMd
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLg
        z: 999
    }
}
