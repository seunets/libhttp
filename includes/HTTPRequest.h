#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <arpa/inet.h>
#include <ctype.h>
#include "HTTPHeader.h"
#include "Message.h"

typedef struct http_request
{
   char *method;
   char *URI;
   char *version;
   HTTPHeader_t *headers;
   char *body;
   unsigned long bodySize;
   char remoteAddr[ INET6_ADDRSTRLEN ];
   char pad[ 2 ];
   struct http_request * ( *parse )( struct http_request *, Message_t * );
   const Message_t * ( *serialize )( const struct http_request * );
   void ( *delete )( struct http_request * );
} HTTPRequest_t;

HTTPRequest_t *HTTPRequest_new( void );

#endif
