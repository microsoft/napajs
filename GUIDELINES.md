# napa/vanilla: root directory for Vanilla Napa 

### IMPORTANT: We follow an approach of adapting old Napa code to this folder in an incremental way, which means every incremental step needs to be clean (no unwanted dependencies introduced). and code get thoroughly reviewed and signed off by vanilla v-team.

## **Best practices**
1. Favor small incremental changes.
2. Every change get signed off by every member of vanilla v-team (Daiyi, Sunghoon, Asi, Ran)
3. Don't bring external dependencies except approved from vanilla v-team.

## **Folder structures**

General guideline:
1. Include necessary code for napa runtime, app engine and module support.
2. Built-in and core Napa modules. (autopilot, bing_features etc will be kept out of this folder)
3. Documentations

### **Source structure**

```
vanilla \
  inc \                            : Root of public header files.
    napa-runtime.h                 : C++ facade header file for napa runtime APIs.
    napa-app.h                     : C++ facade header file for napa app APIs.
    napa-c.h                       : C facade header of napa APIs.
    napa \                         : Public component-level headers
  
  src \                            : Root of implementations.
    <dir-for-component1>           : Static lib for component 1.
       lib \                       : Source code of static lib.
       unittest \                      : Unit test of static lib.
    <dir-for-component2>           : Static lib for component 2.
       lib \                       : Source code of static lib.
       unittest \                      : Unit test of static lib.
    scripts                        : TypeScript files in vanilla.
    napa                           : napa.dll
  
  binding \                        : Root of language bindings of vanilla.
    node \                         : binding for Node.JS.
    csharp \                       : binding for C#.
    ...
  
  tests \                          : Root of end-to-end tests.
    <test1> \                      : Sub-folder for test1.
    <test2> \                      : Sub-folder for test2.
    <test3> \                      : Sub-folder for test3.

  examples \                       : Root of examples.
    <example1> \                   : Sub-folder for example 1.
    <example2> \                   : Sub-folder for example 2.
    <example3> \                   : Sub-folder for example 3.

  docs \                           : Root for user facing documentations.
    index.md \                     : Home page in markdown format.
    features \                     : Home directory for features documentations.
      <feature1.md>                : Documentation for feature 1.
      <feature2.md>                : Documentation for feature 2.
    ...

```

### **Distributable structure**

For both under Node.JS and embed mode.
```
  .\                               : Current directory of hosting service.
    node_modules \                 : Root of node_modules.
      napa \                       : Root of napa module.
        internal \                 : Internal .js files to be compatible with Node.JS
      
      <module1> \                  : Module 1 that support for both Node.JS and Napa.
        <js-file1.js>              : JS that works for both Node.JS and Napa.
        <addon1.napa>              : Binary addon for Napa.
        <addon1.node>              : Binary addon for Node.JS.
      
      <module2> \                  : Module 2 that support only Node.JS.
        <js-file2.js>              : JS that may work only for Node.JS.
        <addon2.node>              : Binary for Node.JS
      
      <application1> \             : Application 1 that may work for both Node.JS and Napa.
        <js-file3.js>              : JS that works for both Node.JS and Napa.
        <js-file4.js>              : JS that works only for Node.JS
        <addon3.napa>              : Binary addon for Napa.
        <addon3.node>              : Binary addon for Node.
```

## **Planned stages**
1. Bootstrap vanilla folder with well defined API and examples.
2. Implement API with adapting existing Napa components to this folder. (Except bug fix, original Napa core will not take new features)
3. Deprecating original Napa core.

## **Open issues**
1. Find a solution for a build system that works both for CloudBuild and OSS (GIT hub).
2. Finalize coding standard for C++ and TypeScript.