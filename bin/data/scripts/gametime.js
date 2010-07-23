/**
 * This object manages the game time, which is the time in the game world.
 * It is measured with a precision of seconds.
 *
 * The time is stored in seconds since the start of the current year and the year itself.
 */
var GameTime = {};

(function() {

    var secondOfTheYear = 0;

    var currentYear = 0;

    var timeChangedListeners = new ListenerQueue;
    var hourChangedListeners = new ListenerQueue;

    var SecondsPerMinute = 60;
    var MinutesPerHour = 60;
    var SecondsPerHour = MinutesPerHour * SecondsPerMinute;
    var HoursPerDay = 24;
    var SecondsPerDay = HoursPerDay * SecondsPerHour;
    var DaysPerMonth = 28;
    var SecondsPerMonth = DaysPerMonth * SecondsPerDay;
    var SecondsPerYear = 12 * SecondsPerMonth;

    function normalizeTime() {
        var wrappedYears = Math.floor(secondOfTheYear / SecondsPerYear);
        secondOfTheYear %= SecondsPerYear;

        if (wrappedYears > 0) {
            currentYear += wrappedYears;
            print("Time overflow. Adding " + wrappedYears + " years.");
        }
    }

    /**
     * Adds a listener that is notified whenever the game time changes.
     * @param callback
     * @param thisObj
     */
    GameTime.addTimeChangedListener = function(callback, thisObj) {
        timeChangedListeners.append(callback, thisObj);
    };

    /**
     * Adds a listener that is called when the our of the day changes.
     * @param callback The callback to register.
     * @param thisObj The this object for the callback.
     */
    GameTime.addHourChangedListener = function(callback, thisObj) {
        hourChangedListeners.append(callback, thisObj);
    };

    /**
     * Removes a previously registered time change listener.
     * @param callback
     * @param thisObj
     */
    GameTime.removeTimeChangedListener = function(callback, thisObj) {
        return timeChangedListeners.remove(callback, thisObj);
    };

    /**
     * Sets the game time to a specific point in time.
     *
     * @param year The year.
     * @param month The month of the year. (1-11)  (Default: 1)
     * @param day The day of the month. (1-28) (Default: 1)
     * @param hour The hour of the day. (0-23) (Default: 0)
     * @param minute The minute of the hour (0-59) (Default: 0)
     * @param second The second of the hour (0-59) (Default: 0)
     */
    GameTime.set = function(year, month, day, hour, minute, second) {
        var oldHour = this.getHourOfDay();

        if (second === undefined)
            second = 0;
        if (minute === undefined)
            minute = 0;
        if (hour === undefined)
            hour = 0;
        if (day === undefined)
            day = 1;
        if (month === undefined)
            month = 1;

        month -= 1;
        day -= 1;

        secondOfTheYear = month * SecondsPerMonth + day * SecondsPerDay
                + hour * SecondsPerHour + minute * SecondsPerMinute + second;

        currentYear = year;

        normalizeTime();

        timeChangedListeners.notify();

        if (oldHour != this.getHourOfDay()) {
            hourChangedListeners.notify(oldHour);
        }
    };

    /**
     * Gets an object with the current game date & time.
     *
     * @returns An object with the following properties:
     *  year - The year.
     *  month - The month of the year. (1-11)  (Default: 1)
     *  day - The day of the month. (1-28) (Default: 1)
     *  hour - The hour of the day. (0-23) (Default: 0)
     *  minute - The minute of the hour (0-59) (Default: 0)
     *  second - The second of the hour (0-59) (Default: 0)
     */
    GameTime.get = function(year, month, day, hour, minute, second) {
        var result = {};

        result.second = this.getSecondOfMinute();
        result.minute = this.getMinuteOfHour();
        result.hour = this.getHourOfDay();
        result.day = this.getDayOfMonth();
        result.month = this.getMonth();
        result.year = this.getYear();

        return result;
    };

    /**
     * Progress game time.
     *
     * @param seconds The number of seconds to progress game time by.
     */
    GameTime.addTime = function(seconds) {
        var oldHour = this.getHourOfDay();

        secondOfTheYear += seconds;
        normalizeTime();

        timeChangedListeners.notify();

        if (oldHour != this.getHourOfDay()) {
            hourChangedListeners.notify(oldHour);
        }
    };

    GameTime.getSecondOfMinute = function() {
        return secondOfTheYear % SecondsPerMinute;
    };

    GameTime.getMinuteOfHour = function() {
        return Math.floor(secondOfTheYear / SecondsPerMinute) % MinutesPerHour;
    };

    GameTime.getHourOfDay = function() {
        return Math.floor(secondOfTheYear / SecondsPerHour) % HoursPerDay;
    };

    GameTime.getDayOfMonth = function() {
        return 1 + Math.floor(secondOfTheYear / SecondsPerDay) % DaysPerMonth;
    };

    GameTime.getMonth = function() {
        return 1 + Math.floor(secondOfTheYear / SecondsPerMonth);
    };

    GameTime.getYear = function() {
        return currentYear;
    };

    GameTime.start = function() {
        // Register timer to increment gametime regularily
    };

    GameTime.stop = function() {
        // Stop game-time (i.e. in combat/game pause)
    };

    GameTime.isDaytime = function() {
        var hour = this.getHourOfDay();
        return hour >= 6 && hour < 18;
    };

    GameTime.isNighttime = function() {
        var hour = this.getHourOfDay();
        return hour < 6 || hour >= 18;
    };

    function load(payload) {
        if (!payload.gametime) {
            print("Gametime is not stored in savegame.");
            return;
        }

        secondOfTheYear = payload.gametime.seconds;
        currentYear = payload.gametime.year;

        if (secondOfTheYear === undefined || currentYear === undefined) {
            print("Error while loading gametime from savegame.");
        }
    }

    function save(payload) {
        payload.gametime = {
            seconds: secondOfTheYear,
            year: currentYear
        };
    }

    GameTime.set(579, 3, 14, 15);

    StartupListeners.add(function() {
        SaveGames.addLoadingListener(load);
        SaveGames.addSavingListener(save);

        var gameTimeTick = function() {
            GameTime.addTime(60); // One minute per second for now.
            gameView.addVisualTimer(100, gameTimeTick);
        };
        gameView.addVisualTimer(1000, gameTimeTick);


    });

})();
