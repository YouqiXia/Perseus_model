#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $SCRIPT_DIR/unitlib_env.sh

BUILD_MODE=release
CMAKE_BUILD_MODE=Release

PROJ_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"
if [ $# == 1 ]; then
    BUILD_MODE=$1
fi

case "$BUILD_MODE" in
    [Rr]elease)
        BUILD_MODE=release
        CMAKE_BUILD_MODE=Release
        ;;
    [Dd]ebug)
        BUILD_MODE=debug
        CMAKE_BUILD_MODE=Debug
        ;;
esac

BUILD_PATH=$PROJ_ROOT/$BUILD_MODE
if [ ! -d "$BUILD_PATH" ]; then
    mkdir -p $BUILD_PATH
fi
echo "Build path: $BUILD_PATH"
cd $BUILD_PATH

UNITLIB_PATH=$BUILD_PATH/unitlib

export PYTHONPATH="$UNITLIB_PATH/lib:$PYTHONPATH"

cmake $PROJ_ROOT -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_MODE
cmake --build $BUILD_PATH --target UnitLib -j 32

rm -rf $UNITLIB_PATH/stubs
pybind11-stubgen UnitLib --output-dir "$UNITLIB_PATH/stubs"

echo "=============================="
echo "You should run the command below to enable UnitLib in python!!!"
echo "export PYTHONPATH=$PYTHONPATH"
echo ""
echo "pyi path is $UNITLIB_PATH/stubs"
echo "=============================="