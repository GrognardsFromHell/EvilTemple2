
function getTimePassed(date) {
    var now = new Date(); // Get current date/time
    print(date);
    var elapsed = now.getTime() - date.getTime();

    if (elapsed < 0) {
        return "in the future";
    }

    elapsed = Math.floor(elapsed / 1000); // Convert to seconds

    var secondsElapsed = elapsed;
    var minutesElapsed = secondsElapsed / 60;
    var hoursElapsed = minutesElapsed / 60;
    var daysElapsed = hoursElapsed / 24;

    // Too long ago to care about the smaller units
    if (daysElapsed >= 2) {
        hoursElapsed = 0;
        minutesElapsed = 0;
        secondsElapsed = 0;
    } else if (daysElapsed >= 1) {
        minutesElapsed = 0;
        secondsElapsed = 0;
    }

    if (hoursElapsed < 2)
        hoursElapsed = 0;
    else
        secondsElapsed = 0;

    if (minutesElapsed > 15)
        secondsElapsed = 0;

    hoursElapsed %= 24;
    minutesElapsed %= 60;
    secondsElapsed %= 60;

    var result = '';
    if (daysElapsed >= 2)
        result += Math.floor(daysElapsed) + ' days ';
    else if (daysElapsed >= 1)
        result += Math.floor(daysElapsed) + ' day ';
    if (hoursElapsed >= 2)
        result += Math.floor(hoursElapsed) + ' hours ';
    else if (hoursElapsed >= 1)
        result += Math.floor(hoursElapsed) + ' hour ';
    if (minutesElapsed >= 1)
        result += Math.floor(minutesElapsed) + ' min ';
    if (secondsElapsed >= 1)
        result += Math.floor(secondsElapsed) + ' sec ';
    result += 'ago';

    return result;
}

function convertDateTime(date) {

    return Qt.formatDateTime(date, Qt.DefaultLocaleShortDate );

}

function getComfortableTime(date) {
    // date = new Date(2010, 05, 1, 12, 33, 32);
    // date = new Date(2010, 06, 19, 12, 33, 32);

    return convertDateTime(date) + " (" + getTimePassed(date) + ")";
}
