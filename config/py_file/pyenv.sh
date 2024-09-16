#!/bin/bash

PROJ_ROOT="$( cd "$( dirname "$0" )" && cd .. && cd .. && pwd )"
PROJ_BUILD_PATH=$PROJ_ROOT/cmake-build-debug

cd $PROJ_BUILD_PATH

export PYTHONPATH="$PROJ_ROOT/lib/python:$PYTHONPATH"
export LD_LIBRARY_PATH="$PROJ_ROOT/thirdparty/DRAMsim3:$LD_LIBRARY_PATH"

cmake --build $PROJ_BUILD_PATH --target unitlib -j 14

cmake --install $PROJ_BUILD_PATH

rm -rf $PROJ_ROOT/lib/stubs

pybind11-stubgen unitlib --output-dir "$PROJ_ROOT/lib/stubs"

cd -

# echo "PROJ_ROOT set to $PROJ_ROOT"
# echo "PYTHONPATH set to $PYTHONPATH"
# echo "Stub files generated in $PROJ_ROOT/stubs"
