import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.FactControls
import QGroundControl.Controls

ColumnLayout {
    spacing: ScreenTools.defaultFontPixelHeight / 2

    property var    _fact:      QGroundControl.settingsManager.appSettings.recordTelemetryCSV
    property string _logsPath:  QGroundControl.settingsManager.appSettings.telemetryCSVSavePath

    FactCheckBoxSlider {
        Layout.fillWidth:   true
        text:               _fact.label
        fact:               _fact
        visible:            _fact.userVisible
    }

    QGCButton {
        text:       qsTr("Show in Finder")
        visible:    Qt.platform.os === "osx"
        onClicked:  Qt.openUrlExternally("file://" + _logsPath)
    }
}
