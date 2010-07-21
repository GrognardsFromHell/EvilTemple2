import Qt 4.7

/*
    Models the utility bar, which is visible on the lower right side in the original game.
*/
Item {
    width: 181
    height: 83

    x: gameView.viewportSize.width - width - 20
    y: 20

    property bool townmapDisabled : false
    property bool journalDisabled : false
    property string restingStatus : 'pass_time_only'
    property variant currentTime : ({
        year: 0,
        month: 0,
        day: 0,
        hour: 0,
        minute: 0,
        second: 0
    });

    signal openOptions
    signal openHelp
    signal openFormations
    signal openLogbook
    signal openTownmap
    signal openResting
    signal selectAll

    /**
        Causes the townmap button to flash a little to indicate a new area has become
        available.
    */
    function flashTownmap() {
        if (!townmapBlingAnimation.running) {
            gameView.playUiSound('sound/utility_bar_Bling-Sound.wav')
            townmapBlingAnimation.running = true;
        }
    }

    function flashJournal() {
        if (!logbookBlingAnimation.running) {
            gameView.playUiSound('sound/utility_bar_Bling-Sound.wav');
            logbookBlingAnimation.running = true;
        }
    }

    onCurrentTimeChanged: {
        if (!currentTime)
            return;

        // We use minute accuracy
        var center = (currentTime.hour * 60 + currentTime.minute) / (24 * 60); // Where should it be centered

        // We have to shift by 64 pixels, since time @ pixel 0 is 6 am
        var left = timeBarContainer.width / 2 + 64 - center * 256;


        if (left >= 0) {
            left -= 256;
        }

        timeBar.x = left;
    }

    Image {
        width: 181
        height: 83
        source: "../art/interface/utility_bar_ui/background.png"
        Button {
            id: selectAllButton
            x: 9
            y: 5
            width: 43
            height: 67
            text: ''
            pressedImage: "art/interface/utility_bar_ui/selectparty_click.png"
            hoverImage: "art/interface/utility_bar_ui/selectparty_hover.png"
            normalImage: "art/interface/utility_bar_ui/selectparty.png"
            disabledImage: "art/interface/utility_bar_ui/selectparty.png"
            onClicked: selectAll()
        }

        Button {
            id: logbookButton
            x: 71
            y: 5
            width: 21
            height: 43
            text: ''
            enabled: !journalDisabled
            pressedImage: "art/interface/utility_bar_ui/logbook_click.png"
            hoverImage: "art/interface/utility_bar_ui/logbook_hover.png"
            normalImage: "art/interface/utility_bar_ui/logbook.png"
            disabledImage: "art/interface/utility_bar_ui/logbook_disabled.png"
            onClicked: {
                openLogbook();
                logbookBlingAnimation.running = false;
            }

            Image {
                id: logbookBlingImage
                anchors.fill: parent
                opacity: 0
                source: "../art/interface/utility_bar_ui/logbook_bling.png"

                SequentialAnimation {
                    id: logbookBlingAnimation
                    loops: 5

                    PropertyAnimation {
                        target: logbookBlingImage
                        property: "opacity"
                        to: 1
                        duration: 750
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        target: logbookBlingImage
                        property: "opacity"
                        to: 0
                        duration: 750
                        easing.type: Easing.InOutQuad
                    }
                    PauseAnimation { duration: 750 }
                }
            }
        }

        Button {
            id: optionsButton
            x: 151
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/options_click.png"
            hoverImage: "art/interface/utility_bar_ui/options_hover.png"
            normalImage: "art/interface/utility_bar_ui/options.png"
            disabledImage: "art/interface/utility_bar_ui/options.png"
            onClicked: openOptions()
        }

        Button {
            id: helpButton
            x: 131
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/help_click.png"
            hoverImage: "art/interface/utility_bar_ui/help_hover.png"
            normalImage: "art/interface/utility_bar_ui/help.png"
            disabledImage: "art/interface/utility_bar_ui/help.png"
            onClicked: openHelp()
        }

        Button {
            id: formationButton
            x: 51
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/formation_click.png"
            hoverImage: "art/interface/utility_bar_ui/formation_hover.png"
            normalImage: "art/interface/utility_bar_ui/formation.png"
            disabledImage: "art/interface/utility_bar_ui/formation.png"
            onClicked: openFormations()
        }

        Button {
            id: passTimeButton
            x: 111
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/camp_click.png"
            hoverImage: "art/interface/utility_bar_ui/camp_hover.png"
            normalImage: "art/interface/utility_bar_ui/camp.png"
            disabledImage: "art/interface/utility_bar_ui/camp_clock_grey.png"
            visible: restingStatus == 'pass_time_only'
        }

        Button {
            id: campSafeButton
            x: 111
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/camp_green_click.png"
            hoverImage: "art/interface/utility_bar_ui/camp_green_hover.png"
            normalImage: "art/interface/utility_bar_ui/camp_green.png"
            disabledImage: "art/interface/utility_bar_ui/camp_grey.png"
            visible: restingStatus == 'safe'
        }

        Button {
            id: campDangerousButton
            x: 111
            y: 5
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/camp_yellow_click.png"
            hoverImage: "art/interface/utility_bar_ui/camp_yellow_hover.png"
            normalImage: "art/interface/utility_bar_ui/camp_yellow.png"
            disabledImage: "art/interface/utility_bar_ui/camp_grey.png"
            visible: restingStatus == 'dangerous'
        }

        Image {
            id: campImpossibleImage
            x: 111
            y: 5
            width: 21
            height: 43
            source: "../art/interface/utility_bar_ui/camp_red.png"
            visible: restingStatus == 'impossible'
            MouseArea {
                id: campImpossibleMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Button {
            id: townmapButton
            x: 91
            y: 5
            enabled: !townmapDisabled
            width: 21
            height: 43
            text: ''
            pressedImage: "art/interface/utility_bar_ui/townmap_click.png"
            hoverImage: "art/interface/utility_bar_ui/townmap_hover.png"
            normalImage: "art/interface/utility_bar_ui/townmap.png"
            disabledImage: "art/interface/utility_bar_ui/townmap_disabled.png"
            onClicked: {
                openTownmap();
                townmapBlingAnimation.running = false;
            }

            Image {
                id: townmapBlingImage
                anchors.fill: parent
                opacity: 0
                source: "../art/interface/utility_bar_ui/townmap_bling.png"

                SequentialAnimation {
                    id: townmapBlingAnimation
                    loops: 5

                    PropertyAnimation {
                        target: townmapBlingImage
                        property: "opacity"
                        to: 1
                        duration: 750
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        target: townmapBlingImage
                        property: "opacity"
                        to: 0
                        duration: 750
                        easing.type: Easing.InOutQuad
                    }
                    PauseAnimation { duration: 750 }
                }
            }
        }

        Item {
            id: timeBarContainer
            x: 53
            y: 49
            width: 117
            height: 21
            clip: true
            z: 1
            Image {
                z: 1
                id: timeBar
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 512
                anchors.bottomMargin: 0
                anchors.topMargin: 0
                fillMode: "TileHorizontally"
                x: 0
                y: 0
                source: "../art/interface/utility_bar_ui/timebar.png"
            }
            MouseArea {
                id: timeBarMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Tooltip {
            shown: timeBarMouseArea.containsMouse
            anchors.top: timeBarContainer.bottom
            anchors.horizontalCenter: timeBarContainer.horizontalCenter
            text: getGameDateTooltip(currentTime)
        }

        Tooltip {
            text: 'Main Menu'
            shown: optionsButton.containsMouse
            anchors.top: optionsButton.bottom
            anchors.horizontalCenter: optionsButton.horizontalCenter
        }

        Tooltip {
            text: 'Townmap'
            shown: townmapButton.containsMouse
            anchors.top: townmapButton.bottom
            anchors.horizontalCenter: townmapButton.horizontalCenter
            z: 10000
        }

        Tooltip {
            text: 'You Cannot Rest Here'
            shown: campImpossibleMouseArea.containsMouse
            anchors.top: campImpossibleMouseArea.bottom
            anchors.horizontalCenter: campImpossibleMouseArea.horizontalCenter
        }

        Tooltip {
            text: 'Rest Here (Safe)'
            shown: campSafeButton.containsMouse
            anchors.top: campSafeButton.bottom
            anchors.horizontalCenter: campSafeButton.horizontalCenter
        }

        Tooltip {
            text: 'Pass Time'
            shown: passTimeButton.containsMouse
            anchors.top: passTimeButton.bottom
            anchors.horizontalCenter: passTimeButton.horizontalCenter
        }

        Tooltip {
            text: 'Select All'
            shown: selectAllButton.containsMouse
            anchors.top: selectAllButton.bottom
            anchors.horizontalCenter: selectAllButton.horizontalCenter
        }

        Tooltip {
            text: 'Rest Here (Dangerous)'
            shown: campDangerousButton.containsMouse
            anchors.top: campDangerousButton.bottom
            anchors.horizontalCenter: campDangerousButton.horizontalCenter
        }

        Tooltip {
            text: 'Formation'
            shown: formationButton.containsMouse
            anchors.top: formationButton.bottom
            anchors.horizontalCenter: formationButton.horizontalCenter
        }

        Tooltip {
            text: 'Help'
            shown: helpButton.containsMouse
            anchors.top: helpButton.bottom
            anchors.horizontalCenter: helpButton.horizontalCenter
        }

        Tooltip {
            text: 'Journal'
            shown: logbookButton.containsMouse
            anchors.top: logbookButton.bottom
            anchors.horizontalCenter: logbookButton.horizontalCenter
        }

        Image {
            id: timeBarArrow
            x: 105
            y: 60
            z: 2
            width: 13
            height: 12
            source: "../art/interface/utility_bar_ui/timebar_arrow.png"
        }
    }

    /* DEBUGGING
    SequentialAnimation {
        running: true
        loops: 24

        ScriptAction {
            script: {
                currentTime = {
                    hour: currentTime.hour + 1,
                    minute: 0
                }
            }
        }
        PauseAnimation {
            duration: 100
        }
    }*/

    function getGameDateTooltip(currentTime) {
        // Months in common, as per Wikipedia
        var months = [
            'Fireseek',
            'Readying',
            'Coldeven',
            'Planting',
            'Flocktime',
            'Wealsun',
            'Reaping',
            'Goodmonth',
            'Harvester',
            'Patchwall',
            'Ready\'reat',
            'Sunsebb'
            ];

        var time = currentTime.hour + ":";
        if (currentTime.minute < 10)
            time += '0';
        time += currentTime.minute;

        var daySuffix = '';
        switch (currentTime.day) {
        case 1:
            daySuffix = 'st';
            break;
        case 2:
            daySuffix = 'nd';
            break;
        case 3:
            daySuffix = 'rd';
            break;
        default:
            daySuffix = 'th';
            break;
        }

        return currentTime.year + " CY " + months[currentTime.month-1] + " " + currentTime.day + daySuffix
                    + " " + time;
    }
}
