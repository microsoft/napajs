// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from "assert";
import * as path from "path";

type Zone = napa.zone.Zone;

describe('napajs/module', function () {
    this.timeout(0);
    let napaZone = napa.zone.create('module-tests-zone', { workers: 1 });

    describe('load', function () {
        it('javascript module', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var jsmodule = require(thisFilePath + '/module/jsmodule');

                assert.notEqual(jsmodule, undefined);
                assert.equal(jsmodule.wasLoaded, true);
            }, [__dirname]);
        });

        it('json module', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var jsonModule = require(thisFilePath + '/module/test.json');
                
                assert.notEqual(jsonModule, undefined);
                assert.equal(jsonModule.prop1, "val1");
                assert.equal(jsonModule.prop2, "val2");
            }, [__dirname]);
        });

        it('napa module', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var napaModule = require(thisFilePath + '/../bin/simple-napa-addon.napa');
                
                assert.notEqual(napaModule, undefined);
                assert.equal(napaModule.getModuleName(), "simple-napa-addon");
            }, [__dirname]);
        });

        it('object wrap module', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var napaModule = require(thisFilePath + '/../bin/simple-napa-addon.napa');
                
                var obj = napaModule.createSimpleObjectWrap();
                assert.notEqual(obj, undefined);
                obj.setValue(3);
                assert.equal(obj.getValue(), 3);
            }, [__dirname]);
        });

        it('circular dependencies', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");

                var cycle_a = require(thisFilePath + '/module/cycle-a.js');
                var cycle_b = require(thisFilePath + '/module/cycle-b.js');

                assert(cycle_a.done);
                assert(cycle_b.done);
            }, [__dirname]);
        });
    });

    describe('resolve', function () {
        // TODO: support correct __dirname in anonymous function and move tests from 'resolution-tests.js' here.
        it('require.resolve', () => {
            return napaZone.execute(__dirname + "/module/resolution-tests.js", "run");
        });
    });

    describe('core-modules', function () {
        describe('process', function () {
            it.skip('argv', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                
                    assert(process.argv.length > 0);
                    assert(process.argv[0].includes('node'));
                });
            });

            it('execPath', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                
                    assert(process.execPath.includes('node'));
                });
            });

            it('env', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                
                    process.env.test = "napa-test";
                    assert.equal(process.env.test, "napa-test");
                });
            });

            it('platform', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                
                    assert(process.platform == 'win32' || 
                           process.platform == 'darwin' || 
                           process.platform == 'linux' || 
                           process.platform == 'freebsd');
                });
            });

            it('umask', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    
                    var old = process.umask(0);
                    assert.equal(process.umask(old), 0);
                });
            });

            it('chdir', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    
                    var cwd = process.cwd();
                    process.chdir('..');
                    assert.notEqual(cwd, process.cwd());
                    assert(cwd.includes(process.cwd()));
                    process.chdir(cwd);
                    assert.equal(cwd, process.cwd());
                });
            });

            it('pid', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    
                    assert.notEqual(typeof process.pid, undefined);
                    assert(!isNaN(process.pid));
                });
            });
        });

        describe('fs', function () {
            it('existsSync', () => {
                return napaZone.execute((thisFilePath: string) => {
                    var assert = require("assert");
                    var fs = require('fs');

                    assert(fs.existsSync(thisFilePath + '/module/jsmodule.js'));
                    assert.ifError(fs.existsSync('non-existing-file.txt'));
                }, [__dirname]);
            });

            it('readFileSync', () => {
                return napaZone.execute((thisFilePath: string) => {
                    var assert = require("assert");
                    var fs = require('fs');

                    var content = JSON.parse(fs.readFileSync(thisFilePath + '/module/test.json'));
                    assert.equal(content.prop1, 'val1');
                    assert.equal(content.prop2, 'val2');
                }, [__dirname]);
            });

            it('mkdirSync', () => {
                return napaZone.execute((thisFilePath: string) => {
                    var assert = require("assert");
                    var fs = require('fs');

                    fs.mkdirSync(thisFilePath + '/module/test-dir');
                    assert(fs.existsSync(thisFilePath + '/module/test-dir'));
                }, [__dirname]).then(()=> {
                    // Cleanup
                    var fs = require('fs');
                    if (fs.existsSync('./module/test-dir')) {
                        fs.rmdir('./module/test-dir');
                    }
                })
            });

            it('writeFileSync', () => {
                return napaZone.execute((thisFilePath: string) => {
                    var assert = require("assert");
                    var fs = require('fs');

                    fs.writeFileSync(thisFilePath + '/module/test-file', 'test');
                    assert.equal(fs.readFileSync(thisFilePath + '/module/test-file'), 'test');
                }, [__dirname]).then(()=> {
                    // Cleanup
                    var fs = require('fs');
                    if (fs.existsSync('./module/test-file')) {
                        fs.unlinkSync('./module/test-file');
                    }
                })
            });

            it('readFileSync', () => {
                return napaZone.execute((thisFilePath: string) => {
                    var assert = require("assert");
                    var fs = require('fs');

                    var testDir = thisFilePath + '/module/test-dir';
                    fs.mkdirSync(testDir);
                    fs.writeFileSync(testDir + '/1', 'test');
                    fs.writeFileSync(testDir + '/2', 'test');
                    
                    assert.deepEqual(fs.readdirSync(testDir).sort(), ['1', '2']);
                }, [__dirname]).then(()=> {
                    // Cleanup
                    var fs = require('fs');
                    if (fs.existsSync('./module/test-dir')) {
                        fs.unlinkSync('./module/test-dir/1');
                        fs.unlinkSync('./module/test-dir/2');
                        fs.rmdir('./module/test-dir');
                    }
                })
            });
        });

        describe('path', function () {
            it('normalize', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.normalize('a\\b\\..\\c/./d/././.'), "a\\c\\d");
                    } else {
                        assert.equal(path.normalize('a\\b\\..\\c/./d/././.'), "a/c/d");
                    }
                });
            });

            it('resolve', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.resolve('c:\\foo/bar', "a.txt"), "c:\\foo\\bar\\a.txt");
                        assert.equal(path.resolve("abc.txt"), process.cwd() + "\\abc.txt");
                        assert.equal(path.resolve("abc", "efg", "../hij", "./xyz.txt"), process.cwd() + "\\abc\\hij\\xyz.txt");
                        assert.equal(path.resolve("abc", "d:/a.txt"), "d:\\a.txt");
                    } else {
                        assert.equal(path.resolve('/foo/bar', "a.txt"), "/foo/bar/a.txt");
                        assert.equal(path.resolve("abc.txt"), process.cwd() + "/abc.txt");
                        assert.equal(path.resolve("abc", "efg", "../hij", "./xyz.txt"), process.cwd() + "/abc/hij/xyz.txt");
                        assert.equal(path.resolve("abc", "/a.txt"), "/a.txt");
                    }
                });
            });

            it('join', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.join("/foo", "bar", "baz/asdf", "quux", ".."), "\\foo\\bar\\baz\\asdf");
                    } else {
                        assert.equal(path.join("/foo", "bar", "baz/asdf", "quux", ".."), "/foo/bar/baz/asdf");
                    }
                });
            });

            // TODO: fix bugs
            //      1. Error: the string "AssertionError: '.' == 'c:'" was thrown, throw an Error :)
            //      2. Error: the string "AssertionError: 'c:' == 'c:\\\\'" was thrown, throw an Error :)
            it.skip('dirname', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.dirname("c:"), "c:");
                        assert.equal(path.dirname("c:\\windows"), "c:\\");
                        assert.equal(path.dirname("c:\\windows\\abc.txt"), "c:\\windows");
                    } else {
                        assert.equal(path.dirname("/"), "/");
                        assert.equal(path.dirname("/etc"), "/");
                        assert.equal(path.dirname("/etc/passwd"), "/etc");
                    }
                });
            });

            it('basename', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.basename("c:\\windows\\abc.txt"), "abc.txt");
                        assert.equal(path.basename("c:\\windows\\a"), "a");
                        assert.equal(path.basename("c:\\windows\\abc.txt", ".txt"), "abc");
                        assert.equal(path.basename("c:\\windows\\abc.txt", ".Txt"), "abc.txt");
                    } else {
                        assert.equal(path.basename("/test//abc.txt"), "abc.txt");
                        assert.equal(path.basename("/test//a"), "a");
                        assert.equal(path.basename("/test/abc.txt", ".txt"), "abc");
                        assert.equal(path.basename("/windows/abc.txt", ".Txt"), "abc.txt");
                    }
                });
            });

            // TODO: fix bugs
            //      1. Error: the string "AssertionError: '' == '.'" was thrown, throw an Error :)
            it.skip('extname', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.extname("c:\\windows\\abc.txt"), ".txt");
                        assert.equal(path.extname("c:\\windows\\a.json.txt"), ".txt");
                        assert.equal(path.extname("c:\\windows\\a."), ".");
                    } else {
                        assert.equal(path.extname("/test/abc.txt"), ".txt");
                        assert.equal(path.extname("/test/a.json.txt"), ".txt");
                        assert.equal(path.extname("/test/a."), ".");
                    }
                });
            });

            it('isAbsolute', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.isAbsolute("c:\\windows\\a."), true);
                        assert.equal(path.isAbsolute("c:/windows/.."), true);
                        assert.equal(path.isAbsolute("../abc"), false);
                        assert.equal(path.isAbsolute("./abc"), false);
                        assert.equal(path.isAbsolute("abc"), false);
                    } else {
                        assert.equal(path.isAbsolute("/test/a."), true);
                        assert.equal(path.isAbsolute("/test/.."), true);
                        assert.equal(path.isAbsolute("../abc"), false);
                        assert.equal(path.isAbsolute("./abc"), false);
                        assert.equal(path.isAbsolute("abc"), false);
                    }
                });
            });

            it('relative', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.relative("c:\\a\\..\\b", "c:\\b"), "");
                        assert.equal(path.relative("c:/a", "d:/b/../c"), "d:\\c");
                        assert.equal(path.relative("z:/a", "a.txt"), process.cwd() + "\\a.txt");
                        assert.equal(path.relative("c:/a", "c:/"), "..");
                    } else {
                        assert.equal(path.relative("/test/a/../b", "/test/b"), "");
                        assert.equal(path.relative("/test/a", "/test1/b/../c"), "../../test1/c");
                        assert.equal(path.relative("/test/a", "a.txt"), "../.." + process.cwd() + "/a.txt");
                        assert.equal(path.relative("/test/a", "/test/"), "..");
                    }
                });
            });

            it('sep', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var path = require("path");
                    
                    if (process.platform == 'win32') {
                        assert.equal(path.sep, "\\");
                    } else {
                        assert.equal(path.sep, "/");
                    }
                });
            });
        });

        describe('os', function () {
            it('type', () => {
                return napaZone.execute(() => {
                    var assert = require("assert");
                    var os = require("os");
                    
                    assert(os.type() == "Windows_NT" || os.type() == "Darwin" || os.type() == "Linux");
                });
            });
        });
    });

    describe('async', function () {
        it('post async work', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var napaModule = require(thisFilePath + '/../bin/simple-napa-addon.napa');

                var obj = napaModule.createSimpleObjectWrap();
                obj.setValue(3);

                var promise = new Promise((resolve) => {
                    obj.postIncrementWork((newValue: number) => {
                        resolve(newValue);
                    });
                });

                // The value shouldn't have changed yet.
                assert.equal(obj.getValue(), 3);

                return promise;
            }, [__dirname]).then((result: napa.zone.Result) => {
                assert.equal(result.value, 4);
            });
        });

        it('do async work', () => {
            return napaZone.execute((thisFilePath: string) => {
                var assert = require("assert");
                var napaModule = require(thisFilePath + '/../bin/simple-napa-addon.napa');

                var obj = napaModule.createSimpleObjectWrap();
                obj.setValue(8);

                var promise = new Promise((resolve) => {
                    obj.doIncrementWork((newValue: number) => {
                        resolve(newValue);
                    });
                });

                // The actual increment happened in the same thread.
                assert.equal(obj.getValue(), 9);

                return promise;
            }, [__dirname]).then((result: napa.zone.Result) => {
                assert.equal(result.value, 9);
            });
        });
    });
});