# napa/vanilla: root directory for Vanilla Napa 

### IMPORTANT: We follow an approach of adapting old Napa code to this folder in an incremental way, which means every incremental step needs to be clean (no unwanted dependencies introduced). and code get thoroughly reviewed and signed off by vanilla v-team.

## **Best practices**
1. Favor small incremental changes.
2. Every change get signed off by every member of vanilla v-team (Daiyi, Sunghoon, Asi, Ran)
3. Don't bring external dependencies except approved from vanilla v-team.

## **Folder scope**
1. Include necessary code for napa runtime, app engine and module support.
2. Built-in and core Napa modules. (autopilot, bing_features etc will be kept out of this folder)
3. Documentations

## **Planned stages**
1. Bootstrap vanilla folder with well defined API and examples.
2. Implement API with adapting existing Napa components to this folder. (Except bug fix, original Napa core will not take new features)
3. Deprecating original Napa core.

## **Open issues**
1. Find a solution for a build system that works both for CloudBuild and OSS (GIT hub).
2. Finalize coding standard for C++ and TypeScript.