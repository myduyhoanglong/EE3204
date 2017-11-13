/*******************************
tcp_client.c: the source file of the client in tcp transmission 
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, int *error_num, int data_len);             //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calculate the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp, *f;
	int error_num;
    int data_len;
    int port;

	if (argc != 4) {
		printf("parameters not match\n");
	}

    data_len = atoi(argv[2]);
    port = atoi(argv[3]);

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);                           //create the socket
	if (sockfd < 0)
	{
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(port);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host
    if (ret != 0) {
        printf ("connection failed, retrying\n"); 
        close(sockfd);
        exit(0);
    }
	
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	ti = str_cli(fp, sockfd, &len, &error_num, data_len);                       //perform the transmission and receiving
	rt = (len/(float)ti);                                 //caculate the average transmission rate
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\nNumber of errors: %d\n", ti, (int)len, rt, error_num);

	if((f = fopen("data.csv", "a+")) != NULL) {
		fprintf(f, "%d, %.3f, %d, %.3f, %d\n", data_len, ti, (int)len, rt, error_num);
		fclose(f);
	} else {
		printf("error in opening data\n")	;
	}

	close(sockfd);
	fclose(fp);
//}
	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, int *error_num, int data_len)
{
	char *buf;
	long lsize, ci;
	struct ack_so ack;
	struct pack_so pck;
	int n, slen, err_num = 0, multiple = 1, packet_num = 1;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;
	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n", data_len);

	// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL)
		exit (2);

  	// copy the file into the buffer.
	fread (buf,1,lsize,fp);

  	/*** the whole file is loaded in the buffer. ***/
	buf[lsize] = '\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	while(ci <= lsize)
	{
		if ((lsize+1-ci) <= multiple * data_len)
			slen = lsize+1-ci;
		else 
			slen = multiple * data_len;

		pck.num = packet_num;
		pck.len = slen;
		memcpy(pck.data, (buf+ci), slen);
		n = send(sockfd, &pck, HEADLEN + slen, 0);
		if(n == -1) {
			printf("send error!");								//send the data
			exit(1);
		}
		if ((n = recv(sockfd, &ack, 2, 0)) == -1)                                   //receive the ack
		{
			printf("error when receiving\n");
			exit(1);
		}
		if (ack.num != packet_num || ack.len != 0) {
			err_num++;
			//printf("error in transmission\n");
		}
		else {
			multiple = 3 - multiple;
			ci += slen;
			packet_num++;
		}
	}
	gettimeofday(&recvt, NULL);
	*len = ci;                                                         //get current time
	*error_num = err_num;
	tv_sub(&recvt, &sendt);                                            // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return (time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
