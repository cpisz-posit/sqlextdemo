cmake_minimum_required(VERSION 3.16)
set(CMAKE_STANDARD_CXX_11)

# Soci with Sqlite 3
option(SOCI_CXX11 "" ON)

find_package(Boost REQUIRED COMPONENTS date_time filesystem)
find_package(SQLite3 REQUIRED)
find_package(SOCI REQUIRED)
find_package(GTest REQUIRED)

# target
add_executable(sqlite_extensions_tests
   uuidextTests.cpp
)

target_include_directories(sqlite_extensions_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/includes    
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(sqlite_extensions_tests PRIVATE
   Boost::date_time
   Boost::filesystem
   SQLite::SQLite3
   SOCI::soci_core
   SOCI::soci_sqlite3
   sqlite_extensions
)
