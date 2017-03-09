var fs = require('fs');

if (fs.existsSync('file-not-exist.json')) throw new Error();

if (!fs.existsSync('fstest.json')) throw new Error();

var content = JSON.parse(fs.readFileSync('fstest.json'));
if (content.query != 'query' || content.document != 'document') throw new Error();

require('./tests/sub/fstestsub');
