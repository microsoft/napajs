var napa = require('../bin/addon');
var program = require('commander');
var readline = require('readline');

// Set command line options.
program.version('0.0.1')
       .option('--file <filename>', 'Specify the file to load, if not provided will go into interactive mode')
       .option('--cores <num>', 'Number of cores/isolates the container will use (1 by default)', Number, 1)
       .option('--timeout <ms>', "Specify timeout in milliseconds for run extension (2000 by default)", Number, 2000)
       .parse(process.argv);

// Initialize napa.
napa.initialize({
    loggingProvider: "nop",
});

// Create a container.
var containerSettings = {};
containerSettings["cores"] = program.cores ? program['cores'] : 1;
var container = napa.createContainer(containerSettings);

if (program.file) {
    container.loadFileSync(program['file']);
    napa.shutdown();
} else {
    // Interactive mode.
    launchInteractiveMode(container);
}

/// <summary> Enables single line interactive mode on the provided container. <summary>
function launchInteractiveMode(container) {
    var rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        terminal: true
    });

    // Set the prompt to be '>' with bright cyan color.
    rl.setPrompt('\x1b[1m\x1b[36m>\x1b[0m ');
    rl.prompt(true);

    rl.on('line', (input) => {
        if (input.length > 0) {
            if (input[0] === '!') {
                invokeExtension(input.substring(1).trim());
            } else {
                container.loadSync(input);
            }
        }

        rl.prompt(true);
    });

    rl.on('close', () => {
        napa.shutdown();
    });
}

/// <summary> Mapping of extension name to extension method. <summary>
var extensions = {
    run: runExtension
};

/// Summary> Dispatches to the correct extension handler. </summary>
function invokeExtension(input) {
    var extensionNameEndPosition = input.indexOf(' ');
    if (extensionNameEndPosition == -1) {
        console.log("Failed to parse the extension name");
        return;
    }

    var extension = input.substring(0, extensionNameEndPosition);
    var extensionFunc = extensions[extension];
    if (extensionFunc === undefined) {
        console.log("Extension '" + extension + "' not found");
        return;
    }
    extensionFunc(input.substring(extensionNameEndPosition).trim());
}

/// <summary> 'run' extension implementation. Usage: !run functionName arg1 arg2 <summary>
function runExtension(args) {
    var argv = args.split(' ');
    if (argv.length == 0) {
        console.log("Illegal call to 'run' extension, use: '!run functionName arg1 arg2'");
        return;
    }

    var response = container.runSync(argv[0], argv.splice(1), program.timeout);
    if (response.code == 0) {
        console.log("\x1b[1m\x1b[32mResponse:\x1b[0m " + JSON.stringify(response.returnValue));
    } else {
        console.log("\x1b[1m\x1b[31mError:\x1b[0m " + response.errorMessage);
    }
}