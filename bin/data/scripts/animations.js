
function openAnimations(modelInstance) {
    var dialog = gameView.addGuiItem("interface/Animations.qml");
    var animationNames = modelInstance.model.animations;
    animationNames.sort();

    var animations = [];
    animationNames.forEach(function (name) {
        animations.push({
            name: name,
            frames: modelInstance.model.animationFrames(name)
        });
    });

    dialog.setAnimations(animations);
    dialog.playAnimation.connect(function (name) {
        print("Playing animation " + name);
        modelInstance.playAnimation(name, false);
    });
    dialog.closeClicked.connect(function() {
        dialog.deleteLater();
    });
}
