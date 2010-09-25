
var initialized = false;
var worldmapButtonComponent;
var imageComponent;

var currentComponents = [];
var currentAreas = [];

function loadComponents()
{
    if (initialized)
        return;

    worldmapButtonComponent = Qt.createComponent('WorldMapButton.qml');

    initialized = true;
}

function addArea(area) {

    knownAreaModel.append(area);

    // Create the actual worldmap button
    var wmb = worldmapButtonComponent.createObject(root);
    wmb.x = area.center[0];
    wmb.y = area.center[1];
    wmb.radius = area.radius;
    wmb.name = area.name;
    wmb.buttonId = area.buttonId;
    currentComponents.push(wmb);

    // Create all associated images
    var i;
    for (i = 0; i < area.images.length; ++i) {
        var image = area.images[i];

        var imgObj = Qt.createQmlObject("import Qt 4.7; Image {}", root, '/imagebtn.qml');
        imgObj.x = wmb.x + image[0];
        imgObj.y = wmb.y + image[1];
        imgObj.source = image[2];
        currentComponents.push(imgObj);
    }
}

function setAreas(areas) {
    // Clear existing components
    currentComponents.forEach(function (comp) {
        comp.deleteLater();
    });
    knownAreaModel.clear();
    currentComponents = [];
    currentAreas = areas;

    loadComponents();
    areas.forEach(addArea);
}

function onClick(mouseX, mouseY) {
    var centerX, centerY, radius;
    var worldmapButton;

    for (var i = 0; i < children.length; ++i) {
        var child = root.children[i];

        radius = child.radius;

        if (!radius || !child.visible)
            continue;

        centerX = (child.x + child.width / 2);
        centerY = (child.y + child.height / 2);

        var diffX = mouseX - centerX;
        var diffY = mouseY - centerY;
        var distance = Math.sqrt(diffX * diffX + diffY * diffY);

        if (distance <= radius) {
            worldmapButton = child;
            break;
        }
    }

    if (worldmapButton) {
        console.debug("Requesting travel to " + worldmapButton.buttonId);
        travelRequested(worldmapButton.buttonId);
    }
}
