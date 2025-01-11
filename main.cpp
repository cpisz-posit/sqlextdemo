
#include <boost/thread.hpp>
#include <sqlite3.h>

int main()
{
    boost::thread thread;

    sqlite3 *database = nullptr;
    int result = sqlite3_open("file:testdb.db", &database);
    sqlite3_close(database);

    return 0;
}