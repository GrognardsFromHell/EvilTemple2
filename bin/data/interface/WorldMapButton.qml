import Qt 4.7

Item {

    /**
        The radius in pixels to which this worldmap button reacts.
    */
    property int radius

    /**
        The name of this zone. It is used for the tooltip
    */
    property string name : 'Zone Name'

    /**
        The id of this button that is passed back to the controller, when
        this button is clicked.
    */
    property string buttonId : 'button-id'
}
