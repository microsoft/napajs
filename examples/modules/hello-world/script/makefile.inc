!if "$(BUILD_PASS)"=="PASS2"

NAPA_BUILD = $(INETROOT)\private\napa\build

BuildScripts:
    $(NAPA_BUILD)\node\node.exe $(NAPA_BUILD)\codegen\tsconfig_gen.js -t $(NAPA_BUILD)\codegen\tsconfig.firstparty.json -v NAPABUILD_TYPES $(NAPA_BUILD)\@types

    $(NAPA_BUILD)\node\node.exe $(NAPA_BUILD)\typescript\lib\tsc.js --outDir $(O)

CreateNpmPackage:
    if not exist $(O)\dist mkdir $(O)\dist
    if not exist $(O)\dist\bin mkdir $(O)\dist\bin
    if not exist $(O)\dist\lib mkdir $(O)\dist\lib
    if not exist $(O)\dist\typings mkdir $(O)\dist\typings

    copy /Y ..\node\$(O)\addon.node $(O)\dist\bin
    copy /Y ..\napa\$(O)\addon.napa $(O)\dist\bin

    copy /Y $(O)\hello-world.js $(O)\dist\lib
    copy /Y $(O)\hello-world.d.ts $(O)\dist\typings

    copy /Y tsconfig.json $(O)\dist
    copy /Y package.json $(O)\dist
!endif
