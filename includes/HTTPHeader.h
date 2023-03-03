#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

typedef struct http_header
{
   char **keys;
   char **values;
   int ( *set )( struct http_header *, const char *, const char * );
   const char * ( *get )( const struct http_header *, const char * );
   void ( *delete )( const struct http_header * );
} HTTPHeader_t;

HTTPHeader_t *HTTPHeader_new( void );

#endif
