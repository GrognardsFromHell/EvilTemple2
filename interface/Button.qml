import Qt 4.7

MouseArea {
    id: root
    property alias text : label.text;
    property alias color: label.color;

    property string normalImage : "art/interface/GENERIC/Accept_Normal.png";
    property string pressedImage : "art/interface/GENERIC/Accept_Pressed.png"
    property string hoverImage : "art/interface/GENERIC/Accept_Hover.png"
    property string disabledImage : "art/interface/GENERIC/Disabled_Normal.png"

    property int fontSize : 14
    property string fontFamily : 'Handserif'
    property bool fontShadow : true

    width: image.width
    height: image.height
    hoverEnabled: true

    Image {
        id: image
        x: 0
        y: 0
        width: sourceSize.width
        height: sourceSize.height
        source: '../' + normalImage
    }

    Image {
        id: imageHover
        x: 0
        y: 0
        width: sourceSize.width
        height: sourceSize.height
        source: '../' + hoverImage
        opacity: 0
    }

    Image {
        id: imagePressed
        x: 0
        y: 0
        width: sourceSize.width
        height: sourceSize.height
        source: '../' + pressedImage
        opacity: 0
    }

    Image {
        id: imageDisabled
        x: 0
        y: 0
        width: sourceSize.width
        height: sourceSize.height
        source: '../' + disabledImage
        opacity: 0
    }

    Text {
        id: label
        text: 'Button'
        font.bold: true
        anchors.fill: parent
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        font.family: root.fontFamily
        font.pointSize: root.fontSize
        color: '#FFFFFF'
        z: 10
        wrapMode: "WrapAtWordBoundaryOrAnywhere"
    }

    Text {
        z: 5
        width: label.width
        height: label.height
        x: label.x + 1
        y: label.y + 1
        text: label.text
        font.bold: label.font.bold
        font.family: label.font.family
        font.pointSize: label.font.pointSize
        color: Qt.darker(Qt.darker(label.color))
        visible: fontShadow
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        wrapMode: "WrapAtWordBoundaryOrAnywhere"
    }

    states: [
       State {
           id: disabledState
             name: 'Disabled'
             when: !root.enabled
             PropertyChanges {
                 target: imageDisabled
                 opacity: 1
             }
             PropertyChanges {
                 target: root
                 onClicked: {}
             }
             PropertyChanges {
                 target: label
                 color: '#333333'
             }
       },
       State {
           id: pressedState
            name: 'MouseDown'
            when: root.containsMouse && root.pressedButtons & Qt.LeftButton
            PropertyChanges {
                target: imagePressed
                opacity: 1
            }
            PropertyChanges {
                target: label
                anchors.horizontalCenterOffset: 1
                anchors.verticalCenterOffset: 1
            }
            StateChangeScript {
                script: gameView.playUiSound('sound/Interface - Button Down.wav')
            }
       },
       State {
           id: hoverState
            name: "Hover"
            when: root.containsMouse || root.pressedButtons & Qt.LeftButton
            PropertyChanges {
                target: imageHover
                opacity: 1
            }
            /*StateChangeScript {
                script: {
                    gameView.playUiSound('sound/Interface - Hover.wav')
                }
            }*/
       }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            animations: NumberAnimation { properties: "opacity"; duration: 100 }

        }
    ]

}
