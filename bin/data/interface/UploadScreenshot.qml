import Qt 4.7

MovableWindow {

    property alias screenshot : previewImage.source;

    function uploadScreenshot() {
        imageUploader.upload(screenshot);
    }

    width: 640
    height: 480
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
            }
        }
    ]

}
