cmake_minimum_required(VERSION 3.16)
set(CMAKE_STANDARD_CXX_11)

find_package(SQLite3 REQUIRED)

add_library(objlib OBJECT
   uuidext.cpp
)

target_include_directories(objlib PUBLIC
   ${CMAKE_SOURCE_DIR}/includes
   ${SQLite3_INCLUDE_DIRS}
)

add_library(sqlite_extensions STATIC
   $<TARGET_OBJECTS:objlib>
)

# Note that anything linking to this will have a dependency on SQLite::SQLite3
