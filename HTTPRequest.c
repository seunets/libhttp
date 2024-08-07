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
         if( ( this-> method = strdup( message-> pdu ) ) != NULL )
         {
            p = strPtr + 1;

            if( ( strPtr = memchr( p, ' ', ( size_t )( newLine - p ) ) ) != NULL )
            {
               *strPtr = '\0';
               if( ( this-> URI = strdup( p ) ) == NULL )
               {
                  return NULL;
               }
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
            if( ( this-> version = strdup( p ) ) != NULL )
            {
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
                  this-> bodySize = strtoull( contentLength, NULL, 10 );
                  if( ( this-> body = calloc( this-> bodySize + 1, sizeof( char ) ) ) == NULL )
                  {
                     return NULL;
                  }
                  memmove( this-> body, message-> pdu + message-> size - this-> bodySize, this-> bodySize );
               }
            }
            else
            {
               return NULL;
            }
         }
      }
      else
      {
         return NULL;
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

      if( ( msg-> size = ( size_t ) asprintf( &msg-> pdu, "%s %s %s\r\n%s\r\n", this-> method, this-> URI, this-> version, strHeaders ) ) != ( size_t ) -1 )
      {
      char *tmp;

         if( ( tmp = realloc( msg-> pdu, msg-> size + this-> bodySize ) ) != NULL )
         {
            msg-> pdu = tmp;
            memcpy( msg-> pdu + msg-> size, this-> body, this-> bodySize );
            msg-> size += this-> bodySize;
         }
         else
         {
            goto outerr;
         }
      }
      else
      {
         goto outerr;
      }
      free( strHeaders );
   }

out:
   return msg;

outerr:
   msg-> delete( msg );
   msg = NULL;
   goto out;
}


static char *setMethod( HTTPRequest_t *this, const char *method )
{
   free( this-> method );
   return this-> method = strdup( method );
}


static char *setVersion( HTTPRequest_t *this, const char *version )
{
   free( this-> version );
   return this-> version = strdup( version );
}


static char *setURI( HTTPRequest_t *this, const char *URI )
{
   free( this-> URI );
   return this-> URI = strdup( URI );
}


static char *setBody( HTTPRequest_t *this, const char *body, size_t size )
{
   free( this-> body );
   if( ( this-> body = malloc( size ) ) != NULL )
   {
      this-> bodySize = size;
      memcpy( this-> body, body, size );
   }
   else
   {
      this-> bodySize = 0;
   }
   return this-> body;
}


static void delete( HTTPRequest_t *this )
{
   this-> headers-> delete( this-> headers );
   free( this-> method );
   free( this-> version );
   free( this-> URI );
   free( this-> body );
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
         this-> setMethod = setMethod;
         this-> setVersion = setVersion;
         this-> setURI = setURI;
         this-> setBody = setBody;
         this-> delete = delete;
      }
      else
      {
         free( this );
         return NULL;
      }
   }

   return this;
}
