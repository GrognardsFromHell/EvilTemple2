import Qt 4.7

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    property bool complete : false
    property real chosenHeight: 0.5
    property int minHeight : 100
    property int maxHeight : 200

    Component.onCompleted: {
        slider.y = slider.drag.minimumY + (slider.drag.maximumY - slider.drag.minimumY) * chosenHeight;
        updateSliderPosition();
        chosenHeightChanged(); // The position is valid initially
    }

    Text {
        x: 5
        y: 5
        width: 427
        height: 17
        color: "#ffffff"
        font.pointSize: 12
        font.family: "Fontin"
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        text: "Choose your Character's Height"
    }

    Image {
        x: 198
        y: 26
        source: '../art/interface/pc_creation/height_bar.png'

        MouseArea {
            id: slider
            drag.minimumY: 13
            drag.maximumY: 176
            drag.axis: Drag.YAxis
            drag.target: slider

            x: 6
            y: 13

            width: childrenRect.width
            height: childrenRect.height

            Image {
                source: '../art/interface/pc_creation/height_slider.png'
            }

            onMousePositionChanged: {
                updateSliderPosition();
            }
        }

        Text {
            id: sliderValueLabel
            anchors.verticalCenter: slider.verticalCenter
            anchors.left: slider.right
            text: '123'
            color: "#ffffff"
            font.pointSize: 17
            font.bold: true
            font.family: "Fontin"
        }
    }

    StandardText {
        id: standardtext1
        x: 138
        y: 35
        width: 52
        height: 18
        horizontalAlignment: "AlignRight"
        text: maxHeight + "cm"
    }

    StandardText {
        id: standardtext2
        x: 138
        y: 198
        width: 52
        height: 18
        horizontalAlignment: "AlignRight"
        text: minHeight + "cm"
    }

    function updateSliderPosition() {
        var f = 1.0 - (slider.y - slider.drag.minimumY) / (slider.drag.maximumY - slider.height);
        var val = Math.floor(minHeight + (maxHeight - minHeight) * f);

        sliderValueLabel.text = val + "cm";
        chosenHeight = f;
    }

}
