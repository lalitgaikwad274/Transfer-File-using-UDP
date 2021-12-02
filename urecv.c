#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFLEN 1024
#define LISTENPORT 1400

struct packet{
  	long int ID;
  	long int length;
  	char data[BUFLEN];
};

int main()
{
	int sockfd, len, n;
	char buffer[BUFLEN];
	struct sockaddr_in receiverAddr, senderAddr;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket system call failed");
		exit(EXIT_FAILURE);
	}

	memset(&receiverAddr, 0, sizeof(receiverAddr));
	memset(&senderAddr, 0, sizeof(senderAddr));

	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_addr.s_addr = INADDR_ANY;
	receiverAddr.sin_port = htons(LISTENPORT);

	if(bind(sockfd, (const struct sockaddr *)&receiverAddr, sizeof(receiverAddr)) < 0){
	 	perror("bind syscall failed");
	 	exit(EXIT_FAILURE);
	}

	while(1){
		len = sizeof(senderAddr);
		printf("waiting file name: \n");
		n = recvfrom(sockfd, (char *)buffer, BUFLEN, MSG_WAITALL, (struct sockaddr *) &senderAddr, &len);
		buffer[n]='\0';
		if(n < 0)
			perror("recv\n");
		
		FILE *fp;
		fp = fopen(buffer,"rb");
		
		if(fp == NULL)
			printf("file is not opened\n");
		else
			printf("file is opened\n");

		fseek(fp,0L,SEEK_END);
		long int size = ftell(fp);
		long int ack = 0;
		long int total_packet = 0;
		struct packet sendp,recvp;
		fseek(fp,0,SEEK_SET);

		if ((size % BUFLEN) != 0)
         total_packet = (size / BUFLEN) + 1;              //Total number of frames to be sent
      else
         total_packet = (size / BUFLEN);
		
		printf("total packets %ld\n",total_packet );

		sendto(sockfd, &total_packet, sizeof(total_packet), 0, (const struct sockaddr *) &senderAddr, len);
	  	n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &receiverAddr, &len);
	  	if(n < 0)
			perror("recv\n");

		while(ack != total_packet){

			sendto(sockfd, &total_packet, sizeof(total_packet), 0, (const struct sockaddr *) &senderAddr, len);
		  	n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &receiverAddr, &len);
		  	if(n < 0)
				perror("recv\n");
		}
		
		for(int i=1;i<=total_packet;i++){
			
			memset(&sendp, 0, sizeof(sendp));
			ack = 0;
			sendp.ID = i;
			sendp.length = fread(sendp.data,1,BUFLEN,fp);

			sendto(sockfd, &sendp, sizeof(sendp), 0, (const struct sockaddr *) &senderAddr, len);
		  	n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &receiverAddr, &len);
		  	if(n < 0)
				perror("recv\n");
			while(ack != sendp.ID){

				sendto(sockfd, &sendp, sizeof(total_packet), 0, (const struct sockaddr *) &senderAddr, len);
			  	n = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &receiverAddr, &len);
			  	if(n < 0)
					perror("recv\n");
			}
			printf("packet = %ld \n",sendp.ID);
			if(total_packet == ack)
				printf("File sent\n");
		}
		fclose(fp);
		
		fclose(fp);
	}
	close(sockfd);
	printf("--------------\n");
	return 0;
}
