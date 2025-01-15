#ifndef SQLITE_UUID_EXT_HPP
#define SQLITE_UUID_EXT_HPP

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
* Initializes the extension with sqlite
* 
*/
int sqlite3_uuid_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

#endif