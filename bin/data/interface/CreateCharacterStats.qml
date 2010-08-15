import Qt 4.7

import 'CreateCharacterStats.js' as CreateCharacterStats
import 'Utilities.js' as Utilities

Item {
    id: root
    width: 431
    height: 233

    property bool complete : false
    property int strength : 0
    property int dexterity : 0
    property int constitution : 0
    property int intelligence : 0
    property int wisdom : 0
    property int charisma : 0
    property int rolls : 0

    Component.onCompleted: CreateCharacterStats.rollStats(false)

    Text {
        x: 2
        y: 2
        width: 427
        height: 17
        color: "#ffffff"
        font.pointSize: 12
        font.family: "Fontin"
        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
        text: 'Roll Your Character\'s Statistics'
    }

    Column {
        id: characterStats
        x: 39
        y: 43
        spacing: 3
        Repeater {
            model: ['str', 'dex', 'con', 'int', 'wis', 'cha']
            Item {
                id: statRoot
                property alias statName : name.text

                property bool highlight : false

                property alias statValue : statValueText.text

                width: childrenRect.width
                height: childrenRect.height

                Component.onCompleted: CreateCharacterStats.registerCharStatItem(modelData, statRoot)

                Rectangle {
                    x: 0
                    y: 6
                    color: '#41586f'
                    width: 38
                    height: 17
                    Text {
                        id: name
                        anchors.centerIn: parent
                        text: modelData.toUpperCase() // TODO: Translate properly
                        color: '#FFFFFF'
                        font.bold: true
                    }
                }

                Rectangle {
                    id: box
                    x: 47
                    y: 0
                    border.width: 1
                    border.color: highlight ? '#00a9f8' : '#41586f'
                    width: 39
                    height: 28
                    color: '#00000000'
                    Text {
                        id: statValueText
                        color: '#FFFFFF'
                        font.bold: true
                        font.pointSize: 17
                        smooth: true
                        visible: text != '0'
                        width: box.width
                        height: box.height
                        verticalAlignment: "AlignVCenter"
                        horizontalAlignment: "AlignHCenter"

                        MouseArea {
                            anchors.fill: parent
                            drag.target: parent
                            drag.axis: Drag.XandYAxis
                            onPressed: {
                                console.log("Pressed " + modelData)
                                // Update the drag bounds (dirty hack, but i see no other way)
                                drag.minimumX = root.mapToItem(box, 0, 0).x;
                                drag.minimumY = root.mapToItem(box, 0, 0).y;
                                drag.maximumX = drag.minimumX + root.width - parent.width;
                                drag.maximumY = drag.minimumY + root.height - parent.height;
                            }
                            onMousePositionChanged: {
                                var centerX = parent.x + parent.width / 2;
                                var centerY = parent.y + parent.height / 2;
                                var posInRoot = root.mapFromItem(box, centerX, centerY);

                                CreateCharacterStats.highlightStatsDrop(posInRoot.x, posInRoot.y)
                            }
                            onReleased: {
                                CreateCharacterStats.disableStatsDropHighlight();
                                console.log("Released " + modelData)

                                var centerX = parent.x + parent.width / 2;
                                var centerY = parent.y + parent.height / 2;
                                var posInRoot = root.mapFromItem(box, centerX, centerY);

                                var stat = CreateCharacterStats.findStatsDrop(posInRoot.x, posInRoot.y);

                                if (stat) {
                                    CreateCharacterStats.swapStats(modelData, stat);
                                }

                                // Always bounce back. We only exchange the actual values.
                                parent.x = 0;
                                parent.y = 0;
                            }
                        }
                    }
                }

                Rectangle {
                    x: 95
                    y: 6
                    border.width: 1
                    border.color: '#41586f'
                    width: 27
                    height: 15
                    color: '#00000000'
                    Text {
                        color: '#FFFFFF'
                        font.pointSize: 12
                        smooth: true
                        visible: statValueText.visible
                        anchors.centerIn: parent
                        text: Utilities.getAbilityModifier(statValueText.text)
                    }
                }
            }
        }
    }

    Column {
        id: rolledStats
        x: 202
        y: 43
        spacing: 3
        Repeater {
            model: ['roll1', 'roll2', 'roll3', 'roll4', 'roll5', 'roll6']
            Rectangle {
                id: box
                width: 39
                height: 28
                border.color: highlight ? '#00a9f8' : '#FFFF00'
                border.width: 1
                color: '#00000000'

                property alias statValue : statLabel.text

                property bool highlight : false

                Component.onCompleted: CreateCharacterStats.registerCharStatItem(modelData, box)

                Text {
                    id: statLabel
                    text: '18'
                    color: '#FFFFFF'
                    font.bold: true
                    font.pointSize: 17
                    smooth: true
                    z: 100
                    width: box.width
                    height: box.height
                    x: 0
                    y: 0
                    visible: statLabel.text != '0'
                    verticalAlignment: "AlignVCenter"
                    horizontalAlignment: "AlignHCenter"

                    MouseArea {
                        anchors.fill: parent
                        drag.target: parent
                        drag.axis: Drag.XandYAxis
                        onPressed: {
                            console.log("Pressed " + modelData)
                            // Update the drag bounds (dirty hack, but i see no other way)
                            drag.minimumX = root.mapToItem(box, 0, 0).x;
                            drag.minimumY = root.mapToItem(box, 0, 0).y;
                            drag.maximumX = drag.minimumX + root.width - parent.width;
                            drag.maximumY = drag.minimumY + root.height - parent.height;
                        }
                        onMousePositionChanged: {
                            var centerX = parent.x + parent.width / 2;
                            var centerY = parent.y + parent.height / 2;
                            var posInRoot = root.mapFromItem(box, centerX, centerY);

                            CreateCharacterStats.highlightStatsDrop(posInRoot.x, posInRoot.y)
                        }
                        onReleased: {
                            CreateCharacterStats.disableStatsDropHighlight();
                            console.log("Released")

                            var centerX = parent.x + parent.width / 2;
                            var centerY = parent.y + parent.height / 2;
                            var posInRoot = root.mapFromItem(box, centerX, centerY);

                            var stat = CreateCharacterStats.findStatsDrop(posInRoot.x, posInRoot.y);

                            if (stat) {
                                CreateCharacterStats.swapStats(modelData, stat);
                            }

                            // Always bounce back. We only exchange the actual values.
                            parent.x = 0;
                            parent.y = 0;
                        }
                    }
                }
            }
        }
    }

    Image {
        x: 279
        y: 135
        source: '../art/interface/pc_creation/rollbox.png'
        CreateCharacterButtonRight {
            x: 5
            y: 5
            text: 'ROLL'
            onClicked: CreateCharacterStats.rollStats(true);
        }
    }

    Text {
        id: rollsLabel
        x: 282
        y: 175
        width: 122
        height: 22
        color: '#FFFFFF'
        font.family: 'Fontin'
        font.pointSize: 12
        smooth: true
        text: 'Rerolls: ' + rolls
    }
}
