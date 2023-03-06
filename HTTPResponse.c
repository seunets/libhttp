#include <stdlib.h>
#include <string.h>
#include "includes/HTTPResponse.h"


static HTTPResponse_t * parse( HTTPResponse_t *response, char *message )
{
char *newLine, *strPtr;

   if( ( newLine = strstr( message, "\r\n" ) ) != NULL )
   {
      if( ( strPtr = strchr( message, ' ' ) ) != NULL )
      {
         if( ( response-> version = strndup( message, ( size_t )( strPtr - message ) ) ) == NULL )
         {
            return NULL;
         }
         message = strPtr + 1;

         if( ( strPtr = strchr( message, ' ' ) ) != NULL )
         {
            if( ( response-> code = strndup( message, ( size_t )( strPtr - message ) ) ) == NULL )
            {
               return NULL;
            }
            message = strPtr + 1;
            if( ( response-> reason = strndup( message, ( size_t )( newLine - message ) ) ) == NULL )
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
                              response-> headers-> set( response-> headers, key, value );
                              free( value );
                              free( key );
                           }
                        }
                     }
                     message = newLine + 2;
                  }
               }
            }
            else
            {
               return NULL;
            }
         }
         message += 2;
         if( *message && ( response-> body = strdup( message ) ) == NULL )
         {
            return NULL;
         }
      }
   }
   else
   {
      return NULL;
   }

   return response;
}


static void delete( const HTTPResponse_t *response )
{
   free( response-> version );
   free( response-> code );
   free( response-> reason );
   response-> headers-> delete( response-> headers );
   free( response-> body );
   free( __DECONST( void *, response ) );
}


HTTPResponse_t *HTTPResponse_new( void )
{
HTTPResponse_t *response;

   if( ( response = calloc( 1, sizeof( HTTPResponse_t ) ) ) != NULL )
   {
      response-> headers = HTTPHeader_new();
      response-> parse = parse;
      response-> delete = delete;
   }
   else
   {
      free( response );
      response = NULL;
   }

   return response;
}
