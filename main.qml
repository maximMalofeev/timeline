import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import my_import 0.1

ApplicationWindow {
  id: root
  width: 650
  minimumWidth: 650
  height: 300
  minimumHeight: 300
  visible: true
  title: qsTr("Timeline")

  Flickable {
    id: flickable
    anchors.fill: parent
    flickableDirection: Flickable.HorizontalFlick
    contentWidth: timeline.width
    contentHeight: timeline.height
    clip: true

    onWidthChanged: {
      if (timeline.width < width) {
        timeline.width = Qt.binding(function(){return width})
      }
    }

    Pane {
      id: timeline
  
      property int groupIntervalInPixel: 100
      property int groupInterval: Math.round(grouper.timelineDuration * groupIntervalInPixel / timeline.width)
      onGroupIntervalChanged: {
        grouper.groupInterval = groupInterval
      }
  
      width: flickable.width
      padding: 0
  
      background: Rectangle {
        border.color: "black"
      }

      Pane {
        id: serifs
        width: parent.width
        height: 40
        padding: 0

        Repeater {
          model: 25

          Item {
            x: parent.width / 24 * modelData
            height: serifs.height
  
            Rectangle {
              id: serif
              anchors.horizontalCenter: parent.horizontalCenter
              color: "black"
              width: 2
              height: parent.height / 2
            }
            Label {
              id: hour
              anchors.horizontalCenter: serif.horizontalCenter
              anchors.bottom: parent.bottom
              text: modelData + "h"
            }
          }
        }
      }
  
      Item {
        anchors.top: serifs.bottom
        anchors.topMargin: 2
  
        Repeater {
          id: repeater
          model: Grouper {
            id: grouper
            timelineDuration: 24 * 60 * 60 * 1000
            groupInterval: timelineDuration / 20
          }
        
          delegate: Control {
            id: control
            x: model.start / grouper.timelineDuration * timeline.width
            width: model.duration / grouper.timelineDuration * timeline.width
            height: 30
            states: State {
              name: "hovered"
              when: control.hovered
              PropertyChanges {
                target: control
                z: 1
              }
              PropertyChanges {
                target: bg
                border.color: "red"
                border.width: 2
                opacity: 1.0
              }
            }

            ToolTip {
              visible: parent.hovered
              y: parent.height + 2

              contentItem: Label {
                id: toolTipLabel
                text: model.toolTip
              }
            }
    
            background: Rectangle {
              id: bg
              color: model.background
              border.color: "black"
              opacity: 0.9
            }
    
            Text {
              anchors.verticalCenter: parent.verticalCenter
              anchors.left: parent.left
              anchors.right: parent.right
              anchors.margins: 2
              text: model.name
              elide: Text.ElideRight
            }
          }
        }
      }
    }
  }

  Popup {
    visible: grouper.busy
    width: root.width
    height: root.height
    background: Rectangle {
      color: "gray"
      opacity: 0.9
    }

    Text {
      anchors.bottom: busyIndicator.top
      anchors.horizontalCenter: busyIndicator.horizontalCenter
      text: "Generating..."
    }
    BusyIndicator {
      id: busyIndicator
      anchors.centerIn: parent
      running: parent.visible
    }
  }

  footer: Pane {
    RowLayout {
      spacing: 20
    
      RowLayout {
        spacing: 4

        Label {
          Layout.alignment: Qt.AlignVCenter
          text: "Bookmarks count"
        }
  
        SpinBox {
          id: spinBox
          editable: true
          from: 0
          to: 100000000
        }
    
        Button {
          text: "Generate"
          onClicked: {
            grouper.generateBookmarks(spinBox.value)
          }
        }
      }
  
      RowLayout {
        spacing: 4

        Label {
          Layout.alignment: Qt.AlignVCenter
          text: "Zoom in/out"
        }
  
        Button {
          text: "+"
          onClicked: {
            timeline.width = timeline.width * 1.2
          }
        }
  
        Button {
          text: "-"
          onClicked: {
            let newWidth = timeline.width / 1.2
            timeline.width = newWidth < root.width ? root.width : newWidth
          }
        }
      }
    }
  }
}
