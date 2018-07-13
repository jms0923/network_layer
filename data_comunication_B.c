#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MAX_SIZE 400
#define M_SIZE 20

int sndsock,rcvsock;
int clen;	
struct sockaddr_in s_addr,r_addr,r_info;

void setSocket(void);
void *do_thread(void *);

struct U_FRAME{
	int start_flag;
	int address;
	int control[4];
	char mg_info[M_SIZE];
	int fcs;
	int end_flag;
};

struct I_FRAME{
	int start_flag;
	int address;
	int control[4];
	char data[M_SIZE];
	int fcs;
	int end_flag;
};

struct S_FRAME{
	int start_flag;
	int address;
	int control[4];
	int fcs;
	int end_flag;
};

void data_send(char *data, int length);
void data_receive();

static int my_address = 0x22;
static int dest_address = 0x11;
static int isConnected = 0;
static int source_sequence = 0;
static int dest_sequence = 0;
static int sabm_req = 0; // to identigy whether or not u-frame(SABM) is forwarded to destination
static int disc_req = 0; // to identify whether or not u-frame(disc) is forwarded to destination

int main (void)
{

	char input[MAX_SIZE];
	int length;
	pthread_t tid;

	setSocket();
	pthread_create(&tid, NULL, do_thread, (void *)0);

	do {
		printf("Input message to send : \n");
		scanf("%s",input);

		length = strlen(input);
		data_send(input, strlen(input));
		while(getchar()!='\n');
		sleep(1);
			
	}while(strcmp(input,"exit"));

	close(sndsock);

}

void *do_thread(void *arg)
{

	char output[MAX_SIZE];
	int length;

	while (1) {
		data_receive();	
	}
	
	close(rcvsock);

}

void setSocket()
{
	
	//Create Send Socket///////////////////////////////////////////////
	if((sndsock=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) 
	{
		perror("socket error : ");
		exit(1);
	}

	memset((char *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	s_addr.sin_port = htons(9001);
	///////////////////////////////////////////////////////////////////

	//Create Receive Socket////////////////////////////////////////////
	memset(&r_info,0,sizeof(struct sockaddr_in));

	if((rcvsock=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) 
	{
		perror("socket error : ");
		exit(1);
	}

	clen = sizeof(r_addr);
	memset(&r_addr,0,sizeof(r_addr));

	r_addr.sin_family = AF_INET;
	r_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	r_addr.sin_port = htons(9000);

	if(bind(rcvsock,(struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) 
	{
		perror("bind error : ");
		exit(1);
	}

	int optvalue = 1;
	int optlen = sizeof(optvalue);

	// setsockopt(sndsock,SOL_SOCKET,SO_REUSEADDR,&optvalue,optlen);
	// setsockopt(rcvsock,SOL_SOCKET,SO_REUSEADDR,&optvalue,optlen);
	///////////////////////////////////////////////////////////////////
}


void data_send(char *data, int length)
{
	char temp[MAX_SIZE];

	memset(temp, 0x00, MAX_SIZE);
	memcpy(temp, data, length);
	
	// if connection is not established, send U-FRAME(SABM)
		// initialize the U-FRAME(SABM)

	// if connection is established, send I-FRAME(data)
		// initialize the I-FRAME(data)

	// if connection is established && strcmp(data, "close") == 0, send U-frame(DISC)
		// initialize the U-frame(DISC)

	if(sendto (sndsock, temp, length, 0, (struct sockaddr *)&s_addr,sizeof(s_addr)) <= 0)
	{
		perror("send error : ");
		exit(1);
	}
}

void data_receive(int *length)
{
	char* data = (char*)malloc(MAX_SIZE);
	char rec_buffer[MAX_SIZE];
	int r_length = 0;
	int s_length = 0;
	int flag;
	int address;
	int control[4];

	if((r_length = recvfrom(rcvsock, rec_buffer, MAX_SIZE, 0,(struct sockaddr *)&r_info, &clen)) <= 0)
	{
		perror("read error : ");
		exit(1);
	}

	memcpy(data,rec_buffer,MAX_SIZE);

	printf("%s\n",data);

	// decalsulate the flag field
	// memcpy(&flag, data, sizeof(flag));
	// data += sizeof(flag);
	
	// decalsulate the address field
	// memcpy(&address, data, sizeof(address));
	// data += sizeof(address);
 
	// printf("packet received (address: %d)\n", address);	
	
	// decalsulate the control field
	// memcpy(&control[0], data, sizeof(control[0]));
	// data += sizeof(control[0]);
	
	// memcpy(&control[1], data, sizeof(control[1]));
	// data += sizeof(control[1]);
	
	// discard the dirty bit
	// memcpy(&control[2], data, sizeof(control[2]));
	// data += sizeof(control[2]);

	// memcpy(&control[3], data, sizeof(control[3]));
	// data += sizeof(control[3]);
		
	// if(control[0] == 11) // if(U-frame)
		// if(control[1] == 11 && control[3] == 100) // received SABM
			// reply UA frame
		// else if(control[1] == 00 && control[3] == 010) //received DISC
			// reply UA frame
	
	// else if(control[0] == 0) // if(I-frame)
		// reply RR frame
				
	
	// else if(control[0] == 10 && control[1] == 0) // if(S-frame)

}

