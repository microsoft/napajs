var assert = require('assert');
var path = require('path');

assert.equal(
    path.normalize('a\\b\\..\\c/./d/././.'),
    "a\\c\\d\\.");

assert.equal(
    path.resolve('c:\\foo/bar', "a.txt"), 
    "c:\\foo\\bar\\a.txt");

assert.equal(
    path.resolve("abc.txt"), 
    process.cwd() + "\\abc.txt");

assert.equal(
    path.resolve("abc", "efg", "../hij", "./xyz.txt"),
    process.cwd() + "\\abc\\hij\\xyz.txt");

assert.equal(
    path.resolve("abc", "d:/a.txt"), 
    "d:\\a.txt");

assert.equal(
    path.join("/foo", "bar", "baz/asdf", "quux", ".."),
    "\\foo\\bar\\baz\\asdf");

assert.equal(
    path.dirname("c:"), 
    "c:");

assert.equal(
    path.dirname("c:\\windows"), 
    "c:\\");

assert.equal(
    path.dirname("c:\\windows\\abc.txt"), 
    "c:\\windows");

assert.equal(
    path.basename("c:\\windows\\abc.txt"), 
    "abc.txt");

assert.equal(
    path.basename("c:\\windows\\a"), 
    "a");

assert.equal(
    path.basename("c:\\windows\\abc.txt", ".txt"), 
    "abc");

assert.equal(
    path.basename("c:\\windows\\abc.txt", ".Txt"), 
    "abc.txt");

assert.equal(
    path.extname("c:\\windows\\abc.txt"), 
    ".txt");

assert.equal(
    path.extname("c:\\windows\\a.json.txt"), 
    ".txt");

assert.equal(
    path.extname("c:\\windows\\a."), 
    ".");

assert.equal(
    path.isAbsolute("c:\\windows\\a."), 
    true);

assert.equal(
    path.isAbsolute("c:/windows/.."), 
    true);

assert.equal(
    path.isAbsolute("../abc"), 
    false);

assert.equal(
    path.isAbsolute("./abc"), 
    false);

assert.equal(
    path.isAbsolute("abc"), 
    false);

assert.equal(
    path.relative("c:\\a\\..\\b", "c:\\b"), 
    ".");

assert.equal(
    path.relative("c:/a", "d:/b/../c"), 
    "d:\\c");

assert.equal(
    path.relative("z:/a", "a.txt"), 
    process.cwd() + "\\a.txt");

assert.equal(
    path.relative("c:/a", "c:/"), 
    "..");

assert.notEqual("foo/bar\\baz".indexOf(path.sep), -1);

true;