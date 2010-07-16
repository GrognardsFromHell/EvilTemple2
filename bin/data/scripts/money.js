
/**
 * A class that represents currency.
 */
var Money = function(value) {
    if (!(this instanceof Money)) {
        throw "Please use the Money constructor only with the new keyword.";
    }

    // All currency is stored in copper-form
    if (value)
        this._value = value; 
    else
        this._value = 0;
};

(function() {

    /*
        Conversion rates.
     */
    var SilverInCopper = 10;
    var GoldInSilver = 10;
    var GoldInCopper = SilverInCopper * GoldInSilver;

    /**
     * Returns the number of copper coins in this money representation.
     */
    Money.prototype.getCopper = function() {
        return this._value % SilverInCopper;
    };

    /**
     * Returns the total number of copper coins in this money representation.
     * This does include all coins that would otherwise be upgraded to silver or gold coins.
     */
    Money.prototype.getTotalCopper = function() {
        return this._value;
    };

    /**
     * Returns the number of silver coins in this money representation.
     * This does not include silver coins that have been "upgraded" to gold coins
     * automatically.
     */
    Money.prototype.getSilver = function() {
        var silver = Math.floor(this._value / SilverInCopper);
        return silver % GoldInSilver;
    };

    /**
     * Returns the total potential number of silver coins in this money representation.
     * This does include coins that are actually represented as gold.
     */
    Money.prototype.getTotalSilver = function() {
        return Math.floor(this._value / SilverInCopper);
    };

    /**
     * Returns the number of gold coins in this money representation.
     */
    Money.prototype.getGold = function() {
        return Math.floor(this._value / GoldInCopper);
    };

    /**
     * Adds gold coins to this money object. If the amount is negative, gold coins are
     * removed from this money object, but the total amount of copper can never go below zero.
     */
    Money.prototype.addGold = function(amount) {
        this._value += amount * GoldInCopper;
        if (this._value < 0)
            this._value = 0;
    };

    /**
     * Adds silver coins to this money object. If the amount is negative, silver coins are
     * removed from this money object, but the total amount of copper can never go below zero.
     */
    Money.prototype.addSilver = function(amount) {
        this._value += amount * SilverInCopper;
        if (this._value < 0)
            this._value = 0;
    };

    /**
     * Adds copper coins to this money object. If the amount is negative, copper coins are
     * removed from this money object, but the total amount can never go below zero.
     */
    Money.prototype.addCopper = function(amount) {
        this._value += amount;
        if (this._value < 0)
            this._value = 0;
    };

})();
