# How to build Napa.V8.Library

Napa runtime requires V8 libraries and this describes how to build a nuget package containing necessary V8 header and library files.

1. Clone node from git.
```cmd
> git clone https://github.com/nodejs/node.git
```

2. Open 'common.gypi' and set 'WholeProgramOptimization' as false and disable 'LTCG'. Otherwise, retail build will complain of compiler version mismatch.

3. Compile both retail and debug version of node as shared library.
```cmd
node> vcbuild.bat x64 debug dll
node> vcbuild.bat x64 release dll
```

4. Open Napa.V8.Library.nuproj and set 'NapaV8LibraryRoot' as node root directory.

5. Update nuget version if necessary. Note that patch version is the same as node module version defined at node_version.h. Also, update icu and v8 libraries if node version is different from existing one.

6. Run 'quickbuild' and vefify nuget package is successfully created at output directory.

7. Go to '..\..\nuget\tools\packages\Microsoft.VisualStudio.Services.Packaging.NuGet.PushTool.0.11.0\tools'

8. Run
```cmd
VstsNuGetPush.exe -Package ..\..\..\..\..\vanilla\deps\objd\amd64\napa.v8.library.{major}.{minor}.{patch}.nupkg -Feed https://msasg.pkgs.visualstudio.com/_packaging/Bing/nuget/v3/index.json
```

You should be able to search for and find napa.v8.libary in 
https://msasg.visualstudio.com/DefaultCollection/Engineering%20Fundamentals/_apps/hub/ms.feed.feed-hub?feedName=Bing&protocolType=NuGet&packageName=ads.nativeads.log.library&packageVersion=1.0.4&_a=view
