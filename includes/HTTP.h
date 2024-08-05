#ifndef HTTP_H
#define HTTP_H

#include "Connection.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

typedef struct http
{
   Connection_t *connection;
   HTTPRequest_t *req;
   HTTPResponse_t *res;
   int ( *request )( struct http *, const char * );
   void ( *serve )( struct http *, const char *, void ( * )( struct http * ) );
   void ( *delete )( struct http * );
} HTTP_t;

HTTP_t *HTTP_new( void );

#endif
