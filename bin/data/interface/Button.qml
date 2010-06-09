import Qt 4.7

MouseArea {
    id: root
    property alias text : label.text;
    property alias color: label.color;

    property string normalImage : "art/interface/GENERIC/Accept_Normal.png";
    property string hoverImage : "art/interface/GENERIC/Accept_Hover.png";
    property string pressedImage : "art/interface/GENERIC/Accept_Pressed.png";
    property string disabledImage : "art/interface/GENERIC/Disabled_Normal.png";

    width: image.width
    height: image.height
    hoverEnabled: true

    Image {
        id: image
        x: 0
        y: 0
        source: '../' + normalImage
        width: sourceSize.width
        height: sourceSize.height

        Text {
            id: label
            text: 'Button'
            font.bold: true
            smooth: true
            anchors.centerIn: parent
            color: '#FFFFFF'
            font.family: 'Handserif'
            font.pointSize: 14
        }

    }
    states: [
       State {
             name: 'Disabled'
             when: !root.enabled
             PropertyChanges {
                 target: image
                 source: '../' + disabledImage
             }
             PropertyChanges {
                 target: label
                 color: '#333333'
             }
             PropertyChanges {
                 target: root
                 onClicked: function() {}
             }
       },
       State {
            name: 'MouseDown'
            when: root.containsMouse && root.pressedButtons & Qt.LeftButton
            PropertyChanges {
                target: image
                source: '../' + pressedImage
            }
            PropertyChanges {
                target: label
                anchors.horizontalCenterOffset: 2
                anchors.verticalCenterOffset: 1
            }
       },
       State {
            name: "Hover"
            when: root.containsMouse || root.pressedButtons & Qt.LeftButton
            PropertyChanges {
                target: image
                source: '../' + hoverImage
            }
       }
    ]


}
