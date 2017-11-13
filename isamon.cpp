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




using namespace std;

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

//dsadasdasdasd
typedef struct{
	bool u;
	bool t;
	bool help;
	int interface;
	int network;
	long int port;
	long int wait;
}Arguments;


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

	/*cout << " index 0" << network[0] << endl;
	cout << " index 1" << network[1] << endl;
	cout << " velkost " << strlen(network) << endl;*/


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

	//kontrola ci je maska v rozsahu 1-32
	if(mask_int < 1 || mask_int > 32){
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

	string byte1,byte2,byte3,byte4;

	for(int i = 0; i < dot1_index ; i++){
		byte1 += ip_str[i];
	}
	for(int i = dot1_index + 1; i < dot2_index ; i++){
		byte2 += ip_str[i];
	}
	for(int i = dot2_index + 1; i < dot3_index ; i++){
		byte3 += ip_str[i];
	}
	for(int i = dot3_index + 1; i < strlen(ip_str) ; i++){
		byte4 += ip_str[i];
	}


	cout << "1  " << byte1 << endl;
	cout << "2  " << byte2 << endl;
	cout << "3  " << byte3 << endl;
	cout << "4  " << byte4 << endl;
	

	int mask_byte;
	int maska_na_cislo;
	if(mask_int <= 8){
		mask_byte = 1;
		byte2 = '0';
		byte3 = '0';
		byte4 = '0';
	}
	else if(mask_int <= 16){
		mask_byte = 2;
		mask_int = mask_int - 8;
		byte3 = '0';
		byte4 = '0';

	}
	else if(mask_int <= 24){
		mask_byte = 3;
		mask_int = mask_int - 16;
		byte4 = '0';
	}
	else{
		mask_byte = 4;
		mask_int = mask_int - 24;
	}

	cout << "mask byte : " << mask_byte << endl;
	cout << "1  " << byte1 << endl;
	cout << "2  " << byte2 << endl;
	cout << "3  " << byte3 << endl;
	cout << "4  " << byte4 << endl;

	
	if(mask_int == 1 ){maska_na_cislo = 127;}
	else if(mask_int == 2 ){maska_na_cislo = 63;}
	else if(mask_int == 3 ){maska_na_cislo = 31;}
	else if(mask_int == 4 ){maska_na_cislo = 15;}
	else if(mask_int == 5 ){maska_na_cislo = 7;}
	else if(mask_int == 6 ){maska_na_cislo = 3;}
	else if(mask_int == 7 ){maska_na_cislo = 1;}
	else if(mask_int == 8 ){maska_na_cislo = 0;}
	else{
		fprintf((stderr), "Wrong Mask\n");
		return 1;
	}







	return 0;

	



	
}