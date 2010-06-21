
function openAnimations(modelInstance) {
    var dialog = gameView.addGuiItem("interface/Animations.qml");
    var animations = modelInstance.model.animations;
    animations.sort();
    dialog.setAnimations(animations);
    dialog.playAnimation.connect(function (name) {
        print("Playing animation " + name);
        modelInstance.playAnimation(name, false);
    });
    dialog.closeClicked.connect(function() {
        dialog.deleteLater();
    });
}
