var sample = require('./sample.napa');

var runSampleNode = function() {
    // Create wrapped C++ object.
    var sampleNode = sample.createSampleNode(1);

    // Calls C++ function through accessor.
    sampleNode.score = 2;

    // Add property from javascript world.
    sampleNode.margin = 3;

    // Access new property from C++ world.
    sampleNode.score += sampleNode.getValue('margin');

    return sampleNode.score;
}

var runSampleContainer = function(score) {
    var sampleContainer = sample.createSampleContainer();

    // Access built-in member.
    sampleContainer.node.score = score;

    var sum = sampleContainer.node.score;
    var sampleNode = sample.createSampleNode(score * 2);

    // replace built-in member.
    sampleContainer.setNode(sampleNode);
    sum += sampleContainer.getNode().score;

    return sum;
}

if (runSampleNode(3) + runSampleContainer(3) == 14) {
    true;
} else {
    false;
}
