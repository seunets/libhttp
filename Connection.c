#include "includes/Connection.h"


static const char *serverName;
static int serverPort;
static int *serverSocket = NULL;
static unsigned int eventListSize = 0;
static int isListening = 0;
static struct sigaction oldint, oldterm;


static void cleanup( int signo )
{
   if( signo == SIGTERM && oldterm.sa_handler ) oldterm.sa_handler( signo );
   else if( signo == SIGINT && oldint.sa_handler ) oldint.sa_handler( signo );
   free( serverSocket );
   isListening = 0;
}


static void drop( Connection_t *this )
{
   close( this-> socket );
}

static void delete( Connection_t *this )
{
   if( this-> peer != NULL )
   {
      free( __DECONST( void *, this-> peer ) );
   }
   if( this-> message != NULL )
   {
      this-> message-> delete( this-> message );
   }
   if( this-> event != NULL )
   {
      free( this-> event );
   }
   free( this );
}


static struct addrinfo *resolve( const char *host, const int port )
{
struct addrinfo hints, *res = NULL;
char *sport;

   memset( &hints, 0, sizeof hints );
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;

   if( asprintf( &sport, "%u", port ) >= 0 )
   {
      errno = getaddrinfo( host, sport, &hints, &res );
      free( sport );
   }
   return res;
}


static void setupSignals( void )
{
struct sigaction act, oact;

   sigemptyset( &act.sa_mask );
   act.sa_flags = 0;
   act.sa_handler = cleanup;
   if( sigaction( SIGINT, &act, &oact ) == -1 )
   {
      exit( EXIT_FAILURE );
   }
   oldint = oact;

   if( sigaction( SIGTERM, &act, &oact ) == -1 )
   {
      exit( EXIT_FAILURE );
   }
   oldterm = oact;
}


static char *socket2addr( int s )
{
struct sockaddr_storage addr;
socklen_t addrLen = sizeof( addr );
char peer[ INET6_ADDRSTRLEN ]; 

   if( getpeername( s, ( struct sockaddr * ) &addr, &addrLen ) != -1 )
   {
      if( addr.ss_family == AF_INET )
      {
         inet_ntop( addr.ss_family, &( ( struct sockaddr_in * ) ( &addr ) )-> sin_addr, peer, sizeof peer );
      }
      else
      {
         inet_ntop( addr.ss_family, &( ( struct sockaddr_in6 * ) ( &addr ) )-> sin6_addr, peer, sizeof peer );
      }
   }

   return strdup( peer );
}


static struct kevent *eventListAdd( Connection_t *this, int sockfd )
{
struct kevent *tmp;

   if( ( tmp = realloc( this-> event, sizeof( struct kevent ) * ( eventListSize + 1 ) ) ) != NULL )
   {
      this-> event = tmp;
      EV_SET( &this-> event[ eventListSize ], sockfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL );
      eventListSize++;
   }

   return tmp;
}


static struct kevent *eventListRemove( Connection_t *this )
{
struct kevent *tmp;

   if( ( tmp = realloc( this-> event, sizeof( struct kevent ) * eventListSize - 1 ) ) != NULL )
   {
      this-> event = tmp;
      eventListSize--;
   }

   return tmp;
}


static int acceptor( Connection_t *this )
{
int j, n;

   if( isListening == 0 )
   {
   struct addrinfo *res, *resp;
   int var;
   unsigned long nsock = 0;

      if( ( res = resolve( serverName, serverPort ) ) == NULL )
      {
         goto outerr;
      }

      if( ( serverSocket = calloc( 1, sizeof( int ) ) ) == NULL )
      {
         freeaddrinfo( res );
         goto outerr;
      }

      for( resp = res, var = 1; resp != NULL; resp = resp-> ai_next, nsock++ )
      {
      void *tmp;

         if( ( tmp = realloc( serverSocket, sizeof( int ) * ( nsock + 2 ) ) ) == NULL )
         {
            freeaddrinfo( res );
            goto outerr;
         }
         else
         {
            serverSocket = tmp;
         }

         if( ( serverSocket[ nsock ] = socket( resp-> ai_family, resp-> ai_socktype, resp-> ai_protocol ) ) == -1 )
         {
            continue;
         }
         serverSocket[ nsock + 1 ] = 0;

         if( setsockopt( serverSocket[ nsock ], SOL_SOCKET, SO_REUSEADDR, &var, sizeof( var ) ) == -1 )
         {
            freeaddrinfo( res );
            goto outerr;
         }

         if( bind( serverSocket[ nsock ], resp-> ai_addr, resp-> ai_addrlen ) == -1 )
         {
            freeaddrinfo( res );
            goto outerr;
         }

         if( listen( serverSocket[ nsock ], -1 ) == -1 )
         {
            freeaddrinfo( res );
            goto outerr;
         }
         isListening = 1;
      }
      freeaddrinfo( res );

      if( ( this-> kq = kqueue() ) == -1 )
      {
         goto outerr;
      }

      if( ( this-> event = malloc( 0 ) ) == NULL )
      {
         goto outerr;
      }

      for( n = 0; serverSocket[ n ]; n++ )
      {
         if( eventListAdd( this, serverSocket[ n ] ) == NULL )
         {
            goto outerr;
         }
      }

      if( kevent( this-> kq, this-> event, n, NULL, 0, NULL ) == -1 )
      {
         goto outerr;
      }

      setupSignals();
   }

   while( isListening && ( ( j = kevent( this-> kq, NULL, 0, this-> event, ( int ) eventListSize, NULL ) ) != -1 || errno == EINTR || errno == ECONNABORTED || errno == ECONNRESET ) )
   {
      for( int i = 0; i < j; i++ )
      {
      int sockfd = ( int ) this-> event[ i ].ident;

         if( this-> event[ i ].flags & EV_EOF )
         {
            free( this-> peer );
            this-> peer = NULL;
            if( eventListRemove( this ) == NULL )
            {
               goto outerr;
            }
            if( close( sockfd ) == -1 && errno != ECONNRESET )
            {
               goto outerr;
            }
            break;
         }
         else
         {
         int l;

            for( l = 0; serverSocket[ l ] && serverSocket[ l ] != sockfd; l++ );

            if( serverSocket[ l ] )
            {
            int tmp;

               if( ( tmp = accept( sockfd, NULL, NULL ) ) == -1 )
               {
                  if( errno == ECONNABORTED )
                  {
                     continue;
                  }
                  if( close( sockfd ) == -1 && errno != ECONNRESET )
                  {
                     goto outerr;
                  }
               }

               if( eventListAdd( this, tmp ) == NULL )
               {
                  goto outerr;
               }

               this-> socket = tmp;
               if( this-> peer != NULL )
               {
                  free( this-> peer );
               }
               this-> peer = socket2addr( tmp );
               if( kevent( this-> kq, &this-> event[ eventListSize - 1 ], 1, NULL, 0, NULL ) == -1 )
               {
                  goto outerr;
               }
            }
            else
            {
               if( this-> event[ i ].filter & EVFILT_READ )
               {
                  if( ( this-> message = Message_new() ) == NULL )
                  {
                     goto outerr;
                  }
                  this-> socket = sockfd;
                  if( this-> message-> receive( this-> message, sockfd ) == EAGAIN )
                  {
                     errno = 0;
                     goto out;
                  }
               }
            }
         }
      }
   }

out:
   return errno;

outerr:
   this = NULL;
   goto out;
}


static int establish( Connection_t *this )
{
struct addrinfo *res, *resp;

   if( ( res = resolve( serverName, serverPort ) ) != NULL )
   {
      for( resp = res; resp != NULL; resp = resp-> ai_next )
      {
         if( ( this-> socket = socket( resp-> ai_family, resp-> ai_socktype, resp-> ai_protocol ) ) == -1 )
         {
            continue;
         }

         if( connect( this-> socket, resp-> ai_addr, resp-> ai_addrlen ) == -1 )
         {
            close( this-> socket );
            this-> socket = errno = 0;
            continue;
         }
         break;
      }
   }
   freeaddrinfo( res );

   return errno;
}


Connection_t *Connection_new( const char *hostName, const int port )
{
Connection_t *this;

   if( ( this = calloc( 1, sizeof( Connection_t ) ) ) != NULL )
   {
      serverName = hostName;
      serverPort = port;
      this-> accept = acceptor;
      this-> establish = establish;
      this-> delete = delete;
      this-> drop = drop;
   }

   return this;
}
