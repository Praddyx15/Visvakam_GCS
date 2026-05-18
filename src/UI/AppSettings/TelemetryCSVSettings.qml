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
        text: {
            if (Qt.platform.os === "osx") {
                return qsTr("Show in Finder");
            } else if (Qt.platform.os === "windows") {
                return qsTr("Show in Explorer");
            } else {
                return qsTr("Show in Folder");
            }
        }
        visible:    Qt.platform.os === "osx" || Qt.platform.os === "windows" || Qt.platform.os === "linux"
        onClicked: {
            var prefix = "file://";
            if (Qt.platform.os === "windows") {
                prefix = "file:///";
            }
            Qt.openUrlExternally(prefix + _logsPath);
        }
    }
}
