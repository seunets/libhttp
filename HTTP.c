#include "includes/HTTP.h"


static void delete( HTTP_t *this )
{
   if( this-> connection != NULL )
   {
      this-> connection-> delete( this-> connection );
   }
   free( this );
}


static HTTPResponse_t * request( HTTP_t *this, const char *hostName, const HTTPRequest_t *req )
{
HTTPResponse_t *res;

   if( ( this-> connection != 0 || ( this-> connection = Connection_new( hostName, 80 ) ) != NULL ) )
   {
      if( this-> connection-> establish( this-> connection ) == 0 )
      {
         if( ( this-> connection-> message = __DECONST( void *, req-> serialize( req ) ) ) != NULL )
         {
            if( this-> connection-> message-> send( this-> connection-> message, this-> connection-> socket ) == -1 )
            {
               this-> connection-> message-> delete( this-> connection-> message );
               return NULL;
            }
         }
      }
   }
   this-> connection-> message-> delete( this-> connection-> message );

   res = HTTPResponse_new();

   this-> connection-> message-> receive( this-> connection-> message, this-> connection-> socket );

   return res-> parse( res, this-> connection-> message );
}


static void handleConnection( HTTP_t *this, const HTTPResponse_t * ( *callback )( const HTTPRequest_t *req ) )
{
HTTPResponse_t *res;
HTTPRequest_t *req;

   if( ( req = HTTPRequest_new() ) == NULL )
   {
      errno = ENOMEM;
   }
   else
   {
      if( req-> parse( req, this-> connection-> message ) == NULL )
      {
         errno = EINVAL;
      }
      else
      {
         res = __DECONST( HTTPResponse_t *, callback( req ) );

         this-> connection-> message-> delete( this-> connection-> message );
         this-> connection-> message = res-> serialize( res );

         this-> connection-> message-> send( this-> connection-> message, this-> connection-> socket );
         res-> delete( res );
         this-> connection-> message-> delete( this-> connection-> message );
         this-> connection-> message = NULL;
         req-> delete( req );
      }
   }
}


static int serve( HTTP_t *this, const char *hostName, const HTTPResponse_t * ( callback )( const HTTPRequest_t * ) )
{
   if( ( this-> connection = Connection_new( hostName, 80 ) ) != NULL )
   {
      while( this-> connection-> accept( this-> connection ) == 0 && errno != EINTR )
      {
         handleConnection( this, callback );
      }
   }

   return errno;
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
