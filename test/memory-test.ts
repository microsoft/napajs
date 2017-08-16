// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from 'assert';
import * as path from 'path';

let NAPA_ZONE_TEST_MODULE = path.resolve(__dirname, 'napa-zone/test');

describe('napajs/memory',
         function() {
             this.timeout(0);
             let napaZone = napa.zone.create('zone5');

             describe('Handle',
                      () => {
                          it('#isEmpty',
                             () => {
                                 assert(napa.memory.isEmpty([ 0, 0 ]));
                                 assert(!napa.memory.isEmpty([ 1, 2 ]));
                             });
                      });

             describe('Shareable',
                      () => {
                          it('#isShareable',
                             () => {
                                 assert(napa.memory.isShareable(napa.memory.crtAllocator));
                                 assert(!napa.memory.isShareable("hello world"));
                                 assert(!napa.memory.isShareable(1));
                                 assert(!napa.memory.isShareable({ a : 1 }));
                             });
                      });

             describe(
                 'Allocators',
                 () => {
                     it('@node: crtAllocator',
                        () => {
                            let handle = napa.memory.crtAllocator.allocate(10);
                            assert(!napa.memory.isEmpty(handle));
                            napa.memory.crtAllocator.deallocate(handle, 10);
                        });

                     it('@napa: crtAllocator', () => { napaZone.execute(NAPA_ZONE_TEST_MODULE, "crtAllocatorTest"); });

                     it('@node: defaultAllocator',
                        () => {
                            let handle = napa.memory.defaultAllocator.allocate(10);
                            assert(!napa.memory.isEmpty(handle));
                            napa.memory.defaultAllocator.deallocate(handle, 10);
                        });

                     it('@napa: defaultAllocator',
                        () => { napaZone.execute(NAPA_ZONE_TEST_MODULE, "defaultAllocatorTest"); });

                     it('@node: debugAllocator',
                        () => {
                            let allocator = napa.memory.debugAllocator(napa.memory.defaultAllocator);
                            let handle = allocator.allocate(10);
                            assert(!napa.memory.isEmpty(handle));
                            allocator.deallocate(handle, 10);
                            let debugInfo = JSON.parse(allocator.getDebugInfo());
                            assert.deepEqual(
                                debugInfo, { allocate : 1, allocatedSize : 10, deallocate : 1, deallocatedSize : 10 });
                        });

                     it('@napa: debugAllocator',
                        () => { napaZone.execute(NAPA_ZONE_TEST_MODULE, "debugAllocatorTest"); });
                 });
         });