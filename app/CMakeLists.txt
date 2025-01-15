cmake_minimum_required(VERSION 3.16)
set(CMAKE_STANDARD_CXX_20)

# Boost
find_package(Boost REQUIRED COMPONENTS date_time)

#Soci with Sqlite 3
option(SOCI_CXX11 "" ON)

find_package(SQLite3 REQUIRED)
find_package(SOCI REQUIRED)

# target
add_executable(app
   main.cpp
)

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/includes    
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(app PRIVATE
   Boost::date_time
   SQLite::SQLite3
   SOCI::soci_core
   SOCI::soci_sqlite3
   sqlite_extensions
)
