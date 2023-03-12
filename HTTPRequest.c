#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "includes/HTTPRequest.h"


static HTTPRequest_t * parse( HTTPRequest_t *request, char *message )
{
char *newLine, *strPtr, *p, *q;

   if( ( newLine = strstr( message, "\r\n" ) ) != NULL )
   {
      if( ( strPtr = strchr( message, ' ' ) ) != NULL )
      {
         if( ( request-> method = strndup( message, ( size_t )( strPtr - message ) ) ) == NULL )
         {
            return NULL;
         }
         message = strPtr + 1;

         strPtr = strchr( message, ' ' );
         strPtr = strPtr ? strPtr : newLine;

         if( ( request-> URI = strndup( message, ( size_t )( strPtr - message ) ) ) == NULL )
         {
            return NULL;
         }

         for( p = q = strchr( request-> URI, '%' ); p && *p; q = p )
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

         if( strPtr == newLine && strcmp( request-> method, "GET" ) != 0 )
         {
            return NULL;
         }
         else
         {
            message = strPtr + 1;

            if( *message != '\n' && ( request-> version = strndup( message, ( size_t )( newLine - message ) ) ) == NULL )
            {
               return NULL;
            }
         }
         message = newLine + 2;
         newLine = strstr( message, "\r\n" );
         if( newLine - message > 2 )
         {
         char *headersEnd;

            if( ( headersEnd = strstr( message, "\r\n\r\n" ) ) != NULL )
            {
               for( ; message < headersEnd; newLine = strstr( message, "\r\n" ) )
               {
               char *key, *value;

                  if( ( strPtr = strchr( message, ':' ) ) != NULL )
                  {
                     if( ( key = strndup( message, ( size_t )( strPtr - message ) ) ) == NULL )
                     {
                        return NULL;
                     }
                     else
                     {
                        strPtr += 2;
                        if( ( value = strndup( strPtr, ( size_t )( newLine - strPtr ) ) ) == NULL )
                        {
                           free( key );
                           return NULL;
                        }
                        else
                        {
                           request-> headers-> set( request-> headers, key, value );
                           free( value );
                           free( key );
                        }
                     }
                  }
                  message = newLine + 2;
               }
            }
            else
            {
               return NULL;
            }
         }
         message += 2;
         if( strcmp( request-> method, "GET" ) != 0 && *message && ( request-> body = strdup( message ) ) == NULL )
         {
            return NULL;
         }
      }
   }
   else
   {
      return NULL;
   }

   return request;
}


static void delete( const HTTPRequest_t *request )
{
   free( request-> method );
   free( request-> URI );
   free( request-> version );
   request-> headers-> delete( request-> headers );
   free( request-> body );
   free( __DECONST( void *, request ) );
}


HTTPRequest_t *HTTPRequest_new( void )
{
HTTPRequest_t *request;

   if( ( request = calloc( 1, sizeof( HTTPRequest_t ) ) ) != NULL )
   {
      request-> headers = HTTPHeader_new();
      request-> parse = parse;
      request-> delete = delete;
   }
   else
   {
      free( request );
      request = NULL;
   }

   return request;
}
