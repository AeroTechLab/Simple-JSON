cmake_minimum_required( VERSION 2.8 )
project( Simple-JSON )

set( CMAKE_C_STANDARD 99 )
set( CMAKE_C_STANDARD_REQUIRED ON )

set( LIBRARY_DIR CACHE PATH "Relative or absolute path to directory where built shared libraries will be placed" )

add_library( SimpleJSON SHARED ${CMAKE_CURRENT_LIST_DIR}/json.c )
set_target_properties( SimpleJSON PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR} )
target_include_directories( SimpleJSON PUBLIC ${CMAKE_CURRENT_LIST_DIR} )
target_compile_definitions( SimpleJSON PUBLIC -DDEBUG )
