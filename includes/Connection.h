#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/event.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Message.h"
#include <unistd.h>

typedef struct connection
{
   char *peer;
   Message_t *message;
   int socket;
   int kq;
   struct kevent *event;
   int ( *accept )( struct connection * );
   int ( *establish )( struct connection * );
   void ( *delete )( struct connection * );
   void ( *drop )( struct connection * );
} Connection_t;

Connection_t *Connection_new( const char *, const int );

#endif
