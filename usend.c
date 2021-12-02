#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define REMOTEPORT 1400
#define BUFLEN 1024

struct packet{
   long int ID;
   long int length;
   char data[BUFLEN];
};

int main(int argc,char *argv[]){
   
   int sockfd,len,n;
   char buffer[BUFLEN];
   struct sockaddr_in   receiverAddr;
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
      perror("socket failed");
      exit(EXIT_FAILURE);
   }

   memset(&receiverAddr, 0, sizeof(receiverAddr));
   receiverAddr.sin_family = AF_INET;
   receiverAddr.sin_port = htons(REMOTEPORT);
   receiverAddr.sin_addr.s_addr = INADDR_ANY;

   len = sizeof(receiverAddr);
   char* filename = argv[1];
   sendto(sockfd, (const char *)filename, strlen(filename), 0, (struct sockaddr *) &receiverAddr, len);

   FILE *fi;
   char file[BUFLEN];
   strcpy(file,"copied");
   strcat(file,filename);
   struct packet recvp,sendp;
   int i=1;
   while(1){
      long int total_packet = 0;
      long int byterecv = 0;

      n = recvfrom(sockfd, &total_packet, sizeof(total_packet), 0, (struct sockaddr *) &receiverAddr, &len);
      if(n < 0)
      perror("recv\n");

      if(total_packet > 0){
         sendto(sockfd, &total_packet, sizeof(total_packet), 0, (const struct sockaddr *) &receiverAddr, len);
         printf("total packet %ld\n",total_packet);

         fi = fopen(file,"wb");

         for(int i=1;i<=total_packet;i++){
            memset(&sendp, 0, sizeof(sendp));

            n = recvfrom(sockfd, &sendp, sizeof(sendp), 0, (struct sockaddr *) &receiverAddr, &len);
            if(n < 0)
               perror("recv\n");
            
            sendto(sockfd, &sendp.ID, sizeof(sendp.ID), 0, (const struct sockaddr *) &receiverAddr, len);

            if(sendp.ID < i || sendp.ID > i)
               i--;
            else{
               fwrite(sendp.data,1,sendp.length,fi);
               printf("packet no %ld\n",sendp.ID);
               byterecv += sendp.length;
            }
            if(i == total_packet)
               printf("file receive\n");
         }
         printf("file size %ld",byterecv);

      }
      else{
         printf("File is empty\n");
      }

   }
   fclose(fi);
   close(sockfd);
   return 0;
}
