#ifndef MESSAGE_H
#define MESSAGE_H

#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 8192

typedef struct message
{
   size_t size;
   char *pdu;
   int ( *send )( const struct message *, const int );
   ssize_t ( *receive )( struct message *, const int );
   void ( *clear )( struct message * );
   void ( *delete )( struct message * );
} Message_t;

Message_t *Message_new( void );

#endif
