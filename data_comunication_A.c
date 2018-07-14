#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define MAX_SIZE 400
#define M_SIZE 20

int sndsock,rcvsock;
int clen;	
int sabm_on = 0;	// 0 if first time, else 1
int source_sequence_num = 0;	// number of send signal
int dest_sequence_num = 0;	// number of get signal
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

static int my_address = 0x11;
static int dest_address = 0x22;

int main (void)
{
	
	char input[MAX_SIZE];
	int length;
	pthread_t tid;

	setSocket();
	pthread_create(&tid, NULL, do_thread, (void *)0);

	do {
		printf("\nInput message to send : \n");
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
	s_addr.sin_port = htons(9000);
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
	r_addr.sin_port = htons(9001);

	if(bind(rcvsock,(struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) 
	{
		perror("bind error : ");
		exit(1);
	}
}

void data_send(char *data, int length)
{
	char temp[MAX_SIZE];

	memset(temp, 0x00, MAX_SIZE);
	
	// initialize the U-FRAME(SABM)
	if(sabm_on == 0){
		printf("connection is not established, send U-FRAME(SABM)\n");
		struct U_FRAME SABM;
		SABM.start_flag = 01111110;
		SABM.address = dest_address;
		SABM.control[0] = 11;
		SABM.control[1] = 11;
		SABM.control[2] = 0;
		SABM.control[3] = 100;
		SABM.mg_info[0] = '0';
		SABM.fcs = 0;
		SABM.end_flag = 01111110;
		length = sizeof(SABM);
		memcpy(temp, &SABM, length);
	}
	// if connection is established && strcmp(data, "close") == 0, send U-frame(DISC)
		// initialize the U-frame(DISC)
	else if(sabm_on == 1 && strcmp(data, "close") == 0){
		printf("disconnetion is requested, send the U-Frame(DISC)\n");
		struct U_FRAME DISC;
		DISC.start_flag = 01111110;
		DISC.address = dest_address;
		DISC.control[0] = 11;
		DISC.control[1] = 00;
		DISC.control[2] = 0;
		DISC.control[3] = 110;
		DISC.mg_info[0] = '0';
		DISC.fcs = 0;
		DISC.end_flag = 01111110;
		length = sizeof(DISC);
		memcpy(temp, &DISC, length);
	}
	
	// if connection is established, send I-FRAME(data)
		// initialize the I-FRAME(data)
	else if(sabm_on == 1){
		struct I_FRAME i_frame;
		i_frame.start_flag = 01111110;
		i_frame.address = 0x22;
		i_frame.control[0] = 0;
		source_sequence_num++;
		i_frame.control[1] = source_sequence_num;
		i_frame.control[2] = 0;
		i_frame.control[3] = dest_sequence_num;
		// i_frame.data = data;
		strcpy(i_frame.data, data);
		i_frame.fcs = 0;
		i_frame.end_flag = 01111110;
		
		printf("send I-FRAME(data), data frame %d\n", source_sequence_num);
		length = sizeof(i_frame);
		memcpy(temp, &i_frame, length);
	}
	
	if(sendto (sndsock, temp, length, 0, (struct sockaddr *)&s_addr,sizeof(s_addr)) <= 0)
	{
		perror("send error : ");
		exit(1);
	}
}

void data_receive(int *length)
{
	char temp[MAX_SIZE];
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
	
	memset(temp, 0x00, MAX_SIZE);

	printf("%s\n",data);

	// decalsulate the flag field
	memcpy(&flag, data, sizeof(flag));
	data += sizeof(flag);
	
	// decalsulate the address field
	memcpy(&address, data, sizeof(address));
	data += sizeof(address);
 
	printf("packet received (address: %d)\n", address);	
	
	// decalsulate the control field
	memcpy(&control[0], data, sizeof(control[0]));
	data += sizeof(control[0]);
	
	memcpy(&control[1], data, sizeof(control[1]));
	data += sizeof(control[1]);
	
	// discard the dirty bit
	memcpy(&control[2], data, sizeof(control[2]));
	data += sizeof(control[2]);

	memcpy(&control[3], data, sizeof(control[3]));
	data += sizeof(control[3]);
		
	if(control[0] == 11){ // if(U-frame)
		if(control[1] == 11 && control[3] == 100){ // received SABM
			// reply UA frame
			printf("received frame type : U-frame(SABM)\n");
			sabm_on = 1;
			struct U_FRAME UA;
			UA.start_flag = 01111110;
			UA.address =0x22;
			UA.control[0] = 11;
			UA.control[1] = 00;
			UA.control[2] = 0;
			UA.control[3] = 110;
			UA.mg_info[0] = '0';
			UA.fcs = 0;
			UA.end_flag = 01111110;
			
			printf("connection is established");
			printf("reply the U-Frame(UA)\n");
			s_length = sizeof(UA);
			memcpy(temp, &UA, s_length);
		}
		else if(control[1] == 00 && control[3] == 010){ //received DISC
			// reply UA frame
			printf("received frame type : U-frame(DISC)\n");
			sabm_on = 0;
			struct U_FRAME UA;
			UA.start_flag = 01111110;
			UA.address =0x22;
			UA.control[0] = 11;
			UA.control[1] = 00;
			UA.control[2] = 0;
			UA.control[3] = 110;
			UA.mg_info[0] = '0';
			UA.fcs = 0;
			UA.end_flag = 01111110;
			
			s_length = sizeof(UA);
			memcpy(temp, &UA, s_length);
			printf("reply the U-Frame(UA)\n");
			printf("successfully disconneted with destination\n");
		}
		else if(control[1] == 00 && control[3] == 110 && sabm_on == 1){	// received UA (be asked disconneted)
			sabm_on == 0;
			printf("successfully disconneted with destination\n");
		}
				else if(control[1] == 00 && control[3] == 110 && sabm_on == 0){	// received UA (be asked conneted)
			sabm_on == 1;
			printf("conneted is established\n");
		}
	}
	else if(control[0] == 0){ // if(I-frame)
		// reply RR frame
		dest_sequence_num++;
		printf("received frame type : I-frame(data frame : %d)\n", dest_sequence_num);
		printf("received message : %");
		struct S_FRAME RR;
		RR.start_flag = 01111110;
		RR.address = 0x22;
		RR.control[0] = 10;
		RR.control[1] = 00;
		RR.control[2] = 0;
		RR.control[3] = source_sequence_num;
		RR.fcs = 0;
		RR.end_flag = 01111110;
		
		printf("send the S-frame(RR)\n");
		s_length = sizeof(RR);
		memcpy(temp, &RR, s_length);
	}
	else if(control[0] == 10 && control[1] == 0){ // if(S-frame)
		dest_sequence_num++;
		printf("received frame type : S-frame(RR)\n");
		printf("requested next packet sequence number : %d\n", dest_sequence_num);
		
	}
	
	if(sendto (sndsock, temp, s_length, 0, (struct sockaddr *)&s_addr,sizeof(s_addr)) <= 0){
		perror("send error : ");
		exit(1);
	}
}
