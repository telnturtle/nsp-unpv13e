#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	struct sockaddr_in	servaddr, cliaddr;
	char				buff[MAXLINE];
	time_t				ticks;

	if (argc != 2)
		err_quit("usage: a.out <port>");

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons((short)(argv[1]));	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		pid_t pid;
		socklen_t len = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA *) cliaddr, &len);

		if ( (pid = Fork()) == 0) {
			Close(listenfd);	// child closes listening socket
			printf("connection from %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
					ntohs(cliaddr.sin_port));
			ticks = time(NULL);
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
			Write(connfd, buff, strlen(buff));
			Close(connfd);		// done with this client
			exit(0);			// child terminates
		}

		Close(connfd);
	}
}
