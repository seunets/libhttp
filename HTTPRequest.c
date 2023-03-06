#include <stdlib.h>
#include <string.h>
#include "includes/HTTPRequest.h"


static HTTPRequest_t * parse( HTTPRequest_t *request, char *message )
{
char *newLine, *strPtr;
size_t lineLen;

   if( ( newLine = strstr( message, "\r\n" ) ) != NULL )
   {
      lineLen = newLine - message;

      if( ( strPtr = strchr( message, ' ' ) ) != NULL )
      {
         if( ( request-> method = strndup( message, strPtr - message ) ) == NULL )
         {
            return NULL;
         }
         message = strPtr + 1;

         if( ( strPtr = strchr( message, ' ' ) ) != NULL )
         {
            if( ( request-> URI = strndup( message, strPtr - message ) ) == NULL )
            {
               return NULL;
            }
            message = strPtr + 1;
         }
         else
         {
            if( strcmp( request-> method, "GET" ) == 0 )
            {
               if( ( request-> URI = strdup( "/" ) ) == NULL )
               {
                  return NULL;
               }
            }
            else
            {
               return NULL;
            }
         }
         if( ( request-> version = strndup( message, newLine - message ) ) == NULL )
         {
            return NULL;
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
                     if( ( key = strndup( message, strPtr - message ) ) == NULL )
                     {
                        return NULL;
                     }
                     else
                     {
                        strPtr += 2;
                        if( ( value = strndup( strPtr, newLine - strPtr ) ) == NULL )
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
