/* Title	:	A minimal FTP client for the AlokFTP Server			*/
/* Source	:	ftp_client.c 							*/	
/* Date		:	09/03/2012	   						*/
/* Author	:	Alok Upadhyay	   						*/
/* Input	:	The incoming connections from user
			- Port 2121 for control messages and data transfer		*/
/* Output	:	The server prompts, the error/warning messages.			*/
/* Method	:	A well planned execution of Linux function calls		*/
/* Possible Further Improvements :	1. User account creation/authentication -not done, since it is tftp type only
					2. A good/innovative prompt.		-done
					3. An impressive banner! :P		-done
					4. Serve users across multiple sessions -done
					5. Serve multiple users at the same time-not done 	
					6. How to recieve/compile using sendfile()-done	*/


/* Included header files */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/* Pre-processor directives */
#define DELIM " "


/* Functions and subroutines declaration */
void writeToFile(char *, char *);
void transferFile(char *);


/* Global Variables */
int sock, bytes_recieved, file_sock, file_bytes_recieved;  
char send_data[1024],recv_data[4096], recv_file_data[4096], send_file_data[4096];
struct hostent *host;
struct sockaddr_in server_addr, file_server_addr;  


int main(int argc, char **argv)

{
	if ( argc != 2 ) /* argc should be 2 for correct execution */
    	{
      	  	 /* We print argv[0] assuming it is the program name */
       		 printf( "usage: %s <server-ip-address>\n", argv[0] );
    	}
   	 else 
   	 {

        host = (struct hostent *)gethostbyname(argv[1]); //self loop IP

	/** Control socket declaration */	
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // TCP connection
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(2121);    // Port to connect to server
        server_addr.sin_addr = *((struct in_addr *) host->h_addr);
        bzero(&(server_addr.sin_zero),8); 

        if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) == -1) 
        {
            perror("Control Connect");
            exit(1);
        }
	/** Control socket declaration ends*/	


	

	//Recieving initial banner information through the control socket.
	 bytes_recieved=recv(sock,recv_data,1024,0);
         recv_data[bytes_recieved] = '\0';
         printf("%s" , recv_data);


        while(1)
        {
        
	//Recieving the customary prompt and response of the previous command.	
		int i=0;
		while(i<4096)
		{	
			recv_data[i] = '\0';	
			i++; 
		}
	
	        bytes_recieved=recv(sock,recv_data,4096,0);
	        recv_data[bytes_recieved] = '\0';
	        printf("%s \b " , recv_data);
       		
	  	gets(send_data);
          
           
                	
		if(!strcmp(send_data, "bye"))
		{
			send(sock,send_data,strlen(send_data), 0); 
			close(sock);
			exit(0);			
			
		}


		else if (!strncmp(send_data, "get", 3))
		{

			send(sock,send_data,strlen(send_data), 0);
			bytes_recieved=recv(sock,recv_data,4096,0);
		        recv_data[bytes_recieved] = '\0';
          		
			printf("File data recieved.\n"  , recv_data);
			
			//send_data will contain the filename; recv_data will contain the actual file data;
			writeToFile(send_data, recv_data);	
			
			int i=0;
			while(i<4096)
			{	
				recv_data[i] = '\0';	
				if(i<1024)
					send_data[i] = '\0';
				i++; 
			}
			

		}

		
		else if (!strncmp(send_data, "put", 3))
		{
			send(sock, send_data, strlen(send_data), 0);	//sending the command to prepare server for data reception.			
			transferFile(send_data);	// will incur one send operation of the data			
		}

		else 
			send(sock,send_data,strlen(send_data), 0); 
        
        }   
	}
	return 0;
}

void writeToFile(char *command, char *data)
{
	FILE *fp;
	
	char * first_arg ,  * second_arg ;
	first_arg = strtok(command, DELIM);
	second_arg = strtok(NULL, DELIM);

	fp = fopen(second_arg, "w");
	if (fp!=NULL)
 	 {
  		 fputs ( data,fp);
  	 	 fclose (fp);
  	 }

}

void transferFile(char *command)
{
	int fd;
	char * first_arg ,  * second_arg ;
	first_arg = strtok(command, DELIM);
	second_arg = strtok(NULL, DELIM);

	
	struct stat stat_buf;	/* argument to fstat */
	off_t offset = 0;          /* file offset */
	int rc;


	/* open the file to be sent */
	    fd = open(second_arg, O_RDONLY);
	    if (fd == -1) {
	      fprintf(stderr, "unable to open '%s': %s\n", second_arg, strerror(errno));
	      //exit(1);
	    }

	/* get the size of the file to be sent */
	    fstat(fd, &stat_buf);


    	/* copy file using sendfile */
	    offset = 0;
	    rc = sendfile (sock, fd, &offset, stat_buf.st_size);
	    if (rc == -1) {
	      fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
	      exit(1);
	    }
	    if (rc != stat_buf.st_size) {
	      fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
	              rc,
	              (int)stat_buf.st_size);
	      exit(1);
	    }
	
    	/* close descriptor for file that was sent */
	    close(fd);

	
		
}
