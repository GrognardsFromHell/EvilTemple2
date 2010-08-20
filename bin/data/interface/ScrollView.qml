import Qt 4.7

Item {
    width: 320
    height: 240
    clip: true

    property alias model : listView.model

    property alias delegate : listView.delegate

    property bool dontUpdate : false

    property alias spacing : listView.spacing

    function refreshScrollbar() {
        if (dontUpdate)
            return;
        var scrollBarSpan = scrollBar.height - scrollBarHandle.height;
        var y = listView.contentY / (listView.contentHeight - listView.height);
        dontUpdate = true;
        scrollBarHandle.y = y * scrollBarSpan;
        dontUpdate = false;
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.rightMargin: scrollBar.width
        boundsBehavior: "StopAtBounds"

        onContentYChanged: refreshScrollbar();
        onContentHeightChanged: refreshScrollbar();
    }

    Rectangle {
        id: scrollBar
        width: 24
        color: '#7f000000'
        radius: 5
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        visible: listView.contentHeight > listView.height

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var y = mouseY - scrollBarHandle.height / 2
                if (y < 0)
                    y = 0;
                else if (y > height - scrollBarHandle.height)
                    y = height - scrollBarHandle.height;
                scrollBarHandle.y = y;
            }
        }

        Rectangle {
            id: scrollBarHandle
            width: parent.width
            height: 24
            color: '#7f000000'
            radius: 5
            border.width: 1
            border.color: '#7fffffff'

            onYChanged: {
                if (dontUpdate)
                    return;

                var y = scrollBarHandle.y / (scrollBar.height - scrollBarHandle.height);
                var listSpan = listView.contentHeight - listView.height;
                y = Math.floor(y * listSpan);
                dontUpdate = true;
                listView.contentY = y;
                dontUpdate = false;
            }

            MouseArea {
                anchors.fill: parent
                drag.axis: Drag.YAxis
                drag.target: parent
                drag.minimumY: 0
                drag.maximumY: scrollBar.height - parent.height
            }
        }
    }
}
