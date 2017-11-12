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
	bool help;
	int interface;
	int network;
	long int port;
	long int wait;
	char fault;
}Arguments;


int arguments(int argc, char *argv[], Arguments *arguments){
	if((argc == 2) && ((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))){
		arguments->help = true;
		return EXIT_SUCCESS;
	}

	if(argc == 1){
		arguments->fault = 'no args';
		return EXIT_FAILURE;
	}

	char *pEnd;
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-p")){
			if((je_to_cislo(argv[i+1]) == 0) || arguments->port != 0){
				arguments->fault = "port not a number";
				return EXIT_FAILURE;
			}
			arguments->port = strtol(argv[i+1],&pEnd, 10);
			if(arguments->port < 1 || arguments->port > 65535){
				arguments->fault = "port out of range";
				return EXIT_FAILURE;
			}
			i++;
		}
		else if(!strcmp(argv[i], "-t")){
			if(arguments->t == true){
				arguments->fault = "repeat of -t";
				return EXIT_FAILURE;
			}
			arguments->t = true;
		}
		else if(!strcmp(argv[i], "-u")){
			if(arguments->u == true){
				arguments->fault = "repeat of -u";
				return EXIT_FAILURE;
			}
			arguments->u = true;
		}
		else if(!strcmp(argv[i], "-w")){
			if((je_to_cislo(argv[i+1]) == 0) || arguments->wait != 0){
				arguments->fault = "wait not a number";
				return EXIT_FAILURE;
			}
			arguments->wait = strtol(argv[i+1],&pEnd, 10);
			i++;
		}
		else if(!strcmp(argv[i], "-i")){
			if(arguments->interface != 0){
				arguments->fault = "interface repeat";
				return EXIT_FAILURE;
			}
			arguments->interface = ++i;
		}
		else if(!strcmp(argv[i], "-n")){
			if(arguments->network != 0){
				arguments->fault = "netwok repeat";
				return EXIT_FAILURE;
			}
			arguments->network = ++i;
		}
		else{
			arguments->fault = "other";
			return EXIT_FAILURE;
		}		

	}
	return EXIT_SUCCESS;	
}

int main(int argc, char *argv[]){
	Arguments argumenty;
	argumenty.port = 0;
	argumenty.wait = 0;
	argumenty.u = false;
	argumenty.t = false;
	argumenty.help = false;
	argumenty.interface = 0;
	argumenty.network = 0;
	argumenty.fault = "";

	//arguments
	if(arguments(argc,argv, &argumenty)){
		fprintf((stderr), "Wrong arguments! - %s \n" , argumenty.fault  );
		return 1;
	}

	if(argumenty.help == true){
		printhelp();
		return 0;
	}

	char *interface = argv[argumenty.interface];
	char *network = argv[argumenty.network];

	cout << "port:" << argumenty.port << endl;
	cout << "wait:" << argumenty.wait << endl;
	cout << "int:" << interface << endl;
	cout << "network:" << network << endl;

	if(argumenty.u == true){
		cout << "udp" << endl;
	}
	if(argumenty.t == true){
		cout << "tcp" << endl;
	}

	return 0;

	
}