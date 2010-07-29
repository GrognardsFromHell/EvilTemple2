var HelpUi = {};

(function() {

    var textData = {}; // This is loaded lazily from help.js
    var loaded = false; // Indicator whether lazy loading occurred
    var dialog = null;

    function load() {
        if (loaded)
            return;
        textData = readJson('help.js');
        loaded = true;
    }

    function makeLink(topic, title) {
        // TODO: HTML-escape title
        return '<a style="color: #00b7e8" href="' + topic + '">' + title + '</a>';
    }

    function getChildren(topic) {
        var children = '';

        for (var k in textData) {
            var entry = textData[k];
            if (entry.parent == topic || entry.unk && entry.unk.indexOf(topic) != -1) {
                children += '<li>' + makeLink(k, entry.title) + '</li>';
            }
        }

        if (children != '') {
            return '<ul>' + children + '</ul>';
        } else {
            return '';
        }
    }

    function getChildrenSorted(topic) {
        var children = [];

        for (var k in textData) {
            var entry = textData[k];
            if (entry.parent == topic || entry.unk && entry.unk.indexOf(topic) != -1) {
                children.push([k, '<li>' + makeLink(k, entry.title) + '</li>']);
            }
        }

        if (children.length == 0)
            return '';

        children.sort(function(a, b) {
            return a[0].localeCompare(b);
        });

        var text = '<ul>';
        children.forEach(function (child) {
            text += child[1];
        });
        text += '</ul>';
        return text;
    }

    function navigate(topic) {
        var record = textData[topic];

        var text;

        if (!record) {
            text = 'Sorry, unknown help topic: ' + topic;
        } else {
            text = record.text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
            text = text.replace(/\x0B/g, "<br>");
            text = '<h1>' + record.title + '</h1><p>' + text + '</p>';
        }

        // Process commands.
        var linkPattern = /~([^~]+)~\[(\w+)\]/g;
        text = text.replace(linkPattern, function(matched, linkText, linkTo) {
            return makeLink(linkTo, linkText);
        });

        var childrenCmd = /\[CMD_CHILDREN\]/g;
        if (childrenCmd.test(text)) {
            text = text.replace(childrenCmd, getChildren(topic));
        }

        var childrenSortedCmd = /\[CMD_CHILDREN_SORTED\]/g;
        if (childrenSortedCmd.test(text)) {
            text = text.replace(childrenSortedCmd, getChildrenSorted(topic));
        }

        dialog.text = text;
    }

    HelpUi.show = function(topic) {
        if (!topic)
            topic = 'TAG_ROOT';

        if (!dialog) {
            dialog = gameView.addGuiItem('interface/Help.qml');
            dialog.linkClicked.connect(function(link) {
                navigate(link);
            });
            dialog.closeClicked.connect(function() {
                dialog.deleteLater();
                dialog = null;
            });
        }

        load();

        navigate(topic);
    }

})();
