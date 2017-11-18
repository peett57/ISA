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
#include <signal.h>

#include <netinet/ip.h> 
#include <sys/ioctl.h>  
#include <bits/ioctls.h>  
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <net/ethernet.h>
#include <asm/types.h>
#include <math.h>

#include <fcntl.h>
#include <sys/select.h>

 #include <ifaddrs.h>
 #include <linux/if_link.h>

#include <netinet/ip_icmp.h>

#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define PROTOCOL_TYPE 0x800
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZE 60

#define IP4_HDRLEN 20         // IPv4 header length
#define ICMP_HDRLEN 8         // ICMP header length for echo request, excludes data





using namespace std;


// struktura pre arp hlavicku
struct arp_header
{
        unsigned short hardware_type;
        unsigned short protocol_type;
        unsigned char hardware_len;
        unsigned char  protocol_len;
        unsigned short opcode;
        unsigned char sender_mac[MAC_LENGTH];
        unsigned char sender_ip[IPV4_LENGTH];
        unsigned char target_mac[MAC_LENGTH];
        unsigned char target_ip[IPV4_LENGTH];
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
int jetocislo(char *s){
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

int tcp_check(const char * ip, long int port_arg, long int wait){
	struct hostent *he;
	struct in_addr **addr_list;
	
	/*if((he = gethostbyname(ip)) == NULL){
		fprintf((stderr), "gethostbyname: TCP  \n" );
		return 1;
	}

	string test;
	addr_list = (struct in_addr **) he->h_addr_list;
	for(int i = 0; addr_list[i] != NULL; i++){
		test = inet_ntoa(*addr_list[i]);
	}
	cout << "test :" << test << endl;*/


	//http://www.matveev.se/cpp/portscaner.htm

	struct timeval timeout;

	if(wait > 0){
			
			timeout.tv_sec = wait /1000;
    		timeout.tv_usec = (wait % 1000) * 1000;	
	}

	int port_start = 1;
	//int port_end = 65535;
	int port_end = 200;

	if(port_arg != 0){
		port_start = port_arg;
		port_end = port_arg;
	}

	for(int x = port_start ; x <= port_end; x++){
		
		int portno = x;
		

		
		const char *hostname = ip;
		const char *protocol = "tcp";

		int sockfd;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0 ){
			fprintf((stderr), "socket:  - %d\n" , x);
			return 1;
		}
		if(wait > 0){
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



		serv_addr.sin_port = htons(portno);



		if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0){
			struct servent *srvport = getservbyport(htons(x), protocol);
			 
			cout << ip << " TCP " << x << endl;
			
		}
		
		
			
		

		close(sockfd);

	}
	return 0;
}

int udp_check(const char * ip, long int port_arg, long int wait){
	//open UDP socket
	unsigned char buffer[BUF_SIZE];
	int sendsd, recvsd;
	if((sendsd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		fprintf((stderr), "socket: DGRAM - \n" );
		return 1;
	}
	//open receive socket pre ICMP response packet
	if((recvsd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		fprintf((stderr), "socket: RAW - \n" );
		return 1;
	}

	int port_start = 1;
	int port_end = 200;

	if(port_arg != 0){
		port_start = port_arg;
		port_end = port_arg;
	}

	for(int x = port_start ; x <= port_end; x++){
		int portno = x;
		
		const char *hostname = ip;

		struct sockaddr_in serv_addr;
		struct hostent *server;

		server = gethostbyname(hostname);
		if(server == NULL){
			fprintf((stderr), "gethostbyname:  \n" );
			return 1;
		}

		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr = *((struct in_addr *)server->h_addr);



		serv_addr.sin_port = htons(portno);

		memset(buffer,0x00,60);

		if(sendto(sendsd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
			fprintf((stderr), "sendto: udp %d \n", x );
			return 1;
		}




		struct timeval timeout;
		if(wait > 0){
				
				timeout.tv_sec = wait /1000;
	    		timeout.tv_usec = (wait % 1000) * 1000;	
		}
		else{
			fprintf((stderr), "pri UDP musi byt wait:   \n");
			return 1;
		}

		while(1){
			int length = 0;

    		
    		/*length = recvfrom(recvsd, &buffer, sizeof(buffer), 0x0, NULL, NULL);
    		if(length == -1){
    			fprintf((stderr), "recvfrom:  %d \n", x );
				return 1;
    		}
    		
    		
    		
    		struct iphdr *iphdr = (struct ip *)buffer;
    		int iplen = iphdr->ip_hl << 2;

    		struct icmphdr *icmp = (struct icmp *)(buffer + iplen);

    		if((icmp->icmp_type == ICMP_UNREACH) && (icmp->icmp_code == ICMP_UNREACH_PORT)){
    			break;
    		}else{
    			cout << ip << " UDP " << x << endl;
    		}*/


    		fd_set set;
   			FD_ZERO(&set);
   			FD_SET(recvsd, &set);
   			int rv = 0;
   			rv = select(recvsd + 1 , &set, NULL, NULL, &timeout);
   			if(rv == -1){
   				fprintf((stderr), "select -1:  %d\n", x );
				return 1;
   			}
   			else if(rv == 0){
   				//fprintf((stderr), "timeout:  %d.%d.%d.%d\n", i,j,k,l );

   				
   				break;

   			}else{
   				//fprintf((stderr), "no timeout:  %d.%d.%d.%d\n", i,j,k,l );

   				length = recvfrom(recvsd, &buffer, sizeof(buffer), 0x0, NULL, NULL);

   				if (length == -1){
                    fprintf((stderr), "receive: %d\n", x );
					return 1;
                }

                struct ip *iphdr = (struct ip *)buffer;
	    		int iplen = iphdr->ip_hl << 2;

	    		struct icmp *icmp = (struct icmp *)(buffer + iplen);

	    		if((icmp->icmp_type == ICMP_UNREACH) && (icmp->icmp_code == ICMP_UNREACH_PORT)){
	    			break;
	    		}else{
	    			cout << ip << " UDP " << x << endl;
	    		}
                
            
   			}

      			

		}
		

		
	}
	close(sendsd);
	close(recvsd);
	cout << "UDP" << endl;
	return 0;
}


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
			if(argc == i +1 || (jetocislo(argv[i+1]) == 0) || arguments->port != 0){
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
			if(argc == i +1 || (jetocislo(argv[i+1]) == 0) || arguments->wait != 0  ){
				return EXIT_FAILURE;
			}
			arguments->wait = strtol(argv[i+1],&pEnd, 10);
			i++;
		}
		else if(!strcmp(argv[i], "-i")){
			if(argc == i +1 || arguments->interface != 0){
				return EXIT_FAILURE;
			}
			arguments->interface = ++i;
		}
		else if(!strcmp(argv[i], "-n")){
			if(argc == i +1 || arguments->network != 0){
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

	
	char *interface;
	if(argumenty.interface == 0){
		interface_set = false;
		//zatial takto
		cout << "nezadany interface - nastavenie defaultneho int na eth1" << endl;
		
		
	}else{
		interface_set = true;
		interface = argv[argumenty.interface];
		
	}
		
	

	/*cout << "port:" << argumenty.port << endl;
	cout << "wait:" << argumenty.wait << endl;
	cout << "int:" << interface << endl;
	cout << "network:" << network << endl;*/

	/*if(argumenty.u == true){
		cout << "udp" << endl;
	}
	if(argumenty.t == true){
		cout << "tcp" << endl;
	}*/

	struct timeval timeout;
	if(argumenty.wait > 0){
			
			timeout.tv_sec = argumenty.wait /1000;
    		timeout.tv_usec = (argumenty.wait % 1000) * 1000;	
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
	
	// vypis
	cout << "byte1 - start : " << final1 << " end : " << end1 << endl;
	cout << "byte1 - start : " << final2 << " end : " << end2 << endl;
	cout << "byte1 - start : " << final3 << " end : " << end3 << endl;
	cout << "byte1 - start : " << final4 << " end : " << end4 << endl;

// ziskanie IP adresy MAC z daneho interfacu 
	int sd;
	int ifindex;
	struct ifreq ifr;

	sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd == -1) {
		fprintf((stderr), "socket:  \n");
		return 1;
	}

	//bude sa robit na konkretnom interface vzchodzi interface je eth1
	if(interface_set == true){
		strcpy(ifr.ifr_name, interface);
	}else{
		strcpy(ifr.ifr_name, "eth1");
	}
	

	//ethernet index
	if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1) {
		fprintf((stderr), "SIOCGIFINDEX  - index interfacu - -i \n" );
		return 1;
	}
	ifindex = ifr.ifr_ifindex;

	//ziskanie mojej IP adresy na danom interface
	if (ioctl(sd, SIOCGIFADDR, &ifr) == -1) {
		fprintf((stderr), "SIOCGIFINDEX  - moja ip adresa na interface\n" );
		return 1;
	}
	char * my_addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	
	//cout << my_addr << endl;


	// ziskanie MAC
	if (ioctl(sd, SIOCGIFHWADDR, &ifr) == -1) {
		perror("SIOCGIFINDEX - ziskanie MAC na interface");
		exit(1);
	}

	close(sd);

	//spracovanie adresy a zistenie ci je moja adresa v sieti
	dot1_index = 0;
	dot2_index = 0;
	dot3_index = 0;
	for(int i = 0 ; i < strlen(my_addr); i++){
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

	string byte1_s_myaddr,byte2_s_myaddr,byte3_s_myaddr,byte4_s_myaddr;

	for(int i = 0; i < dot1_index ; i++){
		byte1_s_myaddr += my_addr[i];
	}
	for(int i = dot1_index + 1; i < dot2_index ; i++){
		byte2_s_myaddr += my_addr[i];
	}
	for(int i = dot2_index + 1; i < dot3_index ; i++){
		byte3_s_myaddr += my_addr[i];
	}
	for(int i = dot3_index + 1; i < strlen(my_addr) ; i++){
		byte4_s_myaddr += my_addr[i];
	}


	int byte1_myaddr,byte2_myaddr,byte3_myaddr,byte4_myaddr;
	byte1_myaddr = atoi(byte1_s_myaddr.c_str());
	byte2_myaddr = atoi(byte2_s_myaddr.c_str());
	byte3_myaddr = atoi(byte3_s_myaddr.c_str());
	byte4_myaddr = atoi(byte4_s_myaddr.c_str());

	/*cout<< byte1_myaddr << endl;
	cout<< byte2_myaddr << endl;
	cout<< byte3_myaddr << endl;
	cout<< byte4_myaddr << endl;*/

	// ak je adresa v sieti tak nastavim scan lokalnej adresy
	bool local_network = false;
	for(int i = final1 ; i <= end1 ; i++){
		for(int j = final2; j <= end2; j++){
			for(int k = final3; k <= end3; k++){
				for(int l = final4; l <= end4 ; l++){
					if(i == byte1_myaddr && j == byte2_myaddr && k == byte3_myaddr && l == byte4_myaddr){
						local_network = true;
					}
				}
			}
		}
	}

	if(local_network == true){
		cout << "lokalna siet - arp scan" << endl;
	}else{
		cout << "mimo lokalnu siet - icmp scan" << endl;;
	}
	
	bool closed = true;
	if(closed == false){
	//if(local_network == true){
		stringstream convert;
		string str_ip_for_scan;
		const char * char_ip_for_scan;
		for(int i = final1 ; i <= end1 ; i++){
			for(int j = final2; j <= end2; j++){
				for(int k = final3; k <= end3; k++){
					for(int l = final4; l <= end4 ; l++){
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

												
						
						unsigned char target_ip[4] = {i,j,k,l};
						
						int ret,length = 0;

						unsigned char buffer[BUF_SIZE];
						struct ethhdr *send_req = (struct ethhdr *)buffer;
						struct ethhdr *rcv_resp= (struct ethhdr *)buffer;
						struct arp_header *arp_req = (struct arp_header *)(buffer+ETH2_HEADER_LEN);
						struct arp_header *arp_resp = (struct arp_header *)(buffer+ETH2_HEADER_LEN);
						struct sockaddr_ll socket_address;


						memset(buffer,0x00,60);


						for (int index = 0; index < 6; index++){
							send_req->h_dest[index] = (unsigned char)0xff;
							arp_req->target_mac[index] = (unsigned char)0x00;
							// doplnenie source MAC do hlavicky
							send_req->h_source[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];
							arp_req->sender_mac[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];
							socket_address.sll_addr[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];

						}


						//priprava sockaddr_ll
						socket_address.sll_family = AF_PACKET;
						socket_address.sll_protocol = htons(ETH_P_ARP);
						socket_address.sll_ifindex = ifindex;
						socket_address.sll_hatype = htons(ARPHRD_ETHER);
						socket_address.sll_pkttype = (PACKET_BROADCAST);
						socket_address.sll_halen = MAC_LENGTH;
						socket_address.sll_addr[6] = 0x00;
						socket_address.sll_addr[7] = 0x00;

						// protocol pre packet
						send_req->h_proto = htons(ETH_P_ARP);

						//vytvorenie arp requestu
						arp_req->hardware_type = htons(HW_TYPE);
						arp_req->protocol_type = htons(ETH_P_IP);
						arp_req->hardware_len = MAC_LENGTH;
						arp_req->protocol_len =IPV4_LENGTH;
						arp_req->opcode = htons(ARP_REQUEST);
						unsigned char source_ip[4] = {byte1_myaddr,byte2_myaddr,byte3_myaddr,byte4_myaddr};

						for(int index=0;index<5;index++)
					    {
					            arp_req->sender_ip[index]=(unsigned char)source_ip[index];
					    }

	        			// doplnenie target ip adresy do arp requestu
				        for(int index=0;index<5;index++)
				        {
			                arp_req->target_ip[index]=(unsigned char)target_ip[index];
				        }




				        // vytvorenie raw socketu na request
				        if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
						    fprintf((stderr), "socket:  %d.%d.%d.%d\n", i,j,k,l );
							return 1;
						}
							
						buffer[32] = 0x00;

						//odoslanie arp requestu 
						ret = sendto(sd, buffer, 42, 0, (struct  sockaddr*)&socket_address, sizeof(socket_address));
				        if (ret == -1)
				        {
				            fprintf((stderr), "sendto:  %d.%d.%d.%d\n", i,j,k,l );
							return 1;
				        }

				       	memset(buffer,0x00,60);

				       	//prijatie odpovedi
				       	while(1){


				       		//length = recvfrom(sd, buffer, BUF_SIZE, 0, NULL, NULL);
				       		if(argumenty.wait > 0){
				       			fd_set set;
				       			FD_ZERO(&set);
				       			FD_SET(sd, &set);
				       			int rv = 0;
				       			rv = select(sd + 1 , &set, NULL, NULL, &timeout);
				       			if(rv == -1){
				       				fprintf((stderr), "select -1:  %d.%d.%d.%d\n", i,j,k,l );
									return 1;
				       			}
				       			else if(rv == 0){
				       				//fprintf((stderr), "timeout:  %d.%d.%d.%d\n", i,j,k,l );

				       				
				       				break;

				       			}else{
				       				//fprintf((stderr), "no timeout:  %d.%d.%d.%d\n", i,j,k,l );

				       				length = recvfrom(sd, buffer, BUF_SIZE, 0, NULL, NULL);

				       				if (length == -1){
					                    fprintf((stderr), "receive:  %d.%d.%d.%d\n", i,j,k,l );
										return 1;
					                }
					                
					            
				       			}
				       		}else{
				       			length = recvfrom(sd, buffer, BUF_SIZE, 0, NULL, NULL);

					       		if (length == -1){
				                    fprintf((stderr), "receive:  %d.%d.%d.%d\n", i,j,k,l );
									return 1;
				                }

				       		}

			                /*if (length == -1){
			                    fprintf((stderr), "receive:  %d.%d.%d.%d\n", i,j,k,l );
								return 1;
			                }*/
			                if(htons(rcv_resp->h_proto) == PROTO_ARP){

				                if(arp_resp->sender_ip[0] == i){
			                		if(arp_resp->sender_ip[1] == j){
			                			if(arp_resp->sender_ip[2] == k){
			                				if(arp_resp->sender_ip[3] == l){
			                					cout << char_ip_for_scan << endl;
			                					if(argumenty.t == true){
			                						// HP nejake random tu mam
			                						if(i == 10 && j == 190 && k == 20 && l == 159){
			                							;
			                						}else{
				                						if(tcp_check(char_ip_for_scan,argumenty.port,argumenty.wait) != 0){
				                							fprintf((stderr), "TCP  \n" );
															return 1;
				                						}
				                					}
			                					}
			                				}
			                			}
			                		}
			                	}
			                	
			                }
			               	close(sd);
			               	
		                	break;

			                

				       	}
				       	close(sd);
				       	

				       	if(10 == i){
	                		if(190 == j){
	                			if(23 == k){
	                				if(178 == l){
	                					cout << char_ip_for_scan << endl;
	                					if(argumenty.t == true){
	                						if(tcp_check(char_ip_for_scan,argumenty.port,argumenty.wait) != 0){
	                							fprintf((stderr), "TCP  \n" );
												return 1;
	                						}
	                					}
	                				}
	                			}
	                		}
	                	}


				       	
						//vymazanie a priprava na dalsiu IP
						str_ip_for_scan = "";
						char_ip_for_scan = str_ip_for_scan.c_str();


						
					}
				}
			}
		}
	}
	else{
		udp_check("10.190.22.250",argumenty.port,argumenty.wait); 
		//tcp_check("10.0.2.3",argumenty.port,argumenty.wait); 
		//cout << "closed" << endl;
	}


	/*struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host_test[NI_MAXHOST];
	char *interfaces;

	if (getifaddrs(&ifaddr) == -1) {
        fprintf((stderr), "getifaddrs  \n" );
		return 1;
   }
   
   for(ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++){

   	if(ifa->ifa_addr == NULL){
   		continue;
   	}
   	family = ifa->ifa_addr->sa_family;
   	
   	if(family == AF_INET){
   		
   		cout << ifa->ifa_name << endl;
   		
   		
   	}



   }
   freeifaddrs(ifaddr);
	*/
	
  
   
	

	return 0;

	



	
}