# SPARTA as an External Project
# ===========================
set(SPARTA_SEARCH_DIR /usr/local)
#set(SPARTA_SEARCH_DIR thirdparty/map/build)
set(SPARTA_INCLUDE_DIRS ${SPARTA_SEARCH_DIR}/include/sparta)
set(CMAKE_MODULE_PATH "${SPARTA_SEARCH_DIR}/lib/cmake/sparta" ${CMAKE_MODULE_PATH})
find_package(Sparta REQUIRED)

list(APPEND Perseus_LIBS SPARTA::sparta)
list(APPEND Perseus_INCLUDES ${SPARTA_INCLUDE_DIRS})

