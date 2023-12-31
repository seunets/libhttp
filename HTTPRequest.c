#include "includes/HTTPRequest.h"


static HTTPRequest_t * parse( HTTPRequest_t *this, Message_t *message )
{
char *newLine, *strPtr;

   if( ( newLine = memmem( message-> pdu, message-> size, "\r\n", 2 ) ) != NULL )
   {
      if( ( strPtr = memchr( message-> pdu, ' ', ( size_t )( newLine - message-> pdu ) ) ) != NULL )
      {
      char *p, *q;

         *strPtr = '\0';
         this-> method = message-> pdu;

         p = strPtr + 1;

         if( ( strPtr = memchr( p, ' ', ( size_t )( newLine - p ) ) ) != NULL )
         {
            *strPtr = '\0';
            this-> URI = p;
         }
         else
         {
            return NULL;
         }

         for( p = q = strchr( this-> URI, '%' ); p && *p; q = p )
         {
            if( isxdigit( p[ 1 ] ) && isxdigit( p[ 2 ] ) )
            {
               *q = ( char )( ( p[ 1 ] >= 'A' ? ( p[ 1 ] & 0xdf ) - 'A' + 10 : p[ 1 ] - '0' ) << 4 );
               *q += p[ 2 ] >= 'A' ? ( p[ 2 ] & 0xdf ) - 'A' + 10 : p[ 2 ] - '0';
            }
            p = strchr( q, '%' );
            p = p ? p : strchr( q, 0 );
            strcpy( q + 1, q + 3 );
            p -= 2;
         }

         p = strPtr + 1;
         if( strncmp( p, "HTTP/", 5 ) != 0 )
         {
            return NULL;
         }
         *newLine = '\0';
         this-> version = p;

         p = newLine + 2;
         newLine = memmem( p, ( size_t )( message-> pdu - p ) + message-> size, "\r\n\r\n", 4 );
         if( newLine - p > 4 )
         {
            this-> headers-> parse( this-> headers, p, newLine );
         }
         if( strcmp( this-> method, "GET" ) != 0 )
         {
         const char *contentLength = this-> headers-> get( this-> headers, "Content-Length" );

            if( contentLength == NULL )
            {
               return NULL;
            }
            this-> body = message-> pdu + message-> size - strtoull( contentLength, NULL, 10 );
         }
      }
      else
      {
         if( strcmp( this-> method, "GET" ) != 0 )
         {
            return NULL;
         }
      }
   }
   else
   {
      return NULL;
   }

   return this;
}


static const Message_t *serialize( const HTTPRequest_t *this )
{
Message_t *msg;

   if( ( msg = Message_new() ) != NULL )
   {
   char *strHeaders = __DECONST( char *, this-> headers-> serialize( this-> headers ) );

      if( ( msg-> size = ( size_t ) asprintf( &msg-> pdu, "%s %s %s\r\n%s\r\n%s", this-> method, this-> URI, this-> version, strHeaders, this-> body ? this-> body : "" ) ) == ( size_t ) -1 )
      {
         msg-> delete( msg );
         msg = NULL;
      }
      free( strHeaders );
   }
   return msg;
}


static void delete( HTTPRequest_t *this )
{
   this-> headers-> delete( this-> headers );
   free( this );
}


HTTPRequest_t *HTTPRequest_new( void )
{
HTTPRequest_t *this;

   if( ( this = calloc( 1, sizeof( HTTPRequest_t ) ) ) != NULL )
   {
      if( ( this-> headers = HTTPHeader_new() ) != NULL )
      {
         this-> parse = parse;
         this-> serialize = serialize;
         this-> delete = delete;
      }
      else
      {
         free( this );
      }
   }

   return this;
}
