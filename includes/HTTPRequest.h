#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <arpa/inet.h>
#include "HTTPHeader.h"


typedef struct http_request
{
   char *method;
   char *URI;
   char *version;
   HTTPHeader_t *headers;
   char *body;
   char remoteAddr[ INET6_ADDRSTRLEN ];
   struct http_request * ( *parse )( struct http_request *, char * );
   void ( *delete )( const struct http_request * );
} HTTPRequest_t;

HTTPRequest_t *HTTPRequest_new( void );

#endif
