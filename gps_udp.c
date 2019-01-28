#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void UDPSend(char *message, int port)
{ 
	if (port > 0)
	{
		int sd, rc;
		struct sockaddr_in cliAddr, remoteServAddr;
		struct hostent *h;
		int broadcast = 1;
	  
		h = gethostbyname("255.255.255.255");
		remoteServAddr.sin_family = h->h_addrtype;
		memcpy((char *) &remoteServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
		remoteServAddr.sin_port = htons(port);

		// socket creation
		sd = socket(AF_INET,SOCK_DGRAM,0);
		if (sd<0)
		{
			printf("Cannot open socket on port %d\n", port);
			return;
		}
		
		if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof broadcast) == -1)
		{
			printf("setsockopt (SO_BROADCAST)");
			return;
		}
	  
		// bind any port
		cliAddr.sin_family = AF_INET;
		cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		cliAddr.sin_port = htons(0);
	  
		rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
		if (rc<0)
		{
			printf("Cannot bind port %d\n", port);
			return;
		}

		// send data
		rc = sendto(sd, message, strlen(message)+1, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));

		if (rc<0)
		{
			printf("%s: cannot send data %s to port %d \n", message, port);
			close(sd);
			return;
		}

		close(sd);
	}
}

void GPSLoop(int Port)
{
	int fd, Length;
	char Line[250];
	
	fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
		
	if (fd >= 0)
	{
		struct termios options;
		
		fcntl(fd, F_SETFL, 0);

		tcgetattr(fd, &options);

		options.c_lflag &= ~ECHO;
		options.c_cc[VMIN]  = 0;
		options.c_cc[VTIME] = 10;

		options.c_cflag &= ~CSTOPB;
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
		options.c_cflag |= CS8;

		tcsetattr(fd, TCSANOW, &options);
		
		printf("Opened serial GPS Port\n");
		
		while (1)
		{
			unsigned char Character;
	
            if (read(fd, &Character, 1) >= 0)
			{
				if (Character == '$')
				{
					Line[0] = Character;
					Length = 1;
				}
				else if (Length > 200)
				{
					Length = 0;
				}
				else if ((Length > 0) && (Character != '\r'))
				{
					Line[Length++] = Character;
					if (Character == '\n')
					{
						Line[Length] = '\0';
						printf("%s", Line);
						
						UDPSend(Line, Port);
						
						Length = 0;
					}
				}
			}
		}
	}
	else
	{
		printf("*** FAILED TO open serial GPS Port ***\n");
	}
}

void main(int argc, char *argv[])
{
	int Port;

	Port = 0;
	if (argc > 1)
	{
		Port = atoi(argv[1]);
	}

	if (Port > 0)
	{
		GPSLoop(Port);
	}
	else
	{
		printf("Usage: gps_udp <port>\n");
	}
}

