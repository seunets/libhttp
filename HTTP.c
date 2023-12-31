#include "includes/HTTP.h"


static void delete( HTTP_t *this )
{
   if( this-> connection != NULL )
   {
      this-> connection-> delete( this-> connection );
   }
   if( this-> req != NULL )
   {
      this-> req-> delete( this-> req );
   }
   if( this-> res != NULL )
   {
      this-> res-> delete( this-> res );
   }
   free( this );
}


void request( HTTP_t *this, const char *hostName )
{
   if( ( this-> connection != NULL || ( this-> connection = Connection_new( hostName, 80 ) ) != NULL ) )
   {
      if( this-> connection-> establish( this-> connection ) == 0 )
      {
         if( this-> connection-> message != NULL )
         {
            this-> connection-> message-> delete( this-> connection-> message );
         }
         if( ( this-> connection-> message = __DECONST( void *, this-> req-> serialize( this-> req ) ) ) != NULL )
         {
            if( this-> connection-> message-> send( this-> connection-> message, this-> connection-> socket ) == -1 )
            {
               this-> res = NULL;
               this-> connection-> message-> clear( this-> connection-> message );
            }
            else
            {
               this-> connection-> message-> clear( this-> connection-> message );
               this-> connection-> message-> receive( this-> connection-> message, this-> connection-> socket );

               if( this-> res != NULL )
               {
                  this-> res-> delete( this-> res );
               }
               if( ( this-> res = HTTPResponse_new() ) != NULL )
               {
                  this-> res = this-> res-> parse( this-> res, this-> connection-> message );
               }
            }
         }
      }
      else
      {
         this-> connection-> delete( this-> connection );
         this-> connection = NULL;
      }
   }
}


static void handleConnection( HTTP_t *this, void ( *callback )( HTTP_t *http ) )
{
   if( ( this-> req = HTTPRequest_new() ) != NULL )
   {
      if( this-> req-> parse( this-> req, this-> connection-> message ) != NULL )
      {
         if( ( this-> res = HTTPResponse_new() ) != NULL )
         {
            callback( this );

            this-> connection-> message-> delete( this-> connection-> message );
            if( ( this-> connection-> message = __DECONST( void *, this-> res-> serialize( this-> res ) ) ) != NULL )
            {
               this-> connection-> message-> send( this-> connection-> message, this-> connection-> socket );
               this-> connection-> message-> delete( this-> connection-> message );
               this-> connection-> message = NULL;
               // IF version 1.0 or Connection header = close
               this-> connection-> drop( this-> connection );
            }
            this-> res-> delete( this-> res );
            this-> res = NULL;
            this-> req-> delete( this-> req );
            this-> req = NULL;
         }
      }
      else
      {
         this-> connection-> drop( this-> connection );
      }
   }
   this-> connection-> drop( this-> connection );
}


static void serve( HTTP_t *this, const char *hostName, void ( callback )( HTTP_t * ) )
{
   if( ( this-> connection = Connection_new( hostName, 80 ) ) != NULL )
   {
      while( this-> connection-> accept( this-> connection ) == 0 && errno != EINTR )
      {
         handleConnection( this, callback );
      }
   }
}


HTTP_t *HTTP_new( void )
{
HTTP_t *this;

   if( ( this = calloc( 1, sizeof( HTTP_t ) ) ) != NULL )
   {
      this-> request = request;
      this-> serve = serve;
      this-> delete = delete;
   }

   return this;
}
