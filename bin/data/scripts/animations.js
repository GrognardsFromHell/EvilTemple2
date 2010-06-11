
function openAnimations(modelInstance) {
    var dialog = gameView.addGuiItem("interface/Animations.qml");
    dialog.setAnimations(modelInstance.model.animations);
    dialog.playAnimation.connect(function (name) {
        modelInstance.playAnimation(name, false);
    });
    dialog.closeClicked.connect(function() {
        dialog.deleteLater();
    });
}
