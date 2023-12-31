PACKAGE=	warden
LIB=	http

CFLAGS =	-O3 -Wall -Werror -pedantic -fPIC
SRCS=	Connection.c HTTP.c HTTPHeader.c HTTPRequest.c HTTPResponse.c Message.c

.include <bsd.lib.mk>
