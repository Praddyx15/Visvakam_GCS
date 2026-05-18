import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Checklist

Rectangle {
    id:     root
    color:  qgcPal.window

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    readonly property color _accentColor:  "#73BAF2"
    readonly property color _textColor:    "#E9E9E9"
    readonly property color _bgColor:      "#2B2B2B"
    readonly property color _readyColor:   "#4CAF50"
    readonly property color _notReadyColor:"#F44336"
    readonly property color _warnColor:    "#FF9800"
    readonly property real  _pad:          ScreenTools.defaultFontPixelWidth

    // -------------------------------------------------------------------------
    // Header bar (matches toolbar height)
    Rectangle {
        id:             header
        anchors.top:    parent.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         ScreenTools.toolbarHeight
        color:          qgcPal.toolbarBackground

        QGCLabel {
            anchors.centerIn:   parent
            text:               qsTr("Pre-Flight Checklist")
            font.pointSize:     ScreenTools.largeFontPointSize
            font.family:        "Bai Jamjuree"
            color:              root._textColor
        }
    }

    // -------------------------------------------------------------------------
    // Ready-to-Arm banner
    Rectangle {
        id:                 banner
        anchors.top:        header.bottom
        anchors.left:       parent.left
        anchors.right:      parent.right
        height:             ScreenTools.defaultFontPixelHeight * 2.4
        color:              ChecklistModel.readyToArm ? root._readyColor : root._notReadyColor

        RowLayout {
            anchors.centerIn:   parent
            spacing:            _pad

            QGCLabel {
                text:       ChecklistModel.readyToArm ? "✓" : "✗"
                color:      "white"
                font.pointSize: ScreenTools.defaultFontPointSize * 1.4
                font.bold:  true
            }

            QGCLabel {
                text:       ChecklistModel.readyToArm
                                ? qsTr("Ready to Arm")
                                : qsTr("Not Ready")
                color:      "white"
                font.pointSize: ScreenTools.defaultFontPointSize * 1.1
                font.bold:  true
            }
        }
    }

    // -------------------------------------------------------------------------
    // Checklist list
    ListView {
        id:                 listView
        anchors.top:        banner.bottom
        anchors.left:       parent.left
        anchors.right:      parent.right
        anchors.bottom:     bottomBar.top
        anchors.margins:    _pad
        clip:               true
        model:              ChecklistModel
        spacing:            _pad / 2

        section.property:   "section"
        section.criteria:   ViewSection.FullString
        section.delegate:   sectionDelegate

        delegate:           itemDelegate

        QGCLabel {
            anchors.centerIn:   parent
            visible:            listView.count === 0
            text:               qsTr("No checklist items loaded")
            color:              qgcPal.text
            font.pointSize:     ScreenTools.defaultFontPointSize * 1.1
        }
    }

    Component {
        id: sectionDelegate

        Rectangle {
            width:  listView.width
            height: sectionLabel.implicitHeight + _pad
            color:  "transparent"

            QGCLabel {
                id:             sectionLabel
                anchors.left:   parent.left
                anchors.right:  parent.right
                anchors.verticalCenter: parent.verticalCenter
                text:           section
                font.bold:      true
                font.pointSize: ScreenTools.defaultFontPointSize * 1.05
                color:          root._accentColor
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left:   parent.left
                anchors.right:  parent.right
                height:         1
                color:          root._accentColor
                opacity:        0.4
            }
        }
    }

    Component {
        id: itemDelegate

        Rectangle {
            id:     itemRoot
            width:  listView.width
            height: itemLayout.implicitHeight + _pad * 1.5
            color:  itemMouseArea.containsMouse ? Qt.rgba(0.15, 0.15, 0.15, 0.6) : "transparent"
            radius: 4

            RowLayout {
                id:                 itemLayout
                anchors.left:       parent.left
                anchors.right:      parent.right
                anchors.top:        parent.top
                anchors.leftMargin: _pad
                anchors.rightMargin: _pad
                anchors.topMargin:  _pad * 0.75
                spacing:            _pad

                CheckBox {
                    id:         itemCheck
                    checked:    model.checked
                    onClicked:  ChecklistModel.setChecked(model.id, checked)

                    indicator: Rectangle {
                        implicitWidth:  ScreenTools.defaultFontPixelHeight * 1.4
                        implicitHeight: implicitWidth
                        radius:         3
                        color:          itemCheck.checked ? root._accentColor : "transparent"
                        border.color:   itemCheck.checked ? root._accentColor : "#888"
                        border.width:   2

                        QGCLabel {
                            anchors.centerIn:   parent
                            text:               "✓"
                            color:              "white"
                            font.bold:          true
                            visible:            itemCheck.checked
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing:          _pad / 4

                    QGCLabel {
                        Layout.fillWidth:   true
                        text:               model.label
                        font.bold:          model.severity === "blocking"
                        color:              qgcPal.text
                        wrapMode:           Text.WordWrap
                    }

                    QGCLabel {
                        Layout.fillWidth:   true
                        text:               model.description
                        font.pointSize:     ScreenTools.smallFontPointSize
                        color:              Qt.rgba(0.7, 0.7, 0.7, 1)
                        wrapMode:           Text.WordWrap
                        visible:            text.length > 0
                    }
                }

                Rectangle {
                    width:   severityLabel.implicitWidth + _pad
                    height:  severityLabel.implicitHeight + _pad / 2
                    radius:  height / 2
                    color:   model.severity === "blocking" ? root._notReadyColor : root._warnColor

                    QGCLabel {
                        id:                 severityLabel
                        anchors.centerIn:   parent
                        text:               model.severity === "blocking"
                                                ? qsTr("BLOCKING")
                                                : qsTr("WARNING")
                        font.pointSize:     ScreenTools.smallFontPointSize * 0.9
                        font.bold:          true
                        color:              "white"
                    }
                }
            }

            MouseArea {
                id:                 itemMouseArea
                anchors.fill:       parent
                hoverEnabled:       true
                onClicked:          ChecklistModel.setChecked(model.id, !model.checked)
            }
        }
    }

    // -------------------------------------------------------------------------
    // Bottom bar
    Rectangle {
        id:                 bottomBar
        anchors.left:       parent.left
        anchors.right:      parent.right
        anchors.bottom:     parent.bottom
        height:             ScreenTools.defaultFontPixelHeight * 3
        color:              qgcPal.toolbarBackground

        QGCButton {
            anchors.centerIn:   parent
            text:               qsTr("Reset Checklist")
            onClicked:          ChecklistModel.reset()
        }
    }
}
