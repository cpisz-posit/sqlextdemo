project(SqlExtDemo)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_STANDARD_CXX_20)

# global directives
add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER -DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_definitions(-Wall -pthread -Werror=return-type -Wformat -Wformat-security -fstack-protector-strong -fPIC -O0 -Wduplicated-cond -Wduplicated-branches -Wrestrict -Wnull-dereference)
add_definitions(-DBOOST_ASIO_DISABLE_KQUEUE)
add_definitions(-D_FORTIFY_SOURCE=2)
add_definitions(-D_GLIBCXX_ASSERTIONS)
add_definitions(-DSQLITE_CORE)

# targets
add_subdirectory(sqlite_extensions)
add_subdirectory(app)
add_subdirectory(tests)