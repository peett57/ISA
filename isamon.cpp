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
	int interface;
	int network;
	long int port;
	long int wait;
}Arguments;


int arguments(int argc, char *argv[], Arguments *arguments){
	if((argc == 2) && ((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))){
		printhelp();
		return EXIT_SUCCESS;
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
			arguments->t == true;
		}
		else if(!strcmp(argv[i], "-u")){
			if(arguments->u == true){
				return EXIT_FAILURE;
			}
			arguments->u == true;
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
	Arguments arguments;
	arguments.port = 0;
	arguments.wait = 0;
	arguments.u = false;
	arguments.t = false;
	arguments.interface = 0;
	arguments.network = 0;

	//arguments
	if(arguments(argc,argv, &arguments)){
		fprintf((stderr), "Wrong arguments!\n" );
		return EXIT_FAILURE;
	}

	cout << "port:" << arguments.port << endl;
	cout << "wait:" << arguments.wait << endl;
	cout << "int:" << arguments.interface << endl;
	cout << "network:" << arguments.network << endl;

	return EXIT_SUCCESS;

	
}