#include "includes/HTTPResponse.h"


static HTTPResponse_t * parse( HTTPResponse_t *this, Message_t *message )
{
char *newLine, *strPtr;

   if( ( newLine = memmem( message->pdu, message-> size, "\r\n", 2 ) ) != NULL && strncmp( message-> pdu, "HTTP/", 5 ) == 0 )
   {
      if( ( strPtr = memchr( message-> pdu, ' ', ( size_t )( newLine - message-> pdu ) ) ) != NULL )
      {
      char *p;

         *strPtr = '\0';
         this-> version = message-> pdu;

         p = strPtr + 1;

         if( ( strPtr = memchr( p, ' ', ( size_t )( newLine - p ) ) ) != NULL )
         {
            if( ( this-> code = ( unsigned int ) strtoul( p, NULL, 10 ) ) == 0 && errno == EINVAL )
            {
               return NULL;
            }

            p = strPtr + 1;
            *newLine = '\0';
            this-> reason = p;

            p = newLine + 2;
            newLine = memmem( p, ( size_t )( message-> pdu - p ), "\r\n\r\n", 4 );
            if( newLine - p > 4 )
            {
               this-> headers-> parse( this-> headers, p, newLine );
            }
         }
      }
   }

   this-> body = newLine + 4;

   return this;
}


static const char *statusToReason( unsigned int status )
{
   if( status == 200 ) return "OK";
   else if( status == 201 ) return "Created";
   else if( status == 204 ) return "No Content";
   else if( status == 400 ) return "Bad Request";
   else if( status == 401 ) return "Unauthorized";
   else if( status == 403 ) return "Forbidden";
   else if( status == 404 ) return "Not Found";
   else if( status == 500 ) return "Internal Server Error";
   else if( status == 501 ) return "Not Implemented";
   else return "UNKNOWN STATUS CODE";
}


//
// Builds a response message to be sent over the network
// Returns: the message or NULL on error
//
static Message_t *serialize( const HTTPResponse_t *this )
{
Message_t *msg = NULL;

   if( ( msg = Message_new() ) != NULL )
   {
   char *strHeaders = __DECONST( char *, this-> headers-> serialize( this-> headers ) );

      if( ( msg-> size = ( size_t ) asprintf( &msg-> pdu, "%s %u %s\r\n%s\r\n%s", this-> version, this-> code, statusToReason( this-> code ), strHeaders, this-> body ? this-> body : "" ) ) == ( size_t ) -1 )
      {
         msg-> delete( msg );
         msg = NULL;
      }
      free( strHeaders );
   }
   return msg;
}


static void delete( HTTPResponse_t *this )
{
   this-> headers-> delete( this-> headers );
   free( this );
}


HTTPResponse_t *HTTPResponse_new( void )
{
HTTPResponse_t *this;

   if( ( this = calloc( 1, sizeof( HTTPResponse_t ) ) ) != NULL )
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
         this = NULL;
      }
   }

   return this;
}
