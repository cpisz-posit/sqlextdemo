/*
** 2019-10-23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This SQLite extension implements functions that handling RFC-4122 UUIDs
** Three SQL functions are implemented:
**
**     uuid()        - generate a version 4 UUID as a string
**     uuid_str(X)   - convert a UUID X into a well-formed UUID string
**     uuid_blob(X)  - convert a UUID X into a 16-byte blob
**
** The output from uuid() and uuid_str(X) are always well-formed RFC-4122
** UUID strings in this format:
**
**        xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
**
** All of the 'x', 'M', and 'N' values are lower-case hexadecimal digits.
** The M digit indicates the "version".  For uuid()-generated UUIDs, the
** version is always "4" (a random UUID).  The upper three bits of N digit
** are the "variant".  This library only supports variant 1 (indicated
** by values of N between '8' and 'b') as those are overwhelming the most
** common.  Other variants are for legacy compatibility only.
**
** The output of uuid_blob(X) is always a 16-byte blob.  The UUID input
** string is converted in network byte order (big-endian) in accordance
** with RFC-4122 specifications for variant-1 UUIDs.  Note that network
** byte order is *always* used, even if the input self-identifies as a
** variant-2 UUID.
**
** The input X to the uuid_str() and uuid_blob() functions can be either
** a string or a BLOB.  If it is a BLOB it must be exactly 16 bytes in
** length or else a NULL is returned.  If the input is a string it must
** consist of 32 hexadecimal digits, upper or lower case, optionally
** surrounded by {...} and with optional "-" characters interposed in the
** middle.  The flexibility of input is inspired by the PostgreSQL
** implementation of UUID functions that accept in all of the following
** formats:
**
**     A0EEBC99-9C0B-4EF8-BB6D-6BB9BD380A11
**     {a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11}
**     a0eebc999c0b4ef8bb6d6bb9bd380a11
**     a0ee-bc99-9c0b-4ef8-bb6d-6bb9-bd38-0a11
**     {a0eebc99-9c0b4ef8-bb6d6bb9-bd380a11}
**
** If any of the above inputs are passed into uuid_str(), the output will
** always be in the canonical RFC-4122 format:
**
**     a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11
**
** If the X input string has too few or too many digits or contains
** stray characters other than {, }, or -, then NULL is returned.
*/
#include "sqlite_extensions/uuidext.hpp"

#include <cassert>
#include <cstring>
#include <cctype>

#if !defined(SQLITE_ASCII) && !defined(SQLITE_EBCDIC)
# define SQLITE_ASCII 1
#endif


static const char * ERR_MSG_MALFORMED = "UUID input param was malformed";


/*
* Translates an single byte of an integer to hex.
* This routine only works if byte really is a valid hexadecimal character:  0..9a..fA..F
*/
static unsigned char sqlite3UuidHexToInt(int byte)
{
    assert( (byte>='0' && byte<='9') ||  (byte>='a' && byte<='f') ||  (byte>='A' && byte<='F') );
#ifdef SQLITE_ASCII
    byte += 9*(1&(byte>>6));
#endif
#ifdef SQLITE_EBCDIC
    byte += 9*(1&~(byte>>4));
#endif
    return (unsigned char)(byte & 0xf);
}

/*
* Converts a 16-byte BLOB into a well-formed RFC-4122 UUID with 8-4-4-4-12 hexidecimal digits, each representing 4 bits.
* The output buffer should be at least 37 bytes in length and will be zero terminted.
*/
static void sqlite3UuidBlobToStr(const unsigned char * bytes, unsigned char * result)
{
    static const char digits[] = "0123456789abcdef";

    for(int byteIndex = 0, pattern = 0x550; byteIndex < 16; byteIndex++, pattern = pattern>>1)
    {
        if( pattern & 1 )
        {
            result[0] = '-';
            result++;
        }

        unsigned byteValue = bytes[byteIndex];
        result[0] = digits[byteValue>>4];
        result[1] = digits[byteValue & 0xf];
        result += 2;
    }

    *result = 0;
}

/*
** Attempt to parse a zero-terminated input string zStr into a binary
** UUID.  Return 0 on success, or non-zero if the input string is not
** parsable.
*/
static int sqlite3UuidStrToBlob(const unsigned char *zStr,   /* Input string */
                                unsigned char *aBlob         /* Write results here */)
{
  int i;
  
  if( zStr[0]=='{' )
    zStr++;

  for(i=0; i<16; i++)
  {
    if( zStr[0]=='-' )
        zStr++;
    if( isxdigit(zStr[0]) && isxdigit(zStr[1]) )
    {
        aBlob[i] = (sqlite3UuidHexToInt(zStr[0])<<4) + sqlite3UuidHexToInt(zStr[1]);
        zStr += 2;
    }
    else
    {
      return 1;
    }
  }
  if( zStr[0]=='}' ) zStr++;
  return zStr[0]!=0;
}

/*
* Convert a sqlite3_value to a a 16-byte UUID blob.
* Sets out pointer to the blob or nullptr if the input is not well-formed.
*/
static void sqlite3UuidInputToBlob(sqlite3_value * value, unsigned char * out)
{
    switch( sqlite3_value_type(value) )
    {
        case SQLITE_TEXT: 
        {
            const unsigned char * text = sqlite3_value_text(value);
            if( sqlite3UuidStrToBlob(text, out) != 0 )
                out = nullptr;
        }
        case SQLITE_BLOB: 
        {
            if( sqlite3_value_bytes(value) == 16 )
            {
                const unsigned char * bytes = reinterpret_cast<const unsigned char *>(sqlite3_value_blob(value));
                memcpy(out, bytes, 16);
            }
            else
            {
                out = nullptr;
            }
        }
        default: 
        {
            out = nullptr;
        }
    }
}

/* 
* Implementation of uuid() sql function we are adding to sqlite
*/
static void sqlite3UuidFunc(sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    unsigned char bytes[16];
    unsigned char text[37];
    (void)argc;
    (void)argv;
    
    sqlite3_randomness(16, bytes);
    bytes[6] = (bytes[6]&0x0f) + 0x40; // set the first nibble of the 6th byte to 4 for the version of uuid
    bytes[8] = (bytes[8]&0x3f) + 0x80; // set the first two bits of the 8th byte to 2 for the variant

    sqlite3UuidBlobToStr(bytes, text);
    sqlite3_result_text(context, reinterpret_cast<char *>(text), 36, SQLITE_TRANSIENT);
}

/* 
* Implementation of the SQL function uuid_str()
*/
static void sqlite3UuidStrFunc(sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    unsigned char bytes[16];
    unsigned char text[37];
    (void)argc;
    
    sqlite3UuidInputToBlob(argv[0], bytes);
    
    if( bytes == nullptr )
    {
        sqlite3_result_error(context, ERR_MSG_MALFORMED, sizeof(ERR_MSG_MALFORMED));
        return;
    }

    sqlite3UuidBlobToStr(bytes, text);
    sqlite3_result_text(context, reinterpret_cast<char *>(text), 36, SQLITE_TRANSIENT);
}

/* 
* Implementation of the SQL function uuid_blob() 
*/
static void sqlite3UuidBlobFunc(sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    unsigned char bytes[16];
    (void)argc;

    sqlite3UuidInputToBlob(argv[0], bytes);
   
    if( bytes == nullptr )
    {
        sqlite3_result_error(context, ERR_MSG_MALFORMED, sizeof(ERR_MSG_MALFORMED));
        return;
    }

    sqlite3_result_blob(context, bytes, 16, SQLITE_TRANSIENT);
}


/*
* Call this to register the extension with sqlite before using it
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_uuid_init(sqlite3 * db, char ** pzErrMsg, const sqlite3_api_routines * pApi)
{
    int returnCode = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);
    (void)pzErrMsg;

    returnCode = sqlite3_create_function(db, "uuid", 0, SQLITE_UTF8|SQLITE_INNOCUOUS, 0, sqlite3UuidFunc, 0, 0);
    
    if( returnCode == SQLITE_OK )
    {
        returnCode = sqlite3_create_function(db, "uuid_str", 1, SQLITE_UTF8|SQLITE_INNOCUOUS|SQLITE_DETERMINISTIC, 0, sqlite3UuidStrFunc, 0, 0);
    }
    
    if( returnCode == SQLITE_OK )
    {
        returnCode = sqlite3_create_function(db, "uuid_blob", 1, SQLITE_UTF8|SQLITE_INNOCUOUS|SQLITE_DETERMINISTIC, 0, sqlite3UuidBlobFunc, 0, 0);
    }

    return returnCode;
}
