#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define MAX_SIZE 300

int tmp_L2_saddr[4];
int tmp_L2_daddr[4];
int tmp_L3_saddr[6];
int tmp_L3_daddr[6];

int sndsock,rcvsock;
int clen;	
struct sockaddr_in s_addr,r_addr,r_info;

void set_address();
void setSocket(void);
void *do_thread(void *);

struct L1 { 
	int sport;
	int dport;
	int length;
	int key;
	int sequence;
	int type;
	char L1_data[MAX_SIZE];
};

struct L2 {
	int saddr[4];
	int daddr[4];
	int length;
	char L2_data[MAX_SIZE];
};

struct L3 {
	int saddr[6];
	int daddr[6];
	int length;
	char L3_data[MAX_SIZE];
};

void L1_send(char *input, int length);
void L2_send(char *input, int length);
void L3_send(char *data, int length);
void L4_send(char *data, int length);
char *L1_receive(int *);
char *L2_receive(int *);
char *L3_receive(int *);
char *L4_receive(int *);

int main (void)
{	
	char input[MAX_SIZE];
	int length;
	pthread_t tid;

	setSocket();
	pthread_create(&tid, NULL, do_thread, (void *)0);

	do {
		while(getchar()!='\n');
		set_address();
		while(getchar()!='\n');
		printf("put message to send : ");
		gets(input);
		printf("\n");
		length = strlen(input);	

		L1_send(input, strlen(input));
		sleep(1);
			
	}while(strcmp(input,"exit"));

	close(sndsock);
}

void *do_thread(void *arg)
{

	char output[MAX_SIZE];
	int length;

	while (1) {
		strcpy(output, L1_receive(&length));		
		output[length] = '\0';
		
		if(length > 0) printf("received message = %s\n\n",output);		
		if(!strcmp(output,"exit")) exit(1);
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

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	s_addr.sin_port = htons(7777);
	///////////////////////////////////////////////////////////////////

	//Create Receive Socket////////////////////////////////////////////
	if((rcvsock=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) 
	{
		perror("socket error : ");
		exit(1);
	}

	clen = sizeof(r_addr);
	memset(&r_addr,0,sizeof(r_addr));

	r_addr.sin_family = AF_INET;
	r_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	r_addr.sin_port = htons(7777);

	if(bind(rcvsock,(struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) 
	{
		perror("bind error : ");
		exit(1);
	}

	int optvalue = 1;
	int optlen = sizeof(optvalue);

	setsockopt(sndsock,SOL_SOCKET,SO_REUSEADDR,&optvalue,optlen);
	setsockopt(rcvsock,SOL_SOCKET,SO_REUSEADDR,&optvalue,optlen);
	///////////////////////////////////////////////////////////////////
}

void set_address(){
	int tmp_set_num=0;
	int check_L2 = 0;
	int check_L3 = 0;
	int check_final = 0;
	char cL2_des[13];
	char cL3_des[17];

	while(check_final != 1){
		printf("1.set L2 address\n2.set L3 address\n3.Send Message\nchoose the number for set address : ");
		scanf("%d",&tmp_set_num);
			switch(tmp_set_num){
			case 1:{
				char tmp_saddr[12];
				char tmp_daddr[13];
				printf("Input my L2 address : ");
				scanf("%s",tmp_saddr);
				printf("Input dest L2 address : ");
				scanf("%s",tmp_daddr);
				
				strcpy(cL2_des,tmp_daddr);
				
				int cont=0;
				char *ptr_saddr = strtok(tmp_saddr, ".");
				while (ptr_saddr != NULL)
				{
					tmp_L2_saddr[cont] = atoi(ptr_saddr);
					cont++;
					ptr_saddr = strtok(NULL, ".");
				}
				cont = 0;

				char *ptr_daddr = strtok(tmp_daddr, ".");
				while (ptr_daddr != NULL)
				{
					tmp_L2_daddr[cont] = atoi(ptr_daddr);
					cont++;
					ptr_daddr = strtok(NULL, ".");
				}
				cont = 0;
				check_L2 = 1;
			break;
			}
			
			case 2:{
				char tmp_saddr[17];
				char tmp_daddr[17];
				printf("Input my L3 address : ");
				scanf("%s",tmp_saddr);
				printf("Input dest L3 address : ");
				scanf("%s",tmp_daddr);
				strcpy(cL3_des,tmp_daddr);
				
				int cont=0;
				char *ptr_saddr = strtok(tmp_saddr, "-");
				//printf("%ld\n",sizeof(ptr_saddr));
				while (ptr_saddr != NULL)
				{
					tmp_L3_saddr[cont] = (int)strtol(ptr_saddr,NULL,16);
					//printf("%d\n",tmp_L3_saddr[cont]);
					ptr_saddr = strtok(NULL, "-");
					cont++;
				}
				cont = 0;

				char *ptr_daddr = strtok(tmp_daddr, "-");
				while (ptr_daddr != NULL)
				{
					tmp_L3_daddr[cont] = (int)strtol(ptr_daddr,NULL,16);
					ptr_daddr = strtok(NULL, "-");
					cont++;
				}
				cont = 0;
				check_L3 = 1;
			break;
			}
			
			case 3:{
				if(check_L2 == 1 && check_L3 == 1){
					char tmp_L2_des[13];
					char tmp_L3_des[17];
					printf("put dest L2 address : ");
					scanf("%s",tmp_L2_des);
					//printf("\n");
					if(strcmp(tmp_L2_des,cL2_des) != 0){
						printf("you put the wrong L2 address\n");
						break;
					}
					
					printf("put dest L3 address : ");
					scanf("%s",tmp_L3_des);
					//printf("\n");
					if(strcmp(tmp_L3_des,cL3_des) != 0){
						printf("you put the wrong L3 address\n");
						break;
					}					
					check_final = 1;
					break;
				}
				else if(check_L2 == 1 && check_L3 != 1){
					printf("you don't setting L3 address\n");
					break;
				}
				else if(check_L2 != 1 && check_L3 == 1){
					printf("you don't setting L2 address\n");
					break;
				}
				else{
					printf("you don't setting L2, L3 address\n");
					break;
				}
			}
		}
	}
}


void L1_send(char *input, int length)
{
	struct L1 data;
	char temp[MAX_SIZE];
	int size = 0;
	static int sequence_num = 0;
	int key=0;

	data.sport = 51245;
	data.dport = 80;
	data.length = length;
	data.sequence = sequence_num;

	printf("input encryption value : ");
	scanf("%d",&key);
	data.key = key;
	for(int i=0; i<length; i++){
		data.L1_data[i] += data.key;
	}
	
	int type;
	printf("1.upper\n2.lower\n3.revers\n4.origin\nchoose the type : \n ");
	scanf("%d",&type);
	data.type = type;

	memset(data.L1_data, 0x00, MAX_SIZE);
	memcpy(data.L1_data, (void*)input, length);

	size = sizeof(struct L1) - sizeof(data.L1_data) + length;

   	memset(temp, 0x00, MAX_SIZE);  
   	memcpy(temp, (void *)&data, size);

	printf("sequence at L1_send : %d\n", sequence_num);
	sequence_num++;				
	
	L2_send(temp, size);
}

void L2_send(char *input, int length)
{
	struct L2 data;
	char temp[MAX_SIZE];
	int size = 0;

	data.saddr[0] = tmp_L2_saddr[0];
	data.saddr[1] = tmp_L2_saddr[1];
	data.saddr[2] = tmp_L2_saddr[2];
	data.saddr[3] = tmp_L2_saddr[3];

	data.daddr[0] = tmp_L2_daddr[0];
	data.daddr[1] = tmp_L2_daddr[1];
	data.daddr[2] = tmp_L2_daddr[2];
	data.daddr[3] = tmp_L2_daddr[3];

	data.length = length;

	memset(data.L2_data, 0x00, MAX_SIZE);
	memcpy(data.L2_data, (void*)input, length);

	size = sizeof(struct L2) - sizeof(data.L2_data) + length;

    	memset(temp, 0x00, MAX_SIZE);  
    	memcpy(temp, (void *)&data, size);

	L3_send(temp, size);
}

void L3_send(char *input, int length)
{
	struct L3 data;
	char temp[MAX_SIZE];
	int size = 0;

	data.saddr[0] = tmp_L3_saddr[0];
	data.saddr[1] = tmp_L3_saddr[1];
	data.saddr[2] = tmp_L3_saddr[2];
	data.saddr[3] = tmp_L3_saddr[3];
	data.saddr[4] = tmp_L3_saddr[4];
	data.saddr[5] = tmp_L3_saddr[5];
	
	data.daddr[0] = tmp_L3_daddr[0];
	data.daddr[1] = tmp_L3_daddr[1];
	data.daddr[2] = tmp_L3_daddr[2];
	data.daddr[3] = tmp_L3_daddr[3];
	data.daddr[4] = tmp_L3_daddr[4];
	data.daddr[5] = tmp_L3_daddr[5];
	
	data.length = length;

	memset(data.L3_data, 0x00, MAX_SIZE);
	memcpy(data.L3_data, (void *)input, length);

	size = sizeof(struct L3) - sizeof(data.L3_data) + length;

	memset(temp, 0x00, MAX_SIZE); 
	memcpy(temp, (void *)&data, size); 

	L4_send(temp,size);
}

void L4_send(char *data, int length)
{
	char temp[MAX_SIZE];
	
	
	memset(temp, 0x00, MAX_SIZE);
	memcpy(temp, (void *)data, length);

	if(sendto (sndsock, temp, length, 0, (struct sockaddr *)&r_addr,sizeof(r_addr)) <= 0)
	{
		perror("send error : ");
		exit(1);
	}
}

char * L1_receive(int *length)
{
	struct L1 *data;
	static int sequence_num = 0;
	
	data = (struct L1*)L2_receive(length); 

	*length = *length - sizeof(data->sport) - sizeof(data->length) - sizeof(data->dport) - sizeof(data->key) - sizeof(data->sequence) - sizeof(data->type);

	
	if(sequence_num == data->sequence){
		printf("sequence at L1_receive : %d\n", sequence_num);
		sequence_num++;
		printf("decrypted : %s\n",data->L1_data);
		char decrypted_m[50];
		int i=0;
		while(data->L1_data[i] != '\0'){
			decrypted_m[i] = data->L1_data[i] - data->key;
			i++;
		}
		printf("encrypted : %s\n", decrypted_m);
		
		int j=0;
		if(data->type == 1){
			while(data->L1_data[j] != '\0'){
			data->L1_data[j] = data->L1_data[j] -32;
			j++;
			}
		}
		else if(data->type == 2){
			while(data->L1_data[j] != '\0'){
			data->L1_data[j] = data->L1_data[j] + 32;
			j++;
			}
		}
		else if(data->type == 3){
			for(j=0 ; j<data->length/2; j++){
				char temp = data->L1_data[j];
				data->L1_data[j] = data->L1_data[data->length-1-j];
				data->L1_data[data->length -1 -j]= temp;
			}
		}
		else if(data->type == 4){
		}
	}
	else{
		printf("not a same sequence number");
	}

	printf("key value : %d\n",data->key);

	
	return (char *)data->L1_data;
}

char * L2_receive(int *length) 
{
	struct L2 *data;
	int L2_receive_address[4] = {192,168,30,46};
	
	data = (struct L2*)L3_receive(length);
	
	if(
	data->daddr[0] == L2_receive_address[0] &&
	data->daddr[1] == L2_receive_address[1] &&
	data->daddr[2] == L2_receive_address[2] &&
	data->daddr[3] == L2_receive_address[3])
	{
		*length = *length - sizeof(data->daddr) - sizeof(data->length) - sizeof(data->saddr);

		return (char *)data->L2_data;
	}
	else{
		printf("L2 : you put the wrong address\nL2 : your request is discard\n");
	}
}

char * L3_receive(int *length)
{
	struct L3 *data;
	int L3_receive_address[6] = {1,35,69,103,137,10};
	
	data = (struct L3*)L4_receive(length);
	
	if(
	data->daddr[0] == L3_receive_address[0] &&
	data->daddr[1] == L3_receive_address[1] &&
	data->daddr[2] == L3_receive_address[2] &&
	data->daddr[3] == L3_receive_address[3] &&
	data->daddr[4] == L3_receive_address[4] &&
	data->daddr[5] == L3_receive_address[5])
	{
		*length = *length - sizeof(data->daddr) - sizeof(data->length) - sizeof(data->saddr);
		
		return (char *)data->L3_data;
	}
	else{
		printf("L3 : you put the wrong address\nL3 : your request is discard\n");
	}
}

char *L4_receive(int *length)
{
	static char data[MAX_SIZE];
	int r_length = 0;

	if((r_length = recvfrom(rcvsock, data, MAX_SIZE, 0,(struct sockaddr *)&r_info, &clen)) <= 0)
	{
		perror("read error : ");
		exit(1);
	}

	data[r_length] = '\0'; 
	*length = r_length;

	return data;
}

