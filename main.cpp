
#include <boost/thread.hpp>
#include <sqlite3.h>
#include <soci/soci.h>

#include <iostream>

int main()
{
    boost::thread thread;

    sqlite3 *database = nullptr;
    int result = sqlite3_open("file:testdb.db", &database);
    
    if(result)
    {
        std::cerr << "Could not open sqlite db" << sqlite3_errmsg(database) << std::endl;
        sqlite3_close(database);
        return 1;
    }

    sqlite3_close(database);

    return 0;
}