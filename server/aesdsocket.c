/**
 * @file    aesdsocket.c
 * @brief   Code to create a socket, receive packet. Local port 9000. Store in file, 
 *	    send it back to local client. accepts -d argument to run as daemon
 *	    in the background. SIGINT and SIGTERM are the only signals possible
 * 	    and are handled.
 *			
 * @author	Venkat Sai Krishna Tata
 * @Date	10/10/2021
 * @References: Textbook: Linux System Programming
 *		https://stackoverflow.com/questions/803776/help-comparing-an-argv-string
 *		https://beej.us/guide/bgnet/html/
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>	
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define USE_AESD_CHAR_DEVICE 1

#define PORT "9000"
#define CHUNK_SIZE 600
#define BACKLOG 10

#define BEGIN 0

char IP_addr[INET6_ADDRSTRLEN];

typedef struct{
    bool thread_complete;                        
    sigset_t mask;
}threadParams_t;

pthread_t threads; 
threadParams_t threadParams;
int serv_sock_fd,client_sock_fd,counter =1,output_file_fd;
int finish;
struct sockaddr_in conn_addr;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}





void close_all()
{
    finish = 1;
	//Functions called due to an error, hence close files errno not 
	//checked in this function
	//All close errors handled in signal handler when no error occured
	close(serv_sock_fd);
	

    //After completing above procedure successfuly, exit logged
	syslog(LOG_DEBUG,"Caught signal, exiting");
	
    //close the logging for aesdsocket                              
    closelog();
}

static void signal_handler(int signo)
{
   if(signo == SIGINT || signo==SIGTERM) 
    {
		shutdown(serv_sock_fd,SHUT_RDWR);
		finish = 1;    
    }
}
float temp_sensor_read()
{
	// Create I2C bus
	int file;
	
	char *i2c_dev_filename = "/dev/i2c-1";//Always adapter 1 on RPi
	file = open(i2c_dev_filename, O_RDWR);
	if(file < 0) 
	{
		syslog(LOG_ERR,"Failed to open the i2c-1 bus");
		exit(1);
	}

	//set the address of the device to address
	if(ioctl(file, I2C_SLAVE, 0x48) < 0) //TMP102 I2C address is 0x48(72)
	{
		syslog(LOG_ERR,"Failed to set the address of the device to address");
		exit(1);
	}

	// Select configuration register(0x01)
	// Continous Conversion mode, 12-Bit Resolution
	char buf[3] = {0};
	buf[0] = 0x01;
	buf[1] = 0x60;
	buf[2] = 0xA0;
	write(file, buf, 3);

	//Wait for the transaction to complete and sensor to initialise and perform measurement
	sleep(1);
	
	//On completing the measurement, the values can be read
	char reg[1] = {0x00};
	write(file, reg, 1);

	//read the measured temperature value
	char measured_temp[2] = {0};
	int rc = read(file, measured_temp, 2);
	if( rc != 2)
	{
		syslog(LOG_ERR,"i2c read transaction failed");
		printf("i2c read transaction failed %d",rc);
		exit(1);
	}
	
	//Convert register values to temperature measurements
	int temp = (measured_temp[0] * 256 + measured_temp[1]) / 16;
	if(temp > 2047)
	{
		temp -= 4096;
	}
	float sensed_temp_value = (temp * 0.0625);
	syslog(LOG_DEBUG,"Temperature in Celsius : %f degree C", sensed_temp_value);
	return sensed_temp_value;
}
void* handle_connection(void *threadp)
{
	int failure=0;
	char temp_values[8]={'\0'};
    threadParams_t *thread_handle_sock = (threadParams_t*)threadp;
     
    while(!finish)
	{
		if (sigprocmask(SIG_BLOCK,&(thread_handle_sock->mask),NULL) == -1)
		{
			perror("sigprocmask block error");
			close_all();
			exit(-1);
		}
		float temperature=temp_sensor_read();
		int whole_temp = (temperature *100);
		int decimal_temp= (temperature *100) - (whole_temp *100);
		sprintf(temp_values,"%d.%d",whole_temp,decimal_temp);
		int rc = send(client_sock_fd, temp_values, strlen(temp_values), 0);
		if(rc == -1)
		{
			perror("send error");
			failure++;
			//Still tries to send next value next time
		}
		if(failure==3)
		{
			close_all();
			exit(-1);
	 	}
	}
	
	thread_handle_sock->thread_complete = true;
	if (sigprocmask(SIG_UNBLOCK,&(thread_handle_sock->mask),NULL) == -1)
	{
		perror("sigprocmask unblock error");
		close_all();
		exit(-1);
	}

    close(client_sock_fd);
    return threadp;
}



int main(int argc, char* argv[])
{

	//Opens a connection with facility as LOG_USER to Syslog.
	openlog("aesdsocket",0,LOG_USER);



    //With node as null and ai_flags as AI_PASSIVE, the socket address 
	//will be suitable for binding a socket that will accept connections
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo *res;
	
	//Node NULL and service set with port number as 9000, the pointer to
	//linked list returned and stored in res
	if (getaddrinfo(NULL, PORT , &hints, &res) != 0) 
	{
		perror("getaddrinfo error");
		exit(-1);
	}

    //Creating an end point for communication with type = SOCK_STREAM(connection oriented)
	//and protocol =0 which allows to use appropriate protocol (TCP) here
	serv_sock_fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(serv_sock_fd==-1)
	{
		perror("socket error");
		exit(-1);
	}
		
	//Set options on socket to prevent binding errors from ocurring
	int dummie =1;
	if (setsockopt(serv_sock_fd, SOL_SOCKET, SO_REUSEADDR, &dummie, sizeof(int)) == -1) 
	{	
		perror("setsockopt error");
    }
    
	//Assign address to the socket created
	int rc=bind(serv_sock_fd, res->ai_addr, res->ai_addrlen);
	if(rc==-1)
	{
		perror("bind error");
		exit(-1);
	}
  
    freeaddrinfo(res);

	//Registering signal_handler as the handler for the signals SIGTERM 
	//and SIGINT
	//Reference: Ch 10 signals, Textbook: Linux System Programming
	sigset_t socket_set;
	if (signal(SIGINT, signal_handler) == SIG_ERR) 
	{
		fprintf (stderr, "Cannot handle SIGINT\n");
		exit (EXIT_FAILURE);
	}
	
	if (signal(SIGTERM, signal_handler) == SIG_ERR) 
	{
		fprintf (stderr, "Cannot handle SIGTERM\n");
		exit (EXIT_FAILURE);
	}

	//Adding only signals SIGINT and SIGTERM to an empty set to enable only them
	rc = sigemptyset(&socket_set);
	if(rc !=0)
	{
		perror("signal empty set error");
		exit(-1);
	}
	
	rc = sigaddset(&socket_set,SIGINT);
	if(rc !=0)
	{
		perror("error adding signal SIGINT to set");
		exit(-1);
	}
	
	rc = sigaddset(&socket_set,SIGTERM);
	if(rc !=0)
	{
		perror("error adding signal SIGTERM to set");
		exit(-1);
	}  

	//Post binding, the server is running on the port 9000, so now
	//remote connections (client) can listen to server
	//Backlog - queue of incoming connections before being accepted to send/recv 
	//backlog set as 4, can be reduced to a lower number since we get only 1 connection
	if(listen(serv_sock_fd,BACKLOG)==-1)
	{
		perror("error listening");
		exit(-1);
	}
	
	while(1) 
	{
		socklen_t conn_addr_len = sizeof(conn_addr);
		//Accept an incoming connection
		client_sock_fd = accept(serv_sock_fd,(struct sockaddr *)&conn_addr,&conn_addr_len);
		if(client_sock_fd ==-1)
		{
			//If no incoming connection, graceful termination done
			perror("client error");

		}
		if(finish) 
			break;

		//Below line of code reference: https://beej.us/guide/bgnet/html/
		//Check if the connection address family is IPv4 or IPv6 and retrieve accordingly
		//Convert the binary to text before logging 
		inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&conn_addr), IP_addr, sizeof(IP_addr));
		syslog(LOG_DEBUG,"Accepted connection from %s", IP_addr);
	

		int return_code = pthread_create(&(threads),NULL,&handle_connection,(void*)&(threadParams));
		if(return_code != 0)
		{
			syslog(LOG_ERR,"Thread creation failed");
			close_all();
		}	

		pthread_join(threads,NULL);
	}
	
	close_all();
	return 0;
}

