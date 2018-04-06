/*
 * client.c
 *	UDP+IP+Eth RAW socket
 *  Created on: 5 апр. 2018 г.
 *      Author: jake
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h> // if_nametoindex()
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

unsigned short csum(unsigned short*, int);

struct udp_header{ //Заголовок UDP
	short source;
	short dest;
	short len;
	short check;
}udph;
struct ip_header{ // Заголовок IP
 	u_char ip_vhl;  /* версия << 4 | длина заголовка >> 2 */
    u_char ip_tos;  /* тип службы */
    u_short ip_len;  /* общая длина */
    u_short ip_id;  /* идентификатор */
    u_short ip_off;  /* поле фрагмента смещения */
    #define IP_RF 0x8000  /* reserved флаг фрагмента */
    #define IP_DF 0x4000  /* dont флаг фрагмента */
    #define IP_MF 0x2000  /* more флаг фрагмента */
    #define IP_OFFMASK 0x1fff /* маска для битов фрагмента */
    u_char ip_ttl;  /* время жизни */
    u_char ip_p;  /* протокол #include <net/ethernet.h> /* the L2 protocols */
    u_short ip_sum;  /* контрольная сумма */
    struct in_addr ip_src,ip_dst; /* адрес источника и адрес назначения */
} iph;
#define IP_HL(ip)  (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)  (((ip)->ip_vhl) >> 4)

int main(void)
{
	int packet_sock,i;
	int yes=1;
	char datagram[248];
	char payload_ip[256];
	struct sockaddr_ll sll;
	struct ethhdr eth;

	memset(datagram,0,sizeof(datagram));
	memset(payload_ip,0,sizeof(payload_ip));
	memset(&sll,0,sizeof(sll));

	strcat(datagram,"Hello RAW Layer 2");

	//Структура Link Layer
	sll.sll_family = htons(AF_PACKET);
	sll.sll_ifindex = if_nametoindex("enp6s1");
	sll.sll_halen = 6;
	sll.sll_addr[0] = 0xfc;
	sll.sll_addr[1] = 0xaa;
	sll.sll_addr[2] = 0x14;
	sll.sll_addr[3] = 0x83;
	sll.sll_addr[4] = 0xb6;
	sll.sll_addr[5] = 0xef;

    //UDP header
    udph.source = htons(40004);
    udph.dest = htons(50004);
    udph.len = htons(sizeof(udph) + strlen(datagram));
    udph.check = 0;

    //IP header
    iph.ip_vhl = 4;
    iph.ip_vhl = iph.ip_vhl<<4 | 5;
    iph.ip_tos = 0;
    iph.ip_len = htons(sizeof(iph)+sizeof(udph) + strlen(datagram));
    iph.ip_id = htons(444);
    iph.ip_off = htons((0&IP_RF) | (IP_DF) | (0&IP_MF));
    iph.ip_ttl=64;
    iph.ip_p = 17;
    inet_aton("192.168.0.235",&(iph.ip_dst));
    inet_aton("192.168.0.43",&(iph.ip_src));
    iph.ip_sum = csum ((unsigned short *) &iph, sizeof(iph));

    //ETHERNET header
    eth.h_dest[0]=0xfc;
    eth.h_dest[1]=0xaa;
    eth.h_dest[2]=0x14;
    eth.h_dest[3]=0x83;
    eth.h_dest[4]=0xb6;
    eth.h_dest[5]=0xef;

    eth.h_source[0]=0x00;
    eth.h_source[1]=0x0f;
    eth.h_source[2]=0xb0;
    eth.h_source[3]=0x93;
    eth.h_source[4]=0x00;
    eth.h_source[5]=0xad;

    eth.h_proto=htons(ETH_P_IP);

    // Сборка пакета
    memcpy(payload_ip,&eth,sizeof(eth));
    memcpy(payload_ip+sizeof(eth),&iph,sizeof(iph));
    memcpy(payload_ip+sizeof(eth)+sizeof(iph),&udph,sizeof(udph));
    memcpy(payload_ip+sizeof(eth)+sizeof(iph)+sizeof(udph),datagram,strlen(datagram));

    // Открытие и настройка L2 RAW socket
	if((packet_sock=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL))) < 0)
	{
		perror("socket error");
		exit(1);
	}

	// Отправка и получение назад пакета
	sendto(packet_sock,payload_ip,sizeof(eth)+sizeof(iph)+sizeof(udph)+strlen(datagram),
			0,(struct sockaddr*)&sll,sizeof(sll));

	recvfrom(packet_sock,payload_ip,sizeof(payload_ip),0,0,0);
	printf("%s\n",payload_ip+sizeof(eth)+sizeof(iph)+sizeof(udph));

	// Закрытие сокета
	close(packet_sock);
	return 0;
}
