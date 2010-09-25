var ModalDialogUi = {};

ModalDialogUi.show = function(title, message, okayCallback, cancelCallback) {

    // TODO: This should probably become "addModalDialog" or similar to have an *actual* modal dialog!
    var dialog = gameView.addGuiItem('interface/ModalDialog.qml');

    dialog.title = title;
    dialog.message = message;
    dialog.okay.connect(function() {
        dialog.deleteLater();
        okayCallback();
    });
    dialog.cancel.connect(function() {
        dialog.deleteLater();
        cancelCallback();
    });
    
};
