/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/

#include <stdlib.h>
#include <time.h>
#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd, int error_rate, int data_len);     // transmitting and receiving function

int main(int argc, char **argv)
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;
	int error_rate;
    int data_len;
    int port;

    data_len = atoi(argv[1]);
    port = atoi(argv[2]);
	if (argc == 4) {
		error_rate = atoi(argv[3]);
    } else {
		error_rate = 0;
    }

//	char *buf;
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);                              //listen
	if (ret <0) {
		printf("error in listening");
		exit(1);
	}

	while (1)
	{
		printf("waiting for data\n");
		sin_size = sizeof (struct sockaddr_in);
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
		if (con_fd <0)
		{
			printf("error in accept\n");
			exit(1);
		}

		if ((pid = fork())==0)                                         // creat acception process
		{
			close(sockfd);
			str_ser(con_fd, error_rate, data_len);                                          //receive packet and response
			close(con_fd);
			exit(0);
		}
		else close(con_fd);                                         //parent process
        break;
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, int error_rate, int data_len)
{
	char buf[BUFSIZE];
	FILE *fp;
	struct ack_so ack;
	struct pack_so pck;
	int end, n = 0, multiple = 1, rand_num;
	long lseek=0;
	end = 0;
	
	printf("receiving data!\n");

	srand(time(NULL));

	while(!end)
	{
		if ((n = recv(sockfd, &pck, HEADLEN + multiple * data_len, 0)) == -1)                                   //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}

		rand_num = (rand() % 100) + 1;

		if (rand_num <= error_rate) {
			ack.num = 0;
			ack.len = 0;	
		}
		else {
			if (pck.data[pck.len-1] == '\0')									//if it is the end of the file
			{
				end = 1;
				pck.len--;
			}
			memcpy((buf+lseek), pck.data, pck.len);
			lseek += pck.len;

			ack.num = pck.num;
			ack.len = 0;
			multiple = 3 - multiple;
		}
		if ((n = send(sockfd, &ack, 2, 0)) == -1)
		{
				printf("send error!");								//send the ack
				exit(1);
		}
	}
	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
