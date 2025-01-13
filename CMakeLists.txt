project(SqlExtDemo)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_STANDARD_CXX_20)

# global directives
add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER -DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_definitions(-Wall -pthread -Werror=return-type -Wformat -Wformat-security -fstack-protector-strong -fPIC -O0 -Wduplicated-cond -Wduplicated-branches -Wrestrict -Wnull-dereference)
add_definitions(-DBOOST_ASIO_DISABLE_KQUEUE)
add_definitions(-D_FORTIFY_SOURCE=2)
add_definitions(-D_GLIBCXX_ASSERTIONS)

# Boost
find_package(Boost REQUIRED COMPONENTS date_time thread)

#Soci with Sqlite 3
option(SOCI_CXX11 "" ON)

find_package(SQLite3 REQUIRED)
find_package(SOCI REQUIRED)

# Project
include_directories(SYSTEM 
   ${Boost_INCLUDE_DIRS}
   ${SQLite3_INCLUDE_DIRS}
)

add_executable(demo
   main.cpp
)

# set link dependencies
target_link_libraries(demo PRIVATE
   Boost::thread
   Boost::date_time
   SQLite::SQLite3
   SOCI::soci_core
   SOCI::soci_sqlite3
   SOCI::soci_postgresql
)