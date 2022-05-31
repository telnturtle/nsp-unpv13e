#include	"unp.h"


void sig_chld(int signo);


int main(int argc, char **argv)
{
	int					listenfd, connfd, udpfd, nready, maxfdp1;
	int 				i, maxi, maxfd, sockfd, client[FD_SETSIZE];
	char				mesg[MAXLINE];
	pid_t				childpid;
	fd_set				rset, allset;
	ssize_t				n;
	socklen_t			len;
	const int			on = 1;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	/* create listening TCP socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	/* create UDP socket */
	udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(udpfd, (SA *) &servaddr, sizeof(servaddr));

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);

	FD_ZERO(&rset);
	maxfdp1 = max(listenfd, udpfd) + 1;
	for ( ; ; )
	{
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
		{
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("select error");
		}

		// Accept new TCP client
		if (FD_ISSET(listenfd, &rset))
		{
			len = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &len);

			for (i = 0; i < FD_SETSIZE; i++)
			{
				if (client[i] < 0)
				{
					client[i] = connfd; /* save descriptor */
					break;
				}
			}
			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
			
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		// TCP clients
		for (i = 0; i <= maxi; i++)
		{
			if ( (sockfd = client[i]) < 0 )
				continue;
			if (FD_ISSET(sockfd, &rset))
			{
				if ( (n = Read(sockfd, mesg, MAXLINE)) == 0 )
				{
					Close(sockfd);		/* connection closed by client */
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
				else
					Writen(sockfd, mesg, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}

		// UDP message
		if (FD_ISSET(udpfd, &rset))
		{
			len = sizeof(cliaddr);
			n = Recvfrom(udpfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);

			Sendto(udpfd, mesg, n, 0, (SA *) &cliaddr, len);
		}
	}
}


void sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}