var path = require('path');

function assertEqual(a, b) {
    if (a != b) {
        console.log("Actual:" + a);
        console.log("Expected:" + b);
        throw new Error();
    }
}

assertEqual(
    path.normalize('a\\b\\..\\c/./d/././.'),
    "a\\c\\d\\.");

assertEqual(
    path.resolve('c:\\foo/bar', "a.txt"), 
    "c:\\foo\\bar\\a.txt");

assertEqual(
    path.resolve("abc.txt"), 
    process.cwd() + "\\abc.txt");

assertEqual(
    path.resolve("abc", "efg", "../hij", "./xyz.txt"),
    process.cwd() + "\\abc\\hij\\xyz.txt");

assertEqual(
    path.resolve("abc", "d:/a.txt"), 
    "d:\\a.txt");

assertEqual(
    path.dirname("c:"), 
    "c:");

assertEqual(
    path.dirname("c:\\windows"), 
    "c:\\");

assertEqual(
    path.dirname("c:\\windows\\abc.txt"), 
    "c:\\windows");

assertEqual(
    path.basename("c:\\windows\\abc.txt"), 
    "abc.txt");

assertEqual(
    path.basename("c:\\windows\\a"), 
    "a");

assertEqual(
    path.basename("c:\\windows\\abc.txt", ".txt"), 
    "abc");

assertEqual(
    path.basename("c:\\windows\\abc.txt", ".Txt"), 
    "abc.txt");

assertEqual(
    path.extname("c:\\windows\\abc.txt"), 
    ".txt");

assertEqual(
    path.extname("c:\\windows\\a.json.txt"), 
    ".txt");

assertEqual(
    path.extname("c:\\windows\\a."), 
    ".");

assertEqual(
    path.isAbsolute("c:\\windows\\a."), 
    true);

assertEqual(
    path.isAbsolute("c:/windows/.."), 
    true);

assertEqual(
    path.isAbsolute("../abc"), 
    false);

assertEqual(
    path.isAbsolute("./abc"), 
    false);

assertEqual(
    path.isAbsolute("abc"), 
    false);

assertEqual(
    path.relative("c:\\a\\..\\b", "c:\\b"), 
    ".");

assertEqual(
    path.relative("c:/a", "d:/b/../c"), 
    "d:\\c");

assertEqual(
    path.relative("z:/a", "a.txt"), 
    process.cwd() + "\\a.txt");

assertEqual(
    path.relative("c:/a", "c:/"), 
    "..");

true;
