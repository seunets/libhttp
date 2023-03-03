#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "includes/HTTPHeader.h"


static char **reallocStringArray( char **strArr, long int size )
{
char **rv;

   if( ( rv = realloc( strArr, ( unsigned long ) size * sizeof( char ** ) ) ) != NULL )
   {
      rv[ size - 1 ] = NULL;
   }

   return rv;
}


static int reallocHeaders( HTTPHeader_t *header, long int size )
{
int rv = -1;
long int i;
char **p;

   for( i = 0; header-> keys[ i ] != NULL; i++ );

   if( ( p = reallocStringArray( header-> keys, size ) ) != NULL )
   {
      header-> keys = p;
      if( ( p = reallocStringArray( header-> values, size ) ) != NULL )
      {
         header-> values = p;
         header-> keys[ size - 1 ] = NULL;
         header-> values[ size - 1 ] = NULL;
         rv = 0;
      }
      else
      {
         reallocStringArray( header-> keys, i );
      }
   }
   return rv;
}


static long int find( char **keys, const char *value )
{
long int i;

   for( i = 0; keys[ i ] != NULL && strcmp( keys[ i ], value ) != 0; i++ );

   return keys[ i ] == NULL ? -1 : i;
}


static int set( HTTPHeader_t *header, const char *key, const char *value )
{
int rv = -1;
long int i;
char *tmp;

   if( ( i = find( header-> keys, key ) ) == -1 )
   {
      for( i = 0; header-> keys[ i ] != NULL; i++ );

      if( reallocHeaders( header, i + 2 ) == 0 )
      {
         if( ( header-> keys[ i ] = strdup( key ) ) != NULL )
         {
            if( ( header-> values[ i ] = strdup( value ) ) != NULL )
            {
               rv = 0;
            }
            else
            {
               reallocHeaders( header, i - 1 );
            }
         }
         else
         {
            reallocHeaders( header, i - 1 );
         }
      }
   }
   else
   {
      if( asprintf( &tmp, "%s, %s", header-> values[ i ], value ) != -1 )
      {
      char *tmp2 = header-> values[ i ];

         if( ( header-> values[ i ] = strdup( tmp ) ) == NULL )
         {
            header-> values[ i ] = tmp2;
         }
         else
         {
            free( tmp2 );
            free( tmp );
            rv = 0;
         }
      }
   }

   return rv;
}


static const char * get( const HTTPHeader_t *header, const char *key )
{
const char *rv = NULL;
long int i = find( header-> keys, key );

   if( i != -1 )
   {
      rv = header-> values[ i ];
   }

   return rv;
}


static void delete( const HTTPHeader_t *header )
{
   for( long int i = 0; header-> keys != NULL && header-> keys[ i ] != NULL; i++ )
   {
      free( header-> keys[ i ] );
      free( header-> values[ i ] );
   }
   free( header-> keys );
   free( header-> values );
   free( __DECONST( void *, header ) );
}


HTTPHeader_t *HTTPHeader_new( void )
{
HTTPHeader_t *header;

   if( ( header = calloc( 1, sizeof( HTTPHeader_t ) ) ) != NULL )
   {
      if( ( header-> keys = calloc( 1, sizeof( char ** ) ) ) != NULL )
      {
         if( ( header-> values = calloc( 1, sizeof( char ** ) ) ) != NULL )
         {
            header-> set = set;
            header-> get = get;
            header-> delete = delete;
         }
         else
         {
            free( header-> keys );
            goto outerr;
         }
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

out:
   return header;

outerr:
   free( header );
   header = NULL;
   goto out;
}
