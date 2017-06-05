import * as napa from "napajs";
import * as assert from "assert";
import * as path from "path";

// Require this module to verify logging was done correctly
var testProvider = require('test-provider');

napa.runtime.setPlatformSettings({ 
    loggingProvider: "test-provider",
    metricProvider: "test-provider"
});

let zone: napa.zone.Zone = napa.zone.create('napa-zone1', { workers: 1});

describe('napajs/logging', function () {

    it('@node: default log', () => {
        napa.log("test");
        assert.strictEqual(testProvider.lastLog.message, 'test');
        assert.strictEqual(testProvider.lastLog.level, 2 /* LogLevel.Info */);
    });

    it('@napa: default log', () => {
        zone.broadcastSync(() => {
            var napa = require('napajs');
            napa.log("test-from-napa");
        }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa');
        assert.strictEqual(testProvider.lastLog.level, 2 /* LogLevel.Info */);
    });

    it('@node: log with section', () => {
        napa.log("test-section", "test");
        assert.strictEqual(testProvider.lastLog.message, 'test');
        assert.strictEqual(testProvider.lastLog.section, "test-section");
    });

    it('@napa: log with section', () => {
        zone.broadcastSync(() => {
            var napa = require('napajs');
            napa.log("test-section", "test-from-napa");
        }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa');
        assert.strictEqual(testProvider.lastLog.section, "test-section");
    });

    it('@node: log with section and trace id', () => {
        napa.log("test-section", "trace-id", "test");
        assert.strictEqual(testProvider.lastLog.message, 'test');
        assert.strictEqual(testProvider.lastLog.section, "test-section");
        assert.strictEqual(testProvider.lastLog.traceId, 'trace-id');
    });

    it('@napa: log with section and trace id', () => {
        zone.broadcastSync(() => {
            var napa = require('napajs');
            napa.log("test-section", "trace-id", "test-from-napa");
        }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa');
        assert.strictEqual(testProvider.lastLog.section, "test-section");
        assert.strictEqual(testProvider.lastLog.traceId, 'trace-id');
    });

    it('@node: log with verbosity', () => {
        napa.log.err("test0");
        assert.strictEqual(testProvider.lastLog.message, 'test0');
        assert.strictEqual(testProvider.lastLog.level, 0 /* LogLevel.Error */);

        napa.log.warn("test1");
        assert.strictEqual(testProvider.lastLog.message, 'test1');
        assert.strictEqual(testProvider.lastLog.level, 1 /* LogLevel.Warning */);

        napa.log.info("test2");
        assert.strictEqual(testProvider.lastLog.message, 'test2');
        assert.strictEqual(testProvider.lastLog.level, 2 /* LogLevel.Information */);

        napa.log.debug("test3");
        assert.strictEqual(testProvider.lastLog.message, 'test3');
        assert.strictEqual(testProvider.lastLog.level, 3 /* LogLevel.Debug */);
    });

    it('@napa: log with verbosity', () => {
        zone.broadcastSync(() => { var napa = require('napajs'); napa.log.err("test-from-napa0"); }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa0');
        assert.strictEqual(testProvider.lastLog.level, 0 /* LogLevel.Error */);

        zone.broadcastSync(() => { var napa = require('napajs'); napa.log.warn("test-from-napa1"); }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa1');
        assert.strictEqual(testProvider.lastLog.level, 1 /* LogLevel.Warning */);

        zone.broadcastSync(() => { var napa = require('napajs'); napa.log.info("test-from-napa2"); }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa2');
        assert.strictEqual(testProvider.lastLog.level, 2 /* LogLevel.Information */);

        zone.broadcastSync(() => { var napa = require('napajs'); napa.log.debug("test-from-napa3"); }, []);
        assert.strictEqual(testProvider.lastLog.message, 'test-from-napa3');
        assert.strictEqual(testProvider.lastLog.level, 3 /* LogLevel.Debug */);
    });
});

describe('napajs/metric', function () {

    it('@node: get metric', () => {
        let metric = napa.metric.get("section1", "name1", napa.metric.MetricType.Number);
        assert.strictEqual(metric.section, 'section1');
        assert.strictEqual(metric.name, 'name1');
    });

    it('@napa: get metric', () => {
        zone.broadcastSync(() => { 
            var napa = require('napajs'); 
            var metric = napa.metric.get("section2", "name2", napa.metric.MetricType.Number) ;
            metric.set(24);
        }, []);

        assert.strictEqual(testProvider.lastMetricValue, 24);
    });

    it('@node: get metric from cache (per isolate)', () => {
        let metric = napa.metric.get("section3", "name3", napa.metric.MetricType.Number);
        metric.set(7);
        metric = napa.metric.get("section3", "name3", napa.metric.MetricType.Number);
        assert.strictEqual(testProvider.lastMetricValue, 7);
    });

    it('@napa: get metric from cache (per isolate)', () => {
        zone.broadcastSync(() => { 
            var napa = require('napajs'); 
            let metric = napa.metric.get("section4", "name4", napa.metric.MetricType.Number);
            metric.set(15);
            metric = napa.metric.get("section4", "name4", napa.metric.MetricType.Number);
            metric.increment();
        }, []);

        assert.strictEqual(testProvider.lastMetricValue, 16);
    });
});