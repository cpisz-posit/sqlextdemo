
#include <boost/thread.hpp>
#include <sqlite3.h>
#include <soci/soci.h>

#include <iostream>

int main()
{
    // Did boost link ok?
    boost::thread thread;

    // Did sqlite3 link ok?
    sqlite3 *database = nullptr;
    int result = sqlite3_open("file:testdb.db", &database);
    
    if(result)
    {
        std::cerr << "Could not open sqlite db" << sqlite3_errmsg(database) << std::endl;
        sqlite3_close(database);
        return 1;
    }

    sqlite3_close(database);

    // Great, let's use soci

    // Make a table of existing data
    try
    {
        soci::session sql("sqlite3", "file:testdb.db");

        sql << "DROP TABLE IF EXISTS licensed_users";
        
        sql <<
            "CREATE TABLE licensed_users(" 
            "user_name text NOT NULL,"
            "locked boolean NOT NULL DEFAULT 0,"
            "last_sign_in text NOT NULL,"
            "is_admin boolean NOT NULL DEFAULT 0,"
            "user_id integer NOT NULL DEFAULT -1,"
            "aws_role_arn text,"
            "aws_role_session_name text,"
            "id_token text,"
            "refresh_token text,"
            "token_expiry text,"
            "id integer PRIMARY KEY,"
            "created TEXT,"
            "last_modified TEXT,"
            "version TEXT,"
            "email TEXT,"
            "display_name TEXT,"
            "posix_name TEXT,"
            "shadow TEXT,"
            "homedir TEXT,"
            "active BOOLEAN NOT NULL DEFAULT 1)";
    }
    catch(const soci::soci_error & e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}