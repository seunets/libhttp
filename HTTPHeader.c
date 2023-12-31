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


static int reallocHeaders( HTTPHeader_t *this, long int size )
{
int rv = -1;
long int i;
char **p;

   for( i = 0; this-> keys[ i ] != NULL; i++ );

   if( ( p = reallocStringArray( this-> keys, size ) ) != NULL )
   {
      this-> keys = p;
      if( ( p = reallocStringArray( this-> values, size ) ) != NULL )
      {
         this-> values = p;
         this-> keys[ size - 1 ] = NULL;
         this-> values[ size - 1 ] = NULL;
         rv = 0;
      }
      else
      {
         reallocStringArray( this-> keys, i );
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


static int set( HTTPHeader_t *this, const char *key, const char *value )
{
int rv = -1;
long int i;
char *tmp;

   if( ( i = find( this-> keys, key ) ) == -1 )
   {
      for( i = 0; this-> keys[ i ] != NULL; i++ );

      if( reallocHeaders( this, i + 2 ) == 0 )
      {
         if( ( this-> keys[ i ] = strdup( key ) ) != NULL )
         {
            if( ( this-> values[ i ] = strdup( value ) ) != NULL )
            {
               rv = 0;
            }
            else
            {
               reallocHeaders( this, i - 1 );
            }
         }
         else
         {
            reallocHeaders( this, i - 1 );
         }
      }
   }
   else
   {
      if( asprintf( &tmp, "%s, %s", this-> values[ i ], value ) != -1 )
      {
      char *tmp2 = this-> values[ i ];

         if( ( this-> values[ i ] = strdup( tmp ) ) == NULL )
         {
            this-> values[ i ] = tmp2;
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


static const char * get( const HTTPHeader_t *this, const char *key )
{
const char *rv = NULL;
long int i = find( this-> keys, key );

   if( i != -1 )
   {
      rv = this-> values[ i ];
   }

   return rv;
}


static void clear( HTTPHeader_t *this )
{
   for( long int i = 0; this-> keys != NULL && this-> keys[ i ] != NULL; i++ )
   {
      free( this-> keys[ i ] );
      free( this-> values[ i ] );
   }
   reallocHeaders( this, 1 );
}


static void delete( HTTPHeader_t *this )
{
   clear( this );
   free( this-> keys );
   free( this-> values );
   free( this );
}


static void parse( HTTPHeader_t *this, const char *start, const char *end )
{
char *p = __DECONST( char *, start );

   if( end != NULL )
   {
      while( p < end )
      {
      char *newLine, *sepPtr, *key, *value;

         newLine = memmem( p, ( size_t )( end - p ), "\r\n", 2 );
         newLine = newLine ? newLine : __DECONST( char *, end );

         if( ( sepPtr = memchr( p, ':', ( size_t )( newLine - p ) ) ) != NULL )
         {
            *sepPtr = '\0';
            key = p;

            sepPtr += 2;
            *newLine = '\0';
            value = sepPtr;

            set( this, key, value );
         }
         p = newLine + 2;
      }
   }
}


static const char * serialize( const HTTPHeader_t *this )
{
char *tmp1 = NULL, *tmp2 = NULL, *tmp3 = NULL;

   for( long int i = 0; this-> keys[ i ]; i++ )
   {
      if( asprintf( &tmp1, "%s: %s\r\n", this-> keys[ i ], this-> values[ i ] ) != -1 )
      {
         if( asprintf( &tmp2, "%s%s", tmp3 ? tmp3 : "", tmp1 ) != -1 )
         {
            free( tmp1 );
            free( tmp3 );
            tmp3 = strdup( tmp2 );
            free( tmp2 );
         }
      }
   }
   return tmp3 ? tmp3 : calloc( 1, sizeof( char ) );
}


HTTPHeader_t *HTTPHeader_new( void )
{
HTTPHeader_t *this;

   if( ( this = calloc( 1, sizeof( HTTPHeader_t ) ) ) != NULL )
   {
      if( ( this-> keys = calloc( 1, sizeof( char ** ) ) ) != NULL )
      {
         if( ( this-> values = calloc( 1, sizeof( char ** ) ) ) != NULL )
         {
            this-> set = set;
            this-> get = get;
            this-> parse = parse;
            this-> serialize = serialize;
            this-> clear = clear;
            this-> delete = delete;
         }
         else
         {
            free( this-> keys );
            goto outerr;
         }
      }
      else
      {
         goto outerr;
      }
   }

out:
   return this;

outerr:
   free( this );
   this = NULL;
   goto out;
}
