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
find_package(Boost REQUIRED COMPONENTS thread)

#Soci with Sqlite 3
find_package(SQLite3 REQUIRED)


message(STATUS poop: ${SQLite3_LIBRARIES})


# set(RSTUDIO_TOOLS_SOCI "${RSTUDIO_TOOLS_ROOT}/soci-4.0.3")
# set(Soci_DIR "${RSTUDIO_TOOLS_SOCI}/build/src/core")
# find_package(Soci REQUIRED)

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
   SQLite::SQLite3
#   soci::core
#   soci::sqlite
)