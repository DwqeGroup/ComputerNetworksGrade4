#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include "pcap.h"
#include "remote-ext.h"
#include "bittypes.h"
#include "ip6_misc.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Packet.lib")
#pragma comment(lib,"wpcap.lib")
#pragma warning(disable:4996)
typedef enum ethernet_frame_protocol
{
	IEEE = 0x0000, // 0x05DC   IEEE 802.3 长度
	EXP = 0x0101, // 0x01FF   实验
	XEROX_NS_IDP = 0x0600, //   XEROX NS IDP
	DLOG = 0x0661, //   DLOG
	IP = 0x0800, // 网际协议（IP）
	X_75Internet = 0x0801, //   X.75 Internet
	NBS_Internet = 0x0802, //   NBS Internet
	ECMA = 0x0803, // ECMA Internet
	Chaosnet = 0x0804, // Chaosnet
	X25_Level3 = 0x0805,   //X.25 Level 3
	ARP = 0x0806,    //地址解析协议（ARP ： Address Resolution Protocol）
	Frame_Relay_ARP = 0x0808,   //帧中继 ARP （Frame Relay ARP） [RFC1701]
	Raw_Frame_Relay = 0x6559,   //原始帧中继（Raw Frame Relay） [RFC1701]
	DARP = 0x8035,   // 动态 DARP （DRARP：Dynamic RARP）反向地址解析协议（RARP：Reverse Address Resolution Protocol）
	Novell_Netware_IPX = 0x8037,   //Novell Netware IPX
	EtherTalk = 0x809B,   //   EtherTalk
	IBM_SNA_Services = 0x80D5,   // IBM SNA Services over Ethernet
	AARP = 0x80F3,   //   AppleTalk 地址解析协议（AARP：AppleTalk Address Resolution Protocol）
	EAPS = 0x8100,   // 以太网自动保护开关（EAPS：Ethernet Automatic Protection Switching）
	IPX = 0x8137,   //    因特网包交换（IPX：Internet Packet Exchange）
	SNMP = 0x814C,   //简单网络管理协议（SNMP：Simple Network Management Protocol）
	IPV6 = 0x86DD,   //   网际协议v6 （IPv6，Internet Protocol version 6）
	PPP = 0x880B ,   // 点对点协议（PPP：Point-to-Point Protocol）
	GSMP = 0x880C,   //   通用交换管理协议（GSMP：General Switch Management Protocol）
	MPLS_unicast = 0x8847,   //   多协议标签交换（单播） MPLS：Multi-Protocol Label Switching <unicast>）
	MPLS_multicast = 0x8848,   //   多协议标签交换（组播）（MPLS, Multi-Protocol Label Switching <multicast>）
	PPPoE_DS = 0x8863,   //   以太网上的 PPP（发现阶段）（PPPoE：PPP Over Ethernet <Discovery Stage>）
	PPPoE_SS = 0x8864,   //   以太网上的 PPP（PPP 会话阶段） （PPPoE，PPP Over Ethernet<PPP Session Stage>）
	LWAPP = 0x88BB,   // 轻量级访问点协议（LWAPP：Light Weight Access Point Protocol）
	LLDP = 0x88CC,   // 链接层发现协议（LLDP：Link Layer Discovery Protocol）
	EAP = 0x8E88,   // 局域网上的 EAP（EAPOL：EAP over LAN）
	Loopback = 0x9000,   // 配置测试协议（Loopback）
	VLAN_Tag1 = 0x9100,   //   VLAN 标签协议标识符（VLAN Tag Protocol Identifier）
	VLAN_Tag2 = 0x9200,   //VLAN 标签协议标识符（VLAN Tag Protocol Identifier）
	MAINSTAIN = 0xFFFF, // 保留
} ETHERNET_FRAME_PROTOCOL;

//以太网帧类型
typedef struct ethernet_frame_type
{
	ETHERNET_FRAME_PROTOCOL type;
	char description[50];

}ETHERNET_FRAME_TYPE;

ETHERNET_FRAME_TYPE eth_match[50];
//以太网数据帧头部结构
typedef struct tagDLCHeader
{
	unsigned char      DesMAC[6];//目标硬件地址
	unsigned char      SrcMAC[6];//源硬件地址
	unsigned short     Ethertype;//以太网类型
} DLCHEADER, *PDLCHEADER;

//IP地址
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

//IPv4头部
typedef struct ip_header
{
	u_char ver_ihl; // 版本 (4 bits) + 首部长度 (4 bits)
	u_char tos; // 服务类型(Type of service)
	u_short tlen; // 总长(Total length)
	u_short identification; // 标识(Identification)
	u_short flags_fo; // 标志位(Flags) (3 bits) + 段偏移量(Fragment offset) (13 bits)
	u_char ttl; // 存活时间(Time to live)
	u_char proto; // 协议(Protocol)
	u_short crc; // 首部校验和(Header checksum)
	ip_address saddr; // 源地址(Source address)
	ip_address daddr; // 目的地址(Destination address)
	u_int op_pad; // 选项与填充(Option + Padding)
}ip_header;

//TCP头部
typedef struct _TCPHeader    //20个字节
{   
	USHORT    sourcePort;        //16位源端口号
	USHORT    destinationPort;//16位目的端口号
	ULONG    sequenceNumber;    //32位序列号
	ULONG    acknowledgeNumber;//32位确认号
	USHORT    dataoffset;        //4位首部长度/6位保留字/6位标志位
	USHORT    windows;        //16位窗口大小
	USHORT    checksum;        //16位校验和
	USHORT    urgentPointer;    //16位紧急数据偏移量
}TCPHeader,*PTCPHeader;

//UDP头部
typedef struct _UDPHeader
{
	USHORT    sourcePort;        //源端口号
	USHORT    destinationPort;//目的端口号
	USHORT    len;            //封包长度
	USHORT    checksum;        //校验和
}UDPHeader,*PUDPHeader;

//ICMP头部
typedef struct _ICMPHeader
{
	UCHAR    icmp_type;        //消息类型
	UCHAR    icmp_code;        //代码
	USHORT    icmp_checksum;    //校验和
	//下面是回显头
	USHORT    icmp_id;        //用来惟一标识此请求的ID号，通常设置为进程ID
	USHORT    icmp_sequence;    //序列号
	ULONG    icmp_timestamp;    //时间戳
}ICMPHeader,*PICMPHeader;

//ARP数据帧
typedef struct tagARPFrame
{
	unsigned short     HW_Type;//硬件类型
	unsigned short     Prot_Type;//协议类型
	unsigned char      HW_Addr_Len;//硬件地址长度
	unsigned char      Prot_Addr_Len;//协议地址长度
	unsigned short     Opcode;//操作码
	unsigned char      Send_HW_Addr[6];//发送方硬件地址
	unsigned char      Send_Prot_Addr[4];//发送方协议地址
	unsigned char      Targ_HW_Addr[6];//目标硬件地址
	unsigned char      Targ_Prot_Addr[4];//目标协议地址
	unsigned char      padding[18];
} ARPFRAME, *PARPFRAME;

//ARP操作字段
typedef struct _opcode 
{
	u_short type;
	char description[50];
}OPCODE;

OPCODE opcode_table[5];

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

//初始化以太网类型表
void initial_eth_type_table()
{
	eth_match[0].type = IEEE;
	strcpy(eth_match[0].description, "IEEE 802.3");
	eth_match[1].type = EXP;
	strcpy(eth_match[1] .description, "EXPERIMENT");
	strcpy(eth_match[1] .description,"EXPERIMENT");
	eth_match[2].type = XEROX_NS_IDP;
	strcpy(eth_match[2] .description," XEROX NS IDP");
	eth_match[3].type = DLOG;
	strcpy(eth_match[3] .description, "DLOG");
	eth_match[4].type = IP;
	strcpy(eth_match[4] .description,"IP");
	eth_match[5].type = X_75Internet;
	strcpy(eth_match[5] .description," X.75 Internet");
	eth_match[6].type = NBS_Internet;
	strcpy(eth_match[6] .description,"NBS Internet");
	eth_match[7].type = ECMA;
	strcpy(eth_match[7] .description,"ECMA Internet");
	eth_match[8].type = Chaosnet;
	strcpy(eth_match[8] .description,"Chaosnet");
	eth_match[9].type = X25_Level3;
	strcpy(eth_match[9] .description,"X.25 Level 3");
	eth_match[10].type = ARP;
	strcpy(eth_match[10] .description,"ARP ： Address Resolution Protocol");
	eth_match[11].type = Frame_Relay_ARP;
	strcpy(eth_match[11] .description,"Frame Relay ARP [RFC1701]");
	eth_match[12].type = Raw_Frame_Relay;
	strcpy(eth_match[12] .description ,"Raw Frame Relay [RFC1701]");
	eth_match[13].type = DARP;
	strcpy(eth_match[13] .description,"Dynamic Reverse Address Resolution Protocol");
	eth_match[14].type = Novell_Netware_IPX;
	strcpy(eth_match[14] .description,"Novell Netware IPX");
	eth_match[15].type = EtherTalk;
	strcpy(eth_match[15] .description," EtherTalk");
	eth_match[16].type = IBM_SNA_Services;
	strcpy(eth_match[16] .description,"IBM SNA Services over Ethernet");
	eth_match[17].type = AARP;
	strcpy(eth_match[17] .description,"AARP：AppleTalk Address Resolution Protocol");
	eth_match[18].type = EAPS;
	strcpy(eth_match[18] .description,"EAPS：Ethernet Automatic Protection Switching");
	eth_match[19].type = IPX;
	strcpy(eth_match[19] .description,"IPX：Internet Packet Exchange");
	eth_match[20].type = SNMP;
	strcpy(eth_match[20] .description,"SNMP：Simple Network Management Protocol");
	eth_match[21].type = IPV6;
	strcpy(eth_match[21] .description,"IPv6，Internet Protocol version 6");
	eth_match[22].type = PPP;
	strcpy(eth_match[22] .description,"PPP：Point-to-Point Protocol");
	eth_match[23].type = GSMP;
	strcpy(eth_match[23] .description,"GSMP：General Switch Management Protocol");
	eth_match[24].type = MPLS_unicast;
	strcpy(eth_match[24] .description,"MPLS：Multi-Protocol Label Switching <unicast>"); 
	eth_match[25].type = MPLS_multicast;
	strcpy(eth_match[25] .description,"MPLS, Multi-Protocol Label Switching <multicast>");
	eth_match[26].type = PPPoE_DS;
	strcpy(eth_match[26] .description,"PPPoE：PPP Over Ethernet <Discovery Stage>");
	eth_match[27].type = PPPoE_SS;
	strcpy(eth_match[27] .description,"PPPoE，PPP Over Ethernet<PPP Session Stage>");
	eth_match[28].type = LWAPP;
	strcpy(eth_match[28] .description,"LWAPP：Light Weight Access Point Protocol");
	eth_match[29].type = LLDP;
	strcpy(eth_match[29] .description,"LLDP：Link Layer Discovery Protocol");
	eth_match[30].type = EAP;
	strcpy(eth_match[30] .description,"EAPOL：EAP over LAN");
	eth_match[31].type = Loopback;
	strcpy(eth_match[31] .description,"Loopback");
	eth_match[32].type = VLAN_Tag1;
	strcpy(eth_match[32] .description,"VLAN Tag Protocol Identifier");
	eth_match[33].type = VLAN_Tag2;
	strcpy(eth_match[33] .description,"VLAN Tag Protocol Identifier");
	eth_match[34].type = MAINSTAIN;
	strcpy(eth_match[34].description,"MAINSTAIN");
}

//获取以太网帧类型
ETHERNET_FRAME_TYPE get_eth_type(u_short type, ETHERNET_FRAME_TYPE eth_type_table[])
{
	for (int i = 0; i <= 34; i++)
	{
		if (type == eth_type_table[i].type)
		{
			return eth_type_table[i];
		}
	}
	return eth_type_table[4];
}

//包捕获回调函数
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm *ltime;
	char timestr[16];
	time_t local_tv_sec;
	ip_header *ih;
	UDPHeader *uh;
	ARPFRAME *ah;
	u_int ip_len;
	u_short sport,dport; 
	DLCHEADER *dlcheader;
	u_short ethernet_type;
	ETHERNET_FRAME_PROTOCOL eth_pro;
	ETHERNET_FRAME_TYPE eth_type;
	unsigned char *ch;
	//获取事件
	local_tv_sec = header->ts.tv_sec;
	ltime=localtime(&local_tv_sec);
	strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);

	printf("\n%s, %.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
	printf("以太网帧内容:\n");
	dlcheader = (DLCHEADER *)pkt_data; //获取以太网帧的首部

	ch = ( unsigned char *)pkt_data;
	for (int i = 0; i < header ->len; i++)
	{
		printf("%02x ", *(ch + i));
		if (i % 16 == 15)
		{
			printf("\n");
		}    
	}
	printf("\n");

	printf("以太网帧首部:\n");
	printf("目标MAC地址: " );
	for (int i = 0; i < 6; i++)
	{
		if (i != 5)
		{
			printf("%02x-", dlcheader->DesMAC[i]);
		}
		else
			printf("%02x\n", dlcheader->DesMAC[i]);
	}
	printf("源MAC地址: " );
	for (int i = 0; i < 6; i++)
	{
		if (i != 5)
		{
			printf("%02x-", dlcheader->SrcMAC[i]);
		}
		else
			printf("%02x\n", dlcheader->SrcMAC[i]);
	}
	ethernet_type = ntohs(dlcheader->Ethertype);
	eth_type = get_eth_type(ethernet_type, eth_match);
	printf("以太网帧格式：0x%04x (%s)\n", ethernet_type, eth_type.description);

	//获得IP数据包头部的位置
	if (eth_type.type == IP)
	{
		ih = (ip_header *) (pkt_data +
			14); //以太网头部长度
		printf("IP头部详细内容:\n");
		printf("版本号和头长度(各占4位): 0x%02x", ih ->ver_ihl);
		int lenth_byte = (ih ->ver_ihl) % 16;
		int ip_type = (ih ->ver_ihl)/16;
		if (ip_type == 4)
		{
			printf("(IPv4)\n");
		}
		else 
		{
			printf("\n");
		}
		printf("服务类型: 0x%02x\n", ih->tos);
		printf("封包总长度(即整个IP报的长度): %d\n", ntohs(ih->tlen));
		printf("封包标识(惟一标识发送的每一个数据报): 0x%04x\n", ntohs(ih->identification));
		printf("标志(3位)和片位移(13位)： 0x%04x\n", ntohs(ih->flags_fo) );
		printf("生存时间TTL: 0x%02x\n", ih ->ttl);
		printf("协议类型: 0x%02x", ih->proto);
		if (ih ->proto == 1)
		{
			printf("(ICMP)\n");
		}
		else if (ih ->proto == 6)
		{
			printf("(TCP)\n");
		}
		else if (ih ->proto == 17)
		{
			printf("(UDP)\n");
		}
		else if (ih ->proto == 2)
		{
			printf("(IGMP)\n");
		}
		else
			printf("\n");
		printf("16位首部校验和: 0x%04x\n", ntohs(ih ->crc));
		printf("32位源ip地址： %d. %d. %d. %d\n", (ih ->saddr).byte1, (ih ->saddr).byte2, (ih ->saddr).byte3, (ih ->saddr).byte4);
		printf("32位目的ip地址： %d. %d. %d. %d\n", (ih ->daddr).byte1, (ih ->daddr).byte2, (ih ->daddr).byte3, (ih ->daddr).byte4);
		if (lenth_byte == 5)
		{
			printf("可选项内容为: 无\n");
		}
		else
		{
			printf("可选项内容为: ");
			for (int i = 34; i < (lenth_byte - 5) * 4 + 34; i++ )
			{
				printf("%02x ", *(ch+ i));
			}
			printf("\n");
		}
		if (ih ->proto == 1)
		{
			printf("ICMP首部内容:\n");
			_ICMPHeader *icmph;
			icmph = (_ICMPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			printf("ICMP类型： 0x%02x\n", icmph->icmp_type);
			printf("ICMP代码：0x%02x\n", icmph ->icmp_code);
			printf("校验和： 0x%04x\n", ntohs(icmph->icmp_checksum));
			printf("标志符： 0x%04x\n", ntohs(icmph->icmp_id));
			printf("序号： 0x%04x\n", ntohs(icmph->icmp_sequence));
		}
		else if (ih ->proto == 6)
		{
			printf("TCP首部内容:\n");
			_TCPHeader *tcph;
			tcph = (_TCPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			printf("16位源端口：%d\n", ntohs(tcph ->sourcePort));
			printf("16位目的端口：%d\n", ntohs(tcph->destinationPort));
			printf("32位发送序号：%ld\n", ntohs(tcph->sequenceNumber));
			printf("32位接收序号：%ld\n", ntohs(tcph->acknowledgeNumber));
			printf("4位首部长度/6位保留字/6位标志位: 0x%04x\n", ntohs(tcph->dataoffset));
			printf("16位窗口大小: %d\n", ntohs(tcph->windows));
			printf("16位校验和: 0x%04x\n", ntohs(tcph->checksum));
			printf("16位紧急数据偏移量： 0x%04x\n", ntohs(tcph->urgentPointer));
		}
		else if (ih ->proto == 17)
		{
			printf("UDP首部内容:\n");
			UDPHeader *udph;
			udph = (UDPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			sport = ntohs( udph->sourcePort );
			dport = ntohs( udph->destinationPort );
			printf("16位UDP源端口号： %d\n", sport);
			printf("16位UDP目的端口号： %d\n", dport);
			printf("16位UDP长度： %d\n", ntohs(udph ->len));
			printf("16位UDP校验和： 0x%04x\n", ntohs(udph ->checksum));
		}
		else
		{
			printf("\n");
		}
	}
	else if (eth_type.type == ARP) //解析ARP数据包
	{
		ah = (ARPFRAME *) (pkt_data +
			14); //以太网头部长度
		printf("ARP数据帧详细内容:\n");
		printf("硬件类型: 0x%04x ", ntohs(ah->HW_Type));
		if (ntohs(ah->HW_Type) == 1)
		{
			printf("(Ethernet)\n");
		}
		else
		{
			printf("\n");
		}
		printf("协议类型: 0x%04x (%s)\n", ntohs(ah ->Prot_Type), get_eth_type( ntohs(ah ->Prot_Type), eth_match).description);
		printf("硬件地址长度: %d\n", ah ->HW_Addr_Len);
		printf("协议地址长度: %d\n", ah ->Prot_Addr_Len);
		printf("操作码: 0x%04x (%s)\n", ntohs(ah ->Opcode), opcode_table[ntohs(ah ->Opcode)].description);
		printf("发送方硬件地址: ");
		for (int i = 0; i < 6; i++)
		{
			if (i != 5)
			{
				printf("%x-", ah->Send_HW_Addr[i]);
			}
			else
				printf("%x\n", ah ->Send_HW_Addr[i]);
		}
		printf("发送方IP地址: ");
		for (int i = 0; i < 4; i++)
		{
			if (i != 3)
			{
				printf("%d. ", ah ->Send_Prot_Addr[i]);
			}
			else 
			{
				printf("%d\n", ah ->Send_Prot_Addr[i]);
			}
		}
		printf("目标硬件地址: ");
		for (int i = 0; i < 6; i++)
		{
			if (i != 5)
			{
				printf("%x-", ah->Targ_HW_Addr[i]);
			}
			else
			{
				printf("%x\n", ah ->Targ_HW_Addr[i]);
			}
		}
		printf("目标IP地址: ");
		for (int i = 0; i < 4; i++)
		{
			if (i != 3)
			{
				printf("%d. ", ah ->Targ_Prot_Addr[i]);
			}
			else 
			{
				printf("%d\n", ah ->Targ_Prot_Addr[i]);
			}
		}
	}
	else
	{
		return;
	}
}
int main()
{
	pcap_if_t *alldevs, *dev;
	int inum, i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter_all[] = "";
	char packet_filter_notarp[] = "not arp"; //
	char packet_filter_tcp[] = "ip and tcp";
	char packet_filter_udp[] = "ip and udp";
	char packet_filter_icmp[] = "ip and icmp";
	struct bpf_program fcode; 
	//初始化以太网帧协议表
	initial_eth_type_table(); 
	//初始化ARP操作字段表
	opcode_table[1].type = 1;
	strcpy(opcode_table[1].description, "ARP request");
	opcode_table[2].type = 2;
	strcpy(opcode_table[2].description, "ARP response");
	opcode_table[3].type = 1;
	strcpy(opcode_table[3].description , "RARP request");
	opcode_table[4].type = 2;
	strcpy(opcode_table[4].description , "RARP response");
	//获取本机网卡
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	//输入网卡类型
	printf("检测到本机有以下网卡设备：\n");
	for(dev=alldevs; dev; dev=dev->next)
	{
		printf("\n");
		printf("%d. %s", ++i, dev->name);
		if (dev->description)
			printf(" (%s)\n", dev->description);
		else
			printf(" (没有合适的描述符)\n");
	}
	if(i==0)
	{
		printf("没有找到接口，请确保你安装了WinPcap.\n");
		return -1;
	}
	printf("输入网卡号 (1-%d):",i);
	scanf("%d", &inum);
	if(inum < 1 || inum > i)
	{
		printf("输入错误.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	//转到选择的网卡
	for(dev=alldevs, i=0; i< inum-1 ;dev=dev->next, i++);
	//打开网卡
	if ( (adhandle= pcap_open(dev->name,         // 设备名
		65536,            // 65535保证能捕获到不同数据链路层上的每个数据包的全部内容
		PCAP_OPENFLAG_PROMISCUOUS,    // 混杂模式
		1000,             // 读取超时时间
		NULL,             // 远程机器验证
		errbuf            // 错误缓冲池
		) ) == NULL)
	{
		fprintf(stderr,"适配器打开失败. %s 不被支持\n", dev->name);
		pcap_freealldevs(alldevs);
		return -1;
	}
	if(dev->addresses != NULL)
		//获得网卡地址掩码
		netmask=((struct sockaddr_in *)(dev->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		//默认掩码255.255.255.0
		netmask=0xffffff;


	//编译过滤器
	printf("捕获选项：1.捕获全部 2.过滤ARP 3.捕获TCP 4.捕获UDP 5.捕获ICMP\n");
	char fileter;
	fileter = getchar();
	scanf( "%c", &fileter);
	switch (fileter)
	{
	case '1':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_all, 1, netmask) <0 )
			{
				fprintf(stderr,"设置失败！\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '2':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_notarp, 1, netmask) <0 )
			{
				fprintf(stderr,"设置失败！\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '3':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_tcp, 1, netmask) <0 )
			{
				fprintf(stderr,"设置失败！\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '4':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_udp, 1, netmask) <0 )
			{
				fprintf(stderr,"设置失败！\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '5':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_icmp, 1, netmask) <0 )
			{
				fprintf(stderr,"设置失败！\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	default:
		if (pcap_compile(adhandle, &fcode, packet_filter_all, 1, netmask) <0 )
		{
			fprintf(stderr,"设置失败！\n");
			pcap_freealldevs(alldevs);
			return -1;
		}    
	}
	//设置过滤器
	if (pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"设置失败！\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("监听 %s\n", dev->description);
	pcap_freealldevs(alldevs);

	//开始抓包
	pcap_loop(adhandle, 0, packet_handler, NULL);
	return 0; 
}
