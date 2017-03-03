var assert = require('assert');

// TODO @suchoi: Move to 'napa/shared-depot'.
var sharedDepot = require('napa-shared-depot');
var dummy = require('./dummy');

describe('Test suite for napa-shared-depot', function() {
    var dummyKey: string = 'dummy';

    it('creates a barrel', function() {
        var barrel = sharedDepot.assign();
        assert(barrel != null);
        assert.equal(sharedDepot.count(), 1);
    })

    it('creates a shared string and adds into a shared barrel', function() {
        var barrel = sharedDepot.assign();

        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        assert(!barrel.exists(dummyKey));

        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 1);

        assert(barrel.exists(dummyKey));

        sharedString = barrel.get(dummyKey);
        assert(sharedString != null);

        var result: string = dummy.readTestableSharedString(sharedString);
        assert.equal(result, 'napa');
    });

    it('restores a barrel using identifier', function() {
        var barrel = sharedDepot.assign();

        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');
        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);

        var restored: any = sharedDepot.find(barrel.id());
        assert(restored != null);

        sharedString = restored.get(dummyKey);
        assert(sharedString != null);

        var result: string = dummy.readTestableSharedString(sharedString);
        assert.equal(result, 'napa');
    });

    it('assigns and releases a barrel', function() {
        var barrel = sharedDepot.assign();

        var anotherBarrel: any = sharedDepot.assign();
        assert.equal(sharedDepot.count(), 2);

        sharedDepot.release(anotherBarrel);
        assert.equal(sharedDepot.count(), 1);
    });

    it('adds two shared strings into a barrel', function() {
        var barrel = sharedDepot.assign();
        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 1);

        assert(barrel.exists(dummyKey));

        sharedString = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'vteam');

        succeeded = barrel.set(dummyKey, sharedString);
        assert(!succeeded);
        assert.equal(barrel.count(), 1);

        succeeded = barrel.set(dummyKey + '1', sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 2);
        
        assert(barrel.exists(dummyKey + '1'));
    });

    it('deletes a shared string from a barrel', function() {
        var barrel = sharedDepot.assign();
        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        barrel.remove(dummyKey);

        var sharedString: any = barrel.get(dummyKey);
        assert(sharedString == null);

        assert.equal(barrel.count(), 0);
    });
})
