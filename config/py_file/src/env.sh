PROJ_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && cd .. && cd .. && pwd )"

PROJ_BUILD_PATH=$PROJ_ROOT/cmake-build-debug

if [ ! -d "$PROJ_BUILD_PATH" ]; then
    mkdir -p $PROJ_BUILD_PATH
fi

PYTHON_LIB_PATH=$PROJ_ROOT/lib/python
echo $PYTHON_LIB_PATH

if [ ! -d "$PYTHON_LIB_PATH" ]; then
    mkdir $PYTHON_LIB_PATH
fi

export PYTHONPATH="$PROJ_ROOT/lib/python:$PYTHONPATH"
export LD_LIBRARY_PATH="$PROJ_ROOT/thirdparty/DRAMsim3:$LD_LIBRARY_PATH"