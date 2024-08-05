# About libhttp

libhttp is a library that aims to provide developers with the easiest possible way to incorporate HTTP client and server support into their applications on FreeBSD.

## DESCRIPTION

The HTTP_new function returns a pointer to an HTTP structure on success and NULL on error. The HTTP structure is defined in the <HTTP.h> include file, and contain at least the following fields:

```
typedef struct http
{
   Connection_t *connection;
   HTTPRequest_t *req;
   HTTPResponse_t *res;
   int ( *request )( struct http *, const char * );
   void ( *serve )( struct http *, const char *, void ( * )( struct http * ) );
   void ( *delete )( struct http * );
} HTTP_t;
```

These elements describe an HTTP type and a set of functions performing various actions. These functions take a pointer to a structure as returned by HTTP_new(), and sometimes one or more pointers to data/callback functions.

___request___ A routine that makes a request for a server. It takes a pointer for an http structure and a hostname.
___serve___ A routine that start a web server. It takes a pointer for an http structure, a listen name or address, and a callback routine to handle requests.
___delete___ A routine that free any resources associated with an HTTP structure.

## Usage examples

### Server
A minimal HTTP server can be written as:
```
#include "includes/HTTP.h"

static void cb( HTTP_t *http )
{
   http-> res-> setVersion( http-> res, "HTTP/1.0" );
   http-> res-> code = 200;
   http-> res-> setBody( http-> res, "It works!", 9 );
}

int main( void )
{
HTTP_t *http;

   http = HTTP_new();
   http-> serve( http, NULL, cb );
   http-> delete( http );
}
```

### Client
A minimal HTTP client can be written as:
```
#include "includes/HTTP.h"

int main( void )
{
HTTP_t *http;

   http = HTTP_new();
   http-> req = HTTPRequest_new();
   http-> req-> setVersion( http-> req, "HTTP/1.0" );
   http-> req-> setMethod( http-> req, "GET" );
   http-> req-> setURI( http-> req, "/" );
   http-> req-> headers = HTTPHeader_new();
   http-> req-> headers-> set( http-> req-> headers, "Host", "example.com" );
   http-> request( http, "example.com" );
   printf( "%.*s", http-> res-> bodySize, http-> res-> body );
   http-> delete( http );
}
```
