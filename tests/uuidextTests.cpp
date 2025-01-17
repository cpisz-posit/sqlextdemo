
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include "sqlite_extensions/uuidext.hpp"

#include <sqlite3.h>
#include <soci/soci.h>
#include <boost/filesystem.hpp>

#include <regex>


TEST_CASE("The UUID SQlite extension creates UUIDs from SQL", "[uuidext]")
{
    // Register extention
    //
    // Sqlite does something very odd where they require you to make an extension registration function that returns an int and takes three params,
    // while also requiring you to pass it as a function that returns void and takes none. See https://www.sqlite.org/c3ref/auto_extension.html
    typedef void(*pfnInitExtensionFunction)(void);
    pfnInitExtensionFunction test = (pfnInitExtensionFunction)sqlite3_uuid_init;
    sqlite3_auto_extension(test);

    // Delete database if it exists
    auto deleteDbFn = [](){
        if( boost::filesystem::exists("testdb.db") )
        {
            boost::filesystem::remove("testdb.db");
        }
    };
    REQUIRE_NOTHROW(deleteDbFn());

    // Create a db to test with
    std::unique_ptr<soci::session> session;
    auto createDbFn = [&session]() 
    {
        session.reset(new soci::session("sqlite3", "file:testdb.db"));
    };
    REQUIRE_NOTHROW(createDbFn());

    // Give it a table
    auto createTableFn = [&session]() 
    {
        *session << "DROP TABLE IF EXISTS test_table";
    
        *session << "CREATE TABLE test_table(" 
            "id integer PRIMARY KEY,"
            "guid TEXT,"
            "guid_bytes BLOB)";
    };
    REQUIRE_NOTHROW(createTableFn());

    SECTION("Inserting 100 rows using the extension to generate a GUID")
    {
        auto insertRowWithGeneratedUuidFn = [&session]()
        {
            int id;
            soci::statement statement = (session->prepare <<
                "INSERT INTO test_table VALUES ("
                    ":val,"              // id
                    "uuid(),"            // guid
                    "NULL"               // guid_bytes
                ")", soci::use(id));
            for (id = 0; id != 100; ++id)
            {
                statement.execute(true);
            }
        };
        REQUIRE_NOTHROW(insertRowWithGeneratedUuidFn());

        SECTION("Generated GUID matches regex pattern")
        {
            soci::rowset<std::string> rowSet = (session->prepare << "SELECT guid from test_table");
            for( std::string & guidAsText : rowSet)
            {
                const std::regex guidRegularExpression("^([0-9a-fA-F]){8}-([0-9a-fA-F]){4}-([0-9a-fA-F]){4}-([0-9a-fA-F]){4}-([0-9a-fA-F]){12}");
                REQUIRE( std::regex_match(guidAsText, guidRegularExpression) );

                // time-low is 4 bytes
                // time_mid is 2 bytes
                // time_hi_and_version is 2 bytes
                // clock_seq_hi_and_reserved 1 byte
                //
                // Version is the most significant 4 bits of time_hi_and_version, and should be equal to 4
                // Variant is the most significant 3 bits of clock_seq_hi_and_reserved, and should be equal to 10x for variant 1,
                //    where the number of bits that are on is the variant and x can be anything.
                REQUIRE(guidAsText[14] == '4');

                std::string eighthOctetAsHex = guidAsText.substr(19,2);
                int eightByte;
                REQUIRE( !(std::istringstream(eighthOctetAsHex) >> std::hex >> eightByte).fail() );
                REQUIRE( (0xC0 & eightByte) == 0x80);
            }
        }
    }

    SECTION("Inserting valid GUID as Text")
    {

    }

    SECTION("Inserting invalid GUID as Text")
    {

    }

    SECTION("Inserting valid GUID as Blob")
    {

    }
    
    SECTION("Inserting invalid GUID as Blob")
    {

    }
}

