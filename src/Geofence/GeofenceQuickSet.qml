import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Geofence

QGCPopupDialog {
    id:      _root
    title:   qsTr("Set Circular Geofence")
    buttons: Dialog.Apply | Dialog.Cancel

    property real _margin: ScreenTools.defaultFontPixelWidth

    onAccepted: {
        var radius = parseFloat(radiusField.text)
        var alt    = parseFloat(altField.text)
        if (isNaN(radius) || radius < 50 || radius > 5000) {
            mainWindow.showCriticalVehicleMessage(qsTr("Radius must be between 50 and 5000 m"))
            preventClose = true
            return
        }
        if (isNaN(alt) || alt < 30 || alt > 500) {
            mainWindow.showCriticalVehicleMessage(qsTr("Max altitude must be between 30 and 500 m"))
            preventClose = true
            return
        }
        GeofenceQuickSetController.applyCircularGeofence(radius, alt)
    }

    GridLayout {
        columns:        2
        columnSpacing:  _margin * 2
        rowSpacing:     _margin

        QGCLabel {
            text:                   qsTr("Radius (m)")
            font.family:            "Bai Jamjuree"
        }

        QGCTextField {
            id:                     radiusField
            Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 12
            text:                   "500"
            inputMethodHints:       Qt.ImhFormattedNumbersOnly
            validator:              IntValidator { bottom: 50; top: 5000 }
        }

        QGCLabel {
            text:                   qsTr("Max Altitude (m AGL)")
            font.family:            "Bai Jamjuree"
        }

        QGCTextField {
            id:                     altField
            Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 12
            text:                   "120"
            inputMethodHints:       Qt.ImhFormattedNumbersOnly
            validator:              IntValidator { bottom: 30; top: 500 }
        }
    }
}
