#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct http_header
{
   char **keys;
   char **values;
   int ( *set )( struct http_header *, const char *, const char * );
   const char * ( *get )( const struct http_header *, const char * );
   void ( *parse )( struct http_header *, const char *, const char * );
   const char * ( *serialize )( const struct http_header * );
   void ( *clear )( struct http_header * );
   void ( *delete )( struct http_header * );
} HTTPHeader_t;

HTTPHeader_t *HTTPHeader_new( void );

#endif
