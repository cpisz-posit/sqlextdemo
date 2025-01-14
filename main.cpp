
#include "uuidext.hpp"

#include <boost/thread.hpp>
#include <sqlite3.h>
#include <soci/soci.h>

#include <iostream>


void testsoci_w_sqlite_ext()
{
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

        sql <<
            "INSERT INTO licensed_users "
            "VALUES ("
                "'Jane',"                       // user_name
                "0,"                            // locked
                "'2013-11-07T08:23:19.120Z',"   // last_sign_in
                "0,"                            // is_admin
                "551,"                          // user_id
                "'some aws arn',"               // aws_role_arn
                "'some aws session name',"      // aws_role_session_name
                "'some id token',"              // id_token
                "'some refresh token',"         // refresh_token
                "'2015-11-22T012:23:19.120Z',"  // token_expiry
                "123,"                          // id
                "'2012-11-07T08:23:19.120Z',"   // created
                "'2025-02-14T08:23:19.120Z',"   // last_modified
                "'1',"                          // version
                "'janedoe@posit.co',"           // email
                "'jane.doe',"                   // display_name
                "'janed',"                      // posix_name
                "'shadow',"                     // shadow
                "'/home/janed/',"               // homedir
                "1"                             // active
            ")";

        sql <<
            "INSERT INTO licensed_users "
            "VALUES ("
                "'John',"                       // user_name
                "0,"                            // locked
                "'2013-10-07T08:23:19.120Z',"   // last_sign_in
                "0,"                            // is_admin
                "550,"                          // user_id
                "'some aws arn',"               // aws_role_arn
                "'some aws session name',"      // aws_role_session_name
                "'some id token',"              // id_token
                "'some refresh token',"         // refresh_token
                "'2015-10-22T012:23:19.120Z',"  // token_expiry
                "124,"                          // id
                "'2012-10-07T08:23:19.120Z',"   // created
                "'2025-01-14T08:23:19.120Z',"   // last_modified
                "'1',"                          // version
                "'johndoe@posit.co',"           // email
                "'john.doe',"                   // display_name
                "'johnd',"                      // posix_name
                "'shadow',"                     // shadow
                "'/home/johnd/',"               // homedir
                "1"                             // active
            ")";
    }
    catch(const soci::soci_error & e)
    {
        std::cerr << e.what() << '\n';
    }

    // Test extension
    try
    {
        soci::session sql("sqlite3", "file:testdb.db");
        
        sql << "ALTER TABLE licensed_users ADD COLUMN uuid varchar(36) NOT NULL DEFAULT('0')";
        sql << "UPDATE licensed_users SET uuid = uuid()";
    }
    catch(const soci::soci_error & e)
    {
        std::cerr << e.what() << '\n';
    }
}

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

    // Register extention
    //
    // Sqlite does something very odd where they require you to make an extension registration function that returns an int and takes three params,
    // while also requiring you to pass it as a function that returns void and takes none. See https://www.sqlite.org/c3ref/auto_extension.html
    typedef void(*pfnInitExtensionFunction)(void);
    pfnInitExtensionFunction test = (pfnInitExtensionFunction)sqlite3_uuid_init;
    sqlite3_auto_extension(test);

    // Test soci using sqlite
    testsoci_w_sqlite_ext();

    return 0;
}