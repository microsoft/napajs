function fstestsub() {
    var filesystem = require('fs');
    var path = require('path');
    var content = JSON.parse(filesystem.readFileSync(path.resolve(__dirname, 'fstestsub.json')));
    if (content.subQuery != 'sub-query' || content.subDocument != 'sub-document') {
        return false;
    } else {
        return true;
    }
}

module.exports = fstestsub();