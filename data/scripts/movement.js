(function() {

    function worldClicked(event, worldPosition) {
        if (Combat.isActive())
            return;

        if (event.button != Mouse.LeftButton)
            return;

        Selection.get().forEach(function (critter) {
            var path = new Path(critter, worldPosition);

            if (!path.isEmpty()) {
                var movementGoal = new MovementGoal(path, false);
                var sayHelloGoal = {
                    advance: function() {
                        gameView.scene.addTextOverlay(worldPosition, "I am there!", [0.9, 0, 0, 0.9]);
                        this.finished = true;
                    },
                    isFinished: function() {
                        return this.finished;
                    },
                    cancel: function() {
                    }
                };

                critter.setGoal(new GoalSequence(movementGoal, sayHelloGoal));
            }
        });
    }

    Maps.addMouseClickListener(worldClicked);

})();
