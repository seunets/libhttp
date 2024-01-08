#include "includes/Message.h"
#include <stdio.h>


static void clear( Message_t *this )
{
   free( this-> pdu );
}


static void delete( Message_t *this )
{
   clear( this );
   free( this );
}


static int send_( const Message_t *this, int clientSocket )
{
   send( clientSocket, this-> pdu, this-> size, 0 );
   return errno;
}


static void *reassemble( char *message, char *fragment, size_t * messageLength, ssize_t fragmentSize )
{
char *tmp;

   if( ( tmp = realloc( message, ( size_t ) fragmentSize + *messageLength + 1 ) ) == NULL )
   {
      return NULL;
   }

   memmove( tmp + *messageLength, fragment, ( size_t ) fragmentSize );
   *messageLength += ( size_t ) fragmentSize;
   tmp[ *messageLength ] = '\0';
   return tmp;
}


static int receive( Message_t *this, int clientSocket )
{
char buffer[ BUFFERSIZE ];
ssize_t bytesRead;

   this-> size = 0;
   if( ( bytesRead = recv( clientSocket, buffer, BUFFERSIZE, 0 ) ) > 0 )
   {
      if( ( this-> pdu = calloc( 1, this-> size ) ) != NULL )
      {
      char *tmp;

         if( ( tmp = reassemble( this-> pdu, buffer, &this-> size, bytesRead ) ) != NULL )
         {
            this-> pdu = tmp;
            while( ( bytesRead = recv( clientSocket, buffer, BUFFERSIZE, MSG_DONTWAIT ) ) > 0 )
            {
               if( ( tmp = reassemble( this-> pdu, buffer, &this-> size, bytesRead ) ) != NULL )
               {
                  this-> pdu = tmp;
               }
               else
               {
                  free( this-> pdu );
                  break;
               }
            }
         }
         else
         {
            free( this-> pdu );
         }
      }
   }
   return errno;
}


Message_t *Message_new( void )
{
Message_t *this;

   if( ( this = calloc( 1, sizeof( Message_t ) ) ) != NULL )
   {
      this-> send = send_;
      this-> receive = receive;
      this-> clear = clear;
      this-> delete = delete;
   }

   return this;
}
