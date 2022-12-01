#include "hhh.h"
#include <IPHlpApi.h>
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"IPHlpApi.lib")    // 发送ARP报文要用的静态库,取MAC用
#define ARP_REQUEST  0x0001        // ARP请求
#define ARP_REPLY    0x0002           // ARP应答
#define IPTOSBUFFERS 12
#define HOSTNUM      255      // 主机数量

#pragma warning(disable:4996)
//#pragma warning(disable:4700)

char* myBroad;
unsigned char* m_MAC = new unsigned char[6];
char* m_IP = (char*)"10.130.86.122";
char* m_mask;
char d_IP[20];
//char* d_IP = (char*)"192.168.137.1";
unsigned char* d_MAC = new unsigned char[6];
bool flag;

char* iptos(u_long in) {
	static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
	static short which;
	u_char* p;

	p = (u_char*)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}

// 打印所有可用信息
void ifprint(pcap_if_t* d) {
	pcap_addr_t* a;

	/* IP addresses */
	for (a = d->addresses; a; a = a->next)
	{
		printf("\tAddress Family: #%d\n", a->addr->sa_family);
		switch (a->addr->sa_family)
		{
		case AF_INET:
			printf("\tAddress Family Name: AF_INET\n");
			if (a->addr)
			{
				m_IP = iptos(((struct sockaddr_in*)a->addr)->sin_addr.s_addr);
				printf("\tIP Address: %s\n", m_IP);
			}
			if (a->netmask)
			{
				m_mask = iptos(((struct sockaddr_in*)a->netmask)->sin_addr.s_addr);
				printf("\tNetmask: %s\n", m_mask);
			}

			if (a->broadaddr)
			{
				myBroad = iptos(((struct sockaddr_in*)a->broadaddr)->sin_addr.s_addr);
				printf("\tBroadcast Address: %s\n", myBroad);
			}
			if (a->dstaddr)
				printf("\tDestination Address: %s\n", iptos(((struct sockaddr_in*)a->dstaddr)->sin_addr.s_addr));
			break;
		default:
			//printf("\tAddress Family Name: Unknown\n");
			break;
		}
	}
	printf("\n");
}

char* GetSelfMac() {
	ULONG MacAddr[2] = { 0 };    // Mac地址长度6字节
	ULONG uMacSize = 6;
	DWORD dwRet = SendARP(inet_addr(m_IP), 0, &MacAddr, &uMacSize);
	if (dwRet == NO_ERROR)
	{
		BYTE* bPhyAddr = (BYTE*)MacAddr;

		if (uMacSize)
		{
			char* sMac = (char*)malloc(sizeof(char) * 18);
			int n = 0;

			memset(sMac, 0, 18);
			sprintf_s(sMac, (size_t)18, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", (int)bPhyAddr[0], (int)bPhyAddr[1], (int)bPhyAddr[2], (int)bPhyAddr[3], (int)bPhyAddr[4], (int)bPhyAddr[5]);
			return sMac;
		}
		else
		{
			printf("Mac地址获取失败！\n");
		}
	}
	else
	{
		printf("ARP报文发送失败:%d\n", dwRet);
	}
	return NULL;
}

void SendArpPacket(pcap_t* adhandle) {
	char* ip = m_IP;
	unsigned char* mac = m_MAC;
	char* netmask = m_mask;
	printf("ip_mac:%02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);
	printf("自身的IP地址为:%s\n", ip);
	printf("地址掩码NETMASK为:%s\n", netmask);
	printf("\n");
	unsigned char* sendbuf; //arp包结构大小
	ArpPacket arp;
	
	//赋值MAC地址
	memset(arp.ed.DesMAC, 0xff, 6);       //目的地址为全为广播地址
	printf("Des_MAC:%02x-%02x-%02x-%02x-%02x-%02x\n", arp.ed.DesMAC[0], arp.ed.DesMAC[1], arp.ed.DesMAC[2],
		arp.ed.DesMAC[3], arp.ed.DesMAC[4], arp.ed.DesMAC[5]);
	memcpy(arp.ed.SrcMAC, mac, 6);
	printf("Src_MAC:%02x-%02x-%02x-%02x-%02x-%02x\n", arp.ed.SrcMAC[0], arp.ed.SrcMAC[1], arp.ed.SrcMAC[2],
		arp.ed.SrcMAC[3], arp.ed.SrcMAC[4], arp.ed.SrcMAC[5]);
	memcpy(arp.SourceMacAdd, mac, 6);
	printf("SourceMacAdd:%02x-%02x-%02x-%02x-%02x-%02x\n", arp.SourceMacAdd[0], arp.SourceMacAdd[1], arp.SourceMacAdd[2],
		arp.SourceMacAdd[3], arp.SourceMacAdd[4], arp.SourceMacAdd[5]);
	memset(arp.DestMacAdd, 0x00, 6);
	printf("DestMacAdd:%02x-%02x-%02x-%02x-%02x-%02x\n", arp.DestMacAdd[0], arp.DestMacAdd[1], arp.DestMacAdd[2],
		arp.DestMacAdd[3], arp.DestMacAdd[4], arp.DestMacAdd[5]);
	arp.ed.FrameType = htons(0x0806);//帧类型为ARP3
	cout << "EthTyte:" << arp.ed.FrameType << endl;
	arp.HardwareType = htons(0x0001);
	cout << "HardwareType:" << arp.HardwareType << endl;
	arp.ProtocolType = htons(0x0800);
	cout << "ProtocolType:" << arp.ProtocolType << endl;
	arp.HardwareAddLen = 6;
	arp.ProtocolAddLen = 4;
	arp.SourceIpAdd = inet_addr(ip); //请求方的IP地址为自身的IP地址
	cout << "SourceIpAss:" << arp.SourceIpAdd << endl;
	arp.OperationField = htons(0x0001);
	cout << "OperationField:" << arp.OperationField << endl;
	arp.DestIpAdd = inet_addr(d_IP);
	cout << "DestIpAdd:" << arp.DestIpAdd << endl;
	u_char* a = (u_char*)&arp;
	//如果发送成功
	cout << "a:";
	for (int i = 0; i < 48; i++) {
		printf("% 02x ", a[i]);
	}
	cout << endl;
	if (pcap_sendpacket(adhandle, (u_char*)&arp, sizeof(ArpPacket)) == 0) {
		printf("\nPacketSend succeed\n");
	}
	else {
		printf("PacketSendPacket in getmine Error: %d\n", GetLastError());
	}
	flag = TRUE;
	return;
}

//(pcap_t *adhandle)
int GetLivePC(pcap_t* adhandle) {
	//gparam *gpara = (gparam *)lpParameter;
	//pcap_t *adhandle = gpara->adhandle;
	int res;
	unsigned char Mac[6];
	struct pcap_pkthdr* pkt_header;
	const u_char* pkt_data;
	while ((res = pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0) {
		//cout << "ETH_ARP:" << *(unsigned short*)(pkt_data + 12) << "        " << htons(0x0806) << endl;
		if (*(unsigned short*)(pkt_data + 12) == htons(0x0806)) {
			ArpPacket* recv = (ArpPacket*)pkt_data;
			//cout << "ARP_REPLY:" << *(unsigned short*)(pkt_data + 20) << "        " << htons(ARP_REPLY) << endl;
			if (*(unsigned short*)(pkt_data + 20) == htons(ARP_REPLY)) {
				printf("sourceIP地址:%d.%d.%d.%d   MAC地址:",
					*(unsigned char*)(pkt_data + 28) & 255,
					*(unsigned char*)(pkt_data + 28 + 1) & 255,
					*(unsigned char*)(pkt_data + 28 + 2) & 255,
					*(unsigned char*)(pkt_data + 28 + 3) & 255);
				for (int i = 0; i < 6; i++) {
					Mac[i] = *(unsigned char*)(pkt_data + 22 + i);
					printf("%02x ", Mac[i]);
				}
				printf("\n");
				printf("destinationIP地址:%d.%d.%d.%d   MAC地址:",
					*(unsigned char*)(pkt_data + 38) & 255,
					*(unsigned char*)(pkt_data + 38 + 1) & 255,
					*(unsigned char*)(pkt_data + 38 + 2) & 255,
					*(unsigned char*)(pkt_data + 38 + 3) & 255);
				for (int i = 0; i < 6; i++) {
					Mac[i] = *(unsigned char*)(pkt_data + 32 + i);
					printf("%02x ", Mac[i]);
				}
				printf("\n");
				return 0;
			}
		}
		Sleep(10);
	}
	return 0;
}

//对IP数据包作分析，IP包的内容在原有物理帧后14字节开始。并提取出相应的字段。
void ip_protocol_packet_handle(const struct pcap_pkthdr* pkt_header, const u_char* pkt_data)
{
	IPHeader_t* IPHeader;
	IPHeader = (IPHeader_t*)(pkt_data + 14);
	sockaddr_in source, dest;
	char sourceIP[MAX_ADDR_LEN], destIP[MAX_ADDR_LEN];
	char str[16];
	source.sin_addr.s_addr = IPHeader->SrcIP;
	dest.sin_addr.s_addr = IPHeader->DstIP;
	strncpy_s(sourceIP, inet_ntop(AF_INET, &source.sin_addr, str, 16), MAX_ADDR_LEN);
	strncpy_s(destIP, inet_ntop(AF_INET, &dest.sin_addr, str, 16), MAX_ADDR_LEN);

	//开始输出
	cout << dec << "Version：" << (int)(IPHeader->Ver_HLen >> 4) << endl;
	cout << "Header Length：";
	cout << (int)((IPHeader->Ver_HLen & 0x0f) * 4) << " Bytes" << endl;
	cout << "Tos：" << (int)IPHeader->TOS << endl;//服务类型
	cout << "Total Length：" << (int)ntohs(IPHeader->TotalLen) << endl;//总长度
	cout << "Identification：0x" << hex << setw(4) << setfill('0') << ntohs(IPHeader->ID) << endl;//标识//ntohs()将一个16位数由网络字节顺序转换为主机字节顺序
	cout << "Flags：" << dec << (int)(ntohs(IPHeader->Flag_Segment)) << endl;//标志 片偏移
	cout << "Time to live：" << (int)IPHeader->TTL << endl;//生存周期
	cout << "Protocol Type： ";//协议
	switch (IPHeader->Protocol)
	{
	case 1:
		cout << "ICMP";
		break;
	case 6:
		cout << "TCP";
		break;
	case 17:
		cout << "UDP";
		break;
	default:
		break;
	}
	cout << "(" << (int)IPHeader->Protocol << ")" << endl;
	cout << "Header checkSum：0x" << hex << setw(4) << setfill('0') << ntohs(IPHeader->Checksum) << endl;//头部校验和
	cout << "Source：" << sourceIP << endl;
	cout << "Destination：" << destIP << endl;

}

//首先对物理帧帧作分析，看其中包含的是IP包还是ARP数据包。如果是IP包则转入到IP包处理函数。
void ethernet_protocol_packet_handle(u_char* param, const struct pcap_pkthdr* pkt_header, const u_char* pkt_data)
{
	FrameHeader_t* ethernet_protocol;//以太网协议
	u_short ethernet_type;			//以太网类型
	u_char* mac_string;				//以太网地址

	//获取以太网数据内容
	ethernet_protocol = (FrameHeader_t*)pkt_data;
	ethernet_type = ntohs(ethernet_protocol->FrameType);

	cout << "==============Ethernet Protocol=================" << endl;

	//以太网目标地址
	mac_string = ethernet_protocol->DesMAC;

	cout << "Destination Mac Address： ";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[0] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[1] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[2] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[3] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[4] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[5] << endl;

	//以太网源地址
	mac_string = ethernet_protocol->SrcMAC;

	cout << "Source Mac Address： ";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[0] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[1] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[2] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[3] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[4] << ":";
	cout << hex << setw(2) << setfill('0') << (u_int)mac_string[5] << endl;

	cout << "Ethernet type： ";
	switch (ethernet_type)
	{
	case 0x0800:
		cout << "IP";
		break;
	case 0x0806:
		cout << "ARP";
		break;
	case 0x0835:
		cout << "RARP";
		break;
	default:
		cout << "Unknown Protocol";
		break;
	}
	cout << " 0x" << setw(4) << setfill('0') << ethernet_type << endl;

	//进入IPHeader处理函数
	if (ethernet_type == 0x0800)
	{
		ip_protocol_packet_handle(pkt_header, pkt_data);
	}
}
int main() {
	//本机接口和IP地址的获取
	pcap_if_t* alldevs; 	               //指向设备链表首部的指针
	pcap_if_t* d;
	pcap_addr_t* a;
	int i = 0;
	int inum = 0;
	pcap_t* adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];	//错误信息缓冲区
	//获得本机的设备列表
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, 	//获取本机的接口设备
		NULL,			       //无需认证
		&alldevs, 		       //指向设备列表首部
		errbuf			      //出错信息保存缓存区
	) == -1)
	{
		//错误处理
		cout << "获取本机设备错误:" << errbuf << endl;
		pcap_freealldevs(alldevs);
		return 0;
	}
	//显示接口列表
	for (d = alldevs; d != NULL; d = d->next)
	{
		cout << dec << ++i << ": " << d->name; //利用d->name获取该网络接口设备的名字
		if (d->description) { //利用d->description获取该网络接口设备的描述信息
			cout << d->description << endl;
		}
		else {
			cout << "无相关描述信息" << endl;
			return -1;
		}
	}
	if (i == 0)
	{
		cout << "wrong!" << endl;
		return -1;
	}

	cout << "请输入要打开的网口号（1-" << i << "）：";
	cin >> inum;

	//检查用户是否指定了有效的设备
	if (inum < 1 || inum > i)
	{
		cout << "适配器数量超出范围" << endl;

		pcap_freealldevs(alldevs);
		return -1;
	}

	//跳转到选定的设备
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);
	ifprint(d);

	//打开网卡
	if ((adhandle = pcap_open(d->name,          // 设备名
		65536,            // 要捕捉的数据包的部分
						  // 65535保证能捕获到不同数据链路层上的每个数据包的全部内容
		PCAP_OPENFLAG_PROMISCUOUS,    // 混杂模式
		1000,             // 读取超时时间
		NULL,             // 远程机器验证
		errbuf            // 错误缓冲池
	)) == NULL)
	{
		cout << "无法打开设备：检查是否是支持的NPcap" << endl;
		pcap_freealldevs(alldevs);
		return -1;
	}
	/* 释放设备列表 */
	//pcap_freealldevs(alldevs);

	printf("\nlistening on %s...\n", d->description);
	m_MAC = (unsigned char*)GetSelfMac();
	printf("输入目标IP:");
	scanf("%s", &d_IP);
	//HANDLE sendthread;      //发送ARP包线程
	//HANDLE recvthread;       //接受ARP包线程

	//sendthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendArpPacket, adhandle, 0, NULL);
	//recvthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetLivePC, adhandle, 0, NULL);

	SendArpPacket(adhandle);
	GetLivePC(adhandle);

	pcap_freealldevs(alldevs);
	//CloseHandle(sendthread);
	//CloseHandle(recvthread);
	return 0;
}