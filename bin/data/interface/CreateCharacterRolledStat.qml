import Qt 4.7

Text {
    text: '18'
    color: '#FFFFFF'
    font.bold: true
    font.pointSize: 17
    smooth: true
    z: 100

    property variant relateTo;

    MouseArea {
        anchors.fill: parent
        drag.target: parent
        drag.axis: Drag.XandYAxis
        drag.minimumX: 0
        drag.maximumX: statsRollGroup.width - parent.width
        drag.minimumY: 0
        drag.maximumY: statsRollGroup.height - parent.height
        onPressed: console.log("Pressed")
        onReleased: {
            console.log("Released")

            var centerX = parent.x + parent.width / 2;
            var centerY = parent.y + parent.height / 2;

            var stat = parent.findStatsDrop(centerX, centerY);

            if (stat) {
                console.log(stat);
            }

            // Always bounce back. We only exchange the actual values.
            parent.x = relateTo.x + (relateTo.width - width) / 2;
            parent.y = relateTo.y + (relateTo.height - height) / 2;
        }
    }
}
