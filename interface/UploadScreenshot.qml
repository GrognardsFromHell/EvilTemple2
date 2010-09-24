import Qt 4.7

MovableWindow {

    id: root

    property alias screenshot : previewImage.source;

    function uploadScreenshot() {
        state = 'Uploading';
        progressBar.progress = 0;
        imageUploader.upload(screenshot);
        imageUploader.uploadProgress.connect(function (p, t) {
            var progress = p / t;
            progressBar.progress = progress;
        });
        imageUploader.uploadFinished.connect(function (result) {
            console.log("Finished uploading: " + result);
            root.state = 'Upload Finished';
            // TODO: Disconnect from imageUploader
            resultText.text = '<p>Your screenshot has been uploaded to imgur.com.</p>' +
                              'You can find more information on the <a style="color: red" ' +
                              'href="' + result.rsp.image.imgur_page + '?tags">' +
                              'image page</a>.';
        });
        imageUploader.uploadFailed.connect(function () {
            console.log("Uploading screenshot failed.");
        });
    }

    onCloseClicked: deleteLater();

    width: 640
    height: 480
    states: [
        State {
            name: "Uploading"

            PropertyChanges {
                target: image1
                visible: false
            }

            PropertyChanges {
                target: image2
                visible: false
            }

            PropertyChanges {
                target: previewImage
                visible: false
            }

            PropertyChanges {
                target: root
                height: 180
            }

            PropertyChanges {
                target: uploadProgressText
                opacity: 1
            }

            PropertyChanges {
                target: progressBar
                opacity: 1
            }
        },
        State {
            name: "Upload Finished"
            PropertyChanges {
                target: image1
                visible: false
            }

            PropertyChanges {
                target: image2
                visible: false
            }

            PropertyChanges {
                target: previewImage
                visible: false
            }

            PropertyChanges {
                target: root
                height: 180
            }

            PropertyChanges {
                target: uploadProgressText
                opacity: 0
            }

            PropertyChanges {
                target: progressBar
                opacity: 0
            }

            PropertyChanges {
                target: resultText
                x: 9
                y: 40
                width: 618
                height: 127
                opacity: 1
            }
        }
    ]
    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { targets: [uploadProgressText, progressBar]; properties: "opacity"; duration: 200 }
            NumberAnimation { targets: [root]; properties: "height"; duration: 200 }
        }

    ]
    title: "Upload Screenshot"
    data: [
        Image {
            id: previewImage
            x: 9
            y: 40
            width: 618
            height: 394
            smooth: false
            fillMode: "PreserveAspectFit"
            source: "qrc:/fxplugin/images/template_image.png"
        },
        Image {
            id: image2
            x: 9
            y: 442
            source: 'ButtonBackdrop.png'
            width: uploadButton.width + 8
            height: uploadButton.height + 8
            Button {
                x: 4
                y: 4
                id: uploadButton
                text: 'Upload'
                onClicked: uploadScreenshot()
            }
        },
        Image {
            id: image1
            x: 131
            y: 442
            source: 'ButtonBackdrop.png'
            width: deleteButton.width + 8
            height: deleteButton.height + 8
            Button {
                x: 4
                y: 4
                id: deleteButton
                text: 'Delete'
                onClicked: {
                    gameView.deleteScreenshot(screenshot);
                    root.deleteLater();
                }
            }
        },
        Text {
            id: uploadProgressText
            x: 9
            y: 60
            width: 618
            height: 20
            color: "#ffffff"
            text: "Uploading screenshot to imgur.com"
            font.family: "Fontin"
            font.pointSize: 12
            opacity: 0
        },
        ProgressBar {
            id: progressBar
            x: 9
            y: 101
            opacity: 0
        },
        Text {
            id: resultText
            opacity: 0
            x: 9
            y: 40
            width: 618
            height: 400
            color: "#ffffff"
            text: "Uploading screenshot to imgur.com"
            font.family: "Fontin"
            font.pointSize: 12
            onLinkActivated: gameView.openBrowser(link)
        }

    ]

}
