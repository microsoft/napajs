// This script is used for updating patch numbers in package.json files.
// It uses the current date to create an 8 digit patch number.

var fs = require('fs');

var packageObj = JSON.parse(fs.readFileSync('package.json'));

if (packageObj.version === undefined) {
    console.error("package.json must have a version property");
    process.exit(1);
}

// Create new package version
var packageVersion = packageObj.version.trim();
packageVersion = packageVersion.substring(0, packageVersion.lastIndexOf('.') + 1);
packageVersion += getPatchNumber();

// Update the package version
packageObj.version = packageVersion;

fs.writeFileSync('package.json', JSON.stringify(packageObj, null, 4));

function getPatchNumber() {
    var date = new Date();

    // Get the current year.
    var year = date.getUTCFullYear().toString();

    // getMonth returns [0,11] so adjust number for normal display.
    var month = (date.getUTCMonth() + 1).toString();
    if (month.length == 1) {
        month = "0" + month;
    }

    // 2 digits that represent the current day in the month.
    var day = date.getUTCDate().toString();

    return year + month + day;
}
