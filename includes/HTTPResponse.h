#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "HTTPHeader.h"

typedef struct http_response
{
   char *version;
   char *code;
   char *reason;
   HTTPHeader_t *headers;
   char *body;
   struct http_response * ( *parse )( struct http_response *, char * );
   void ( *delete )( const struct http_response * );
} HTTPResponse_t;

HTTPResponse_t *HTTPResponse_new( void );

#endif
