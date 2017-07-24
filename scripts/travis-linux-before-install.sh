#!/bin/bash

if [ "$COMPILER_OVERRIDE" == "" ]; then
  #use g++5 by default
  export CXX="g++-5" CC="gcc-5"
else
  export ${COMPILER_OVERRIDE}
fi
