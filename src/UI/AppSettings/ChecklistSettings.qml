import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Checklist

ColumnLayout {
    spacing: ScreenTools.defaultFontPixelHeight / 2

    property var _fact: QGroundControl.settingsManager.appSettings.checklistCustomPath

    // Current path row
    RowLayout {
        Layout.fillWidth: true
        spacing:          ScreenTools.defaultFontPixelWidth

        QGCLabel {
            text:   qsTr("Custom Checklist JSON:")
            color:  qgcPal.text
        }

        QGCTextField {
            Layout.fillWidth:   true
            text:               _fact.rawValue
            placeholderText:    qsTr("(using built-in default)")
            readOnly:           true
        }
    }

    // Buttons row
    RowLayout {
        Layout.fillWidth: true
        spacing:          ScreenTools.defaultFontPixelWidth

        QGCButton {
            text:       qsTr("Browse…")
            visible:    !ScreenTools.isMobile
            onClicked:  fileDialog.open()
        }

        QGCButton {
            text:       qsTr("Reset to Default")
            onClicked: {
                _fact.rawValue = ""
                ChecklistModel.loadFromFile("")
            }
        }
    }

    QGCPalette { id: qgcPal }

    FileDialog {
        id:             fileDialog
        title:          qsTr("Select Checklist JSON")
        nameFilters:    ["JSON files (*.json)"]
        onAccepted: {
            var path = selectedFile.toString().replace(/^file:\/\//, "")
            // For Windows paths, SelectedFile may contain file:///C:/... which becomes /C:/... or C:/...
            // Standardise path so it works across platforms
            if (path.match(/^\/[A-Za-z]:/)) {
                path = path.substring(1);
            }
            _fact.rawValue = path
            ChecklistModel.loadFromFile(path)
        }
    }
}
