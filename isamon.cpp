/*ISA projekt*/
/*Name: Peter Revaj*/
/*Login: xrevaj00*/
/*2017/2018*/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>

#include <netinet/ip.h> 
#include <sys/ioctl.h>  
#include <bits/ioctls.h>  
#include <net/if.h> 
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>




using namespace std;


// struktura pre arp hlavicku
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};


//help	
void printhelp(){
	printf("Usage:\n");
	printf("\n");
	printf("isamon [-h] [-i <interface>] [-t] [-u] [-p <port>] [-w <ms>] -n <net_address/mask> \n");
	printf("-h --help -- zobrazí nápovědu\n");
	printf("-i --interface <interface> -- rozhraní na kterém bude nástroj scanovat \n");
	printf("-n --network <net_address/mask> -- ip adresa síťe s maskou definující rozsah pro scanování \n");
	printf("-t --tcp -- použije TCP \n");
	printf("-u --udp -- použije UDP \n");
	printf("-p --port <port> -- specifikace scanovaného portu, pokud není zadaný, scanujte celý rozsah \n");
	printf("-w --wait <ms> -- dodatečná informace pro Váš nástroj jaké je maximální přípustné RTT \n");

}

//funkcia na zistenie ci je argument cislo
int je_to_cislo(char *s){
	int i;
	while(*s != 0){
		if (!isdigit(*s++)){
			return 0;
		}
	}
	return 1;
}

//struktura pre argumenty
typedef struct{
	bool u;
	bool t;
	bool help;
	int interface;
	int network;
	long int port;
	long int wait;
}Arguments;

//funkcia na kontrolu validity argumentov
int arguments(int argc, char *argv[], Arguments *arguments){
	if((argc == 2) && ((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))){
		arguments->help = true;
		return EXIT_SUCCESS;
	}

	if(argc == 1){
		return EXIT_FAILURE;
	}

	char *pEnd;
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-p")){
			if((je_to_cislo(argv[i+1]) == 0) || arguments->port != 0){
				return EXIT_FAILURE;
			}
			arguments->port = strtol(argv[i+1],&pEnd, 10);
			if(arguments->port < 1 || arguments->port > 65535){
				return EXIT_FAILURE;
			}
			i++;
		}
		else if(!strcmp(argv[i], "-t")){
			if(arguments->t == true){
				return EXIT_FAILURE;
			}
			arguments->t = true;
		}
		else if(!strcmp(argv[i], "-u")){
			if(arguments->u == true){
				return EXIT_FAILURE;
			}
			arguments->u = true;
		}
		else if(!strcmp(argv[i], "-w")){
			if((je_to_cislo(argv[i+1]) == 0) || arguments->wait != 0){
				return EXIT_FAILURE;
			}
			arguments->wait = strtol(argv[i+1],&pEnd, 10);
			i++;
		}
		else if(!strcmp(argv[i], "-i")){
			if(arguments->interface != 0){
				return EXIT_FAILURE;
			}
			arguments->interface = ++i;
		}
		else if(!strcmp(argv[i], "-n")){
			if(arguments->network != 0){
				return EXIT_FAILURE;
			}
			arguments->network = ++i;
		}
		else{
			return EXIT_FAILURE;
		}		

	}
	return EXIT_SUCCESS;	
}

int main(int argc, char *argv[]){
	bool interface_set = false;
	Arguments argumenty;
	argumenty.port = 0;
	argumenty.wait = 0;
	argumenty.u = false;
	argumenty.t = false;
	argumenty.help = false;
	argumenty.interface = 0;
	argumenty.network = 0;

	/*int port_start = 1;
	int port_end = 200;*/


	//arguments
	if(arguments(argc,argv, &argumenty)){
		fprintf((stderr), "Wrong arguments!\n");
		return 1;
	}

	//help
	if(argumenty.help == true){
		printhelp();
		return 0;
	}

	/*if(argumenty.port != 0){

	}*/

	//argument network nezadany
	if(argumenty.network == 0){
		fprintf((stderr), "Wrong arguments! no network\n");
		return 1;
	}
	char *network = argv[argumenty.network];

	

	if(argumenty.interface != 0){
		interface_set = true;
	}
	char *interface = argv[argumenty.interface];
	

	/*cout << "port:" << argumenty.port << endl;
	cout << "wait:" << argumenty.wait << endl;
	cout << "int:" << interface << endl;
	cout << "network:" << network << endl;*/

	if(argumenty.u == true){
		cout << "udp" << endl;
	}
	if(argumenty.t == true){
		cout << "tcp" << endl;
	}

	string str = string(network);


	string ip_address ;
	string mask ;
	bool lomitko = false;
	for(int i = 0; i < strlen(network) ; i++){
		if(network[i] == '/'){
			lomitko = true;
			for(int x = 0; x < i ; x++){
				ip_address += network[x]; 
			}
			mask += network[i+1];
			mask += network[i+2];
		}
	}

	//kontrola validneho argumentu -n zadane lomitko
	if(lomitko == false){
		fprintf((stderr), "Wrong -n\n");
		return 1;
	}

	int mask_int = atoi(mask.c_str());
	const char * ip_char = ip_address.c_str();

	cout << "ip adresa : " << ip_address << endl;

	


	struct sockaddr_in sa;

	

	//kontrola validnej IP adresy
	if(inet_pton(AF_INET, ip_char , &(sa.sin_addr)) != 1){
		fprintf((stderr), "Wrong IP Address\n");
		return 1;
	}

	char ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(sa.sin_addr), ip_str, INET_ADDRSTRLEN);

	cout << "ip adresa koniec : " << ip_str << endl;

	//kontrola ci je maska v rozsahu 4-30
	if(mask_int < 4 || mask_int > 30){
		fprintf((stderr), "Wrong Mask\n");
		return 1;
	}
	cout << "maska : " << mask_int << endl;



	int dot1_index = 0;
	int dot2_index = 0;
	int dot3_index = 0;
	for(int i = 0 ; i < strlen(ip_str); i++){
		if(ip_str[i] == '.'){
			if(dot1_index == 0){
				dot1_index = i;
			}
			else if(dot2_index == 0){
				dot2_index = i;
			}
			else{
				dot3_index = i;
			}
		}
	}

	string byte1_s,byte2_s,byte3_s,byte4_s;

	for(int i = 0; i < dot1_index ; i++){
		byte1_s += ip_str[i];
	}
	for(int i = dot1_index + 1; i < dot2_index ; i++){
		byte2_s += ip_str[i];
	}
	for(int i = dot2_index + 1; i < dot3_index ; i++){
		byte3_s += ip_str[i];
	}
	for(int i = dot3_index + 1; i < strlen(ip_str) ; i++){
		byte4_s += ip_str[i];
	}


	int byte1,byte2,byte3,byte4;
	byte1 = atoi(byte1_s.c_str());
	byte2 = atoi(byte2_s.c_str());
	byte3 = atoi(byte3_s.c_str());
	byte4 = atoi(byte4_s.c_str());

	cout << "1  " << byte1 << endl;
	cout << "2  " << byte2 << endl;
	cout << "3  " << byte3 << endl;
	cout << "4  " << byte4 << endl;

	int final1=byte1;
	int final2=byte2;
	int final3=byte3;
	int final4=byte4;

	int end1=byte1;
	int end2=byte2;
	int end3=byte3;
	int end4=byte4;


	int maska_na_cislo;
	int mask_help = mask_int;
	


	int mask_byte;
	
	if(mask_int <= 8){
		mask_byte = 1;
	}
	else if(mask_int <= 16){
		mask_byte = 2;
		mask_help = mask_help - 8;
	}
	else if(mask_int <= 24){
		mask_byte = 3;
		mask_help = mask_help - 16;
	}
	else{
		mask_byte = 4;
		mask_help = mask_help - 24;
	}

	//zistenie networku podla masky
	if(mask_help == 8 ){maska_na_cislo = 255;}
	else if(mask_help == 7 ){maska_na_cislo = 254;}
	else if(mask_help == 6 ){maska_na_cislo = 252;}
	else if(mask_help == 5 ){maska_na_cislo = 248;}
	else if(mask_help == 4 ){maska_na_cislo = 240;}
	else if(mask_help == 3 ){maska_na_cislo = 224;}
	else if(mask_help == 2 ){maska_na_cislo = 192;}
	else if(mask_help == 1 ){maska_na_cislo = 128;}
	else{
		fprintf((stderr), "Wrong Mask\n");
		return 1;
	}


	if(mask_byte == 1){
		final1 = byte1 & maska_na_cislo;
		end1 = byte1 | (255 - maska_na_cislo);
		final2 = 0;
		final3 = 0;
		final4 = 0;

		end2=255;
		end3=255;
		end4=255;
	}
	if(mask_byte == 2){
		final2 = byte2 & maska_na_cislo;
		end2 = byte2 | (255 - maska_na_cislo);
		final3 = 0;
		final4 = 0;

		end3=255;
		end4=255;
	}
	if(mask_byte == 3){
		final3 = byte3 & maska_na_cislo;
		end3 = byte3 | (255 - maska_na_cislo);
		final4 = 0;

		end4=255;
	}
	if(mask_byte == 4){
		final4 = byte4 & maska_na_cislo;
		end4 = byte4 | (255 - maska_na_cislo);
	}


	
	cout << "mask byte : " << mask_byte << endl;
	cout << "1  " << final1 << endl;
	cout << "2  " << final2 << endl;
	cout << "3  " << final3 << endl;
	cout << "4  " << final4 << endl;

	/*string final_network;

	stringstream convert;
	convert << final1;
	final_network += convert.str();
	convert.str("");

	final_network += '.';

	
	convert << final2;
	final_network += convert.str();
	convert.str("");

	final_network += '.';

	
	convert << final3;
	final_network += convert.str();
	convert.str("");

	final_network += '.';

	
	convert << final4;
	final_network += convert.str();
	convert.str("");

	cout << "final  " << final_network << endl;

	//vysledna adresa siete v ktorej sa budu skumat ip adresy
	const char * network_address = final_network.c_str();
	
	cout << "final network char *  " << network_address << endl;*/

	
	// vypis
	cout << " byte1 - start : " << final1 << " end : " << end1 << endl;
	cout << " byte1 - start : " << final2 << " end : " << end2 << endl;
	cout << " byte1 - start : " << final3 << " end : " << end3 << endl;
	cout << " byte1 - start : " << final4 << " end : " << end4 << endl;


	struct timeval timeout;
	if(argumenty.wait > 0){
			
			timeout.tv_sec = argumenty.wait /1000;
    		timeout.tv_usec = (argumenty.wait % 1000) * 1000;	
	}

	stringstream convert;
	string str_ip_for_scan;
	const char * char_ip_for_scan;
	for(int i = final1 ; i <= end1 ; i++){
		for(int j = final2; j <= end2; j++){
			for(int k = final3; k <= end3; k++){
				for(int l = final4; l <= end4 ; l++){
					//cout << "IP address: " << i << "." << j << "." << k << "." << l << endl; 
					//vsetky ip adresy ktore mam prechadzat na danom networku 
					convert << i;
					str_ip_for_scan += convert.str();
					convert.str("");

					str_ip_for_scan += '.';

					convert << j;
					str_ip_for_scan += convert.str();
					convert.str("");

					str_ip_for_scan += '.';

					convert << k;
					str_ip_for_scan += convert.str();
					convert.str("");

					str_ip_for_scan += '.';

					convert << l;
					str_ip_for_scan += convert.str();
					convert.str("");

					//
					char_ip_for_scan = str_ip_for_scan.c_str();

					//cout << "IP address - char: " << char_ip_for_scan << endl;

					





					//vymazanie a priprava na dalsiu IP
					str_ip_for_scan = "";
					char_ip_for_scan = str_ip_for_scan.c_str();


					
				}
			}
		}
	}

	//http://www.matveev.se/cpp/portscaner.htm

	int port_start = 1;
	int port_end = 200;

	if(argumenty.port != 0){
		port_start = argumenty.port;
		port_end = argumenty.port;
	}

	for(int x = port_start ; x <= port_end; x++){
		
		int portno = x;
		//const char *hostname = "10.190.22.160";

		// ip_char je ip adresa z masky
		const char *hostname = "localhost";
		const char *protocol = "tcp";

		int sockfd;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0 ){
			fprintf((stderr), "socket:  - %d\n" , x);
			return 1;
		}
		if(argumenty.wait > 0){
			if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
				fprintf((stderr), "setsockopt:  \n" );
				return 1;
			}
			if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
				fprintf((stderr), "setsockopt:  \n" );
				return 1;
			}
		}

		server = gethostbyname(hostname);
		if(server == NULL){
			fprintf((stderr), "gethostbyname:  \n" );
			return 1;
		}
		

		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

		cout << x << " server " << server->h_name << endl; 

		serv_addr.sin_port = htons(portno);



		if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0){
			struct servent *srvport = getservbyport(htons(x), protocol);
			/*if(srvport != NULL){
				cout << "TCP " << srvport->s_name << endl;
			}*/
			cout << "TCP " << x << endl;
			
		}
		
		
			
		

		close(sockfd);

	}

	

	return 0;

	



	
}