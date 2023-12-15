#ifndef HTTP_H
#define HTTP_H

#include "Connection.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

typedef struct http
{
   Connection_t *connection;
   struct http_response * ( *request )( struct http *, const char *, const struct http_request * );
   int ( *serve )( struct http *, const char *, const HTTPResponse_t * ( * )( const HTTPRequest_t * ) );
   void ( *delete )( struct http * );
} HTTP_t;

HTTP_t *HTTP_new( void );

#endif
