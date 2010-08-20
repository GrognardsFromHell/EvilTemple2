
var selectedDomains = [];
var checkboxes = [];

function selectDomain(id) {
    selectedDomains.push(id);
    updateCheckboxes();
    domainsSelected(selectedDomains);
}

function deselectDomain(id) {
    selectedDomains.splice(selectedDomains.indexOf(id), 1);
    updateCheckboxes();
    domainsSelected(selectedDomains);
}

function canSelectMore() {
    return selectedDomains.length < numberOfSelectableDomains;
}

function registerCheckbox(checkbox) {
    checkboxes.push(checkbox);
}

function updateCheckboxes() {
    maySelectMore = canSelectMore();
}
