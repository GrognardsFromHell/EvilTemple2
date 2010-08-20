/*
 Registry for Domains.
 */
var Domains = {};

(function() {

    var list = [];
    var byId = {};

    Domains.register = function(domain) {
        byId[domain.id] = domain;
        list.push(domain);
    };

    Domains.getAll = function() {
        return list.slice(0); // Return a shallow-copy
    };

    Domains.getById = function(id) {
        return byId[id];
    };

})();
