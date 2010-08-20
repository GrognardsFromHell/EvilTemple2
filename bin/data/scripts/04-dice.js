var Dice;

(function() {

    var DicePattern = /^(\d*)d([1-9]\d*)([+\-]\d+)?$/i;

    /**
     * Constructs a dice object that describes a dice roll template commonly used throughout the ruleset.
     * Examples: 1d6, 1d6+5 5d10-1 and so on.
     * @param dieString The die format string of the following form:
     * An optional, positive number of dice followed by the character 'd', followed by the positive number of
     * sides (at least 1), followed by an optional, positive or negative bonus to the dice roll.
     * Expressed as a regular expression: (\d*)d([1-9]\d*)([+\-]\d+)?
     *
     * The returned object will have three properties:
     * <ul>
     * <li>dice - The number of dice being rolled.</li>
     * <li>sides - The number of sides each dice being rolled has. Minimum of 1.</li>
     * <li>bonus - The bonus added to the dice throw.</li>
     * </ul>
     */
    Dice = function(dieString) {

        var matches = DicePattern.exec(dieString);

        if (!matches) {
            throw "Invalid dice pattern: " + pattern;
        }

        this.dice = matches[1] ? parseInt(matches[1]) : 1;
        this.sides = parseInt(matches[2]);
        this.bonus = matches[3] ? parseInt(matches[3]) : 0;
    };

    /**
     * Returns the absolute minimum value that a roll of this dice object will return. It is assumed that
     * every dice roll will be 1.
     * @returns Number of dice + bonus
     */
    Dice.prototype.getMinimum = function() {
        return this.dice + this.bonus;
    };

    /**
     * Returns the maximum value that a roll of this dice object will return.
     * @returns Number of dice * sides + bonus
     */
    Dice.prototype.getMaximum = function() {
        return this.dice * this.sides + this.bonus;
    };

    /**
     * Performs this dice roll and returns the result.
     */
    Dice.prototype.roll = function() {
        var result = 0;

        for (var i = 0; i < this.dice; ++i) {
            result += randomRange(1, this.sides);
        }

        return result + this.bonus;
    };

})();

new Dice('1d6');
