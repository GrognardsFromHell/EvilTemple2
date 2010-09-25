
var currentPath;
var nextPathElement;
var curX;
var curY;
var lastX;
var lastY;

function startTravel(path) {
    curX = path.from[0];
    curY = path.from[1];

    lastX = 0;
    lastY = 0;

    currentPath = path;
    nextPathElement = 0;

    travelTimer.start();
}

function doTravelStep() {
    var pathStepComponent = Qt.createComponent('WorldMapPathStep.qml');
    var trailIcon;

    // Modify curX/curY based on opcode
    var opcode = currentPath.path[nextPathElement++];

    switch (opcode) {
    case 'up':
        curY--;
        break;
    case 'down':
        curY++;
        break;
    case 'left':
        curX--;
        break;
    case 'right':
        curX++;
        break;
    case 'upleft':
        curY--;
        curX--;
        break;
    case 'upright':
        curY--;
        curX++;
        break;
    case 'downleft':
        curY++;
        curX--;
        break;
    case 'downright':
        curY++;
        curX++;
        break;
    case 'stay':
        break; // Stay where we are (mostly useful for the first step)
    default:
        console.log('Unknown path opcode: ' + opcode);
    }

    if (Utilities.distance(lastX, lastY, curX, curY) >= 8) {
        trailIcon = pathStepComponent.createObject(root);
        trailIcon.x = curX;
        trailIcon.y = curY;
        lastX = curX;
        lastY = curY;

        gameView.playUiSound('sound/worldmap_walking_sound.wav');
    }

    if (nextPathElement >= currentPath.path.length) {
        travelTimer.stop();
        travelFinished();
    }
}

