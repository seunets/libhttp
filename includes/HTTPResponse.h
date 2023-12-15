#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "Message.h"
#include "HTTPHeader.h"

typedef struct http_response
{
   char *version;
   unsigned int code;
   char pad[ 4 ];
   char *reason;
   HTTPHeader_t *headers;
   char *body;
   struct http_response * ( *parse )( struct http_response *, Message_t * );
   Message_t * ( *serialize )( const struct http_response * );
   void ( *delete )( struct http_response * );
} HTTPResponse_t;

HTTPResponse_t *HTTPResponse_new( void );

#endif
