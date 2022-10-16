#include<iostream>
#include"pcap.h"
#include<iomanip>
#include<WS2tcpip.h>
#include<windows.h>
#include<cstdlib>
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"packet.lib")
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"ws2_32.lib")
#define LINE_LEN 16
#define MAX_ADDR_LEN 16
using namespace std;

#pragma pack(1)		//进入字节对齐方式
typedef struct FrameHeader_t {	//帧首部
	BYTE	DesMAC[6];	// 目的地址
	BYTE 	SrcMAC[6];	// 源地址
	WORD	FrameType;	// 帧类型
} FrameHeader_t;
typedef struct IPHeader_t {		//IP首部
	BYTE Ver_HLen;//版本
	BYTE TOS;//服务类型
	WORD TotalLen;//总长度
	WORD ID;//标识
	WORD Flag_Segment;//标志 片偏移
	BYTE TTL;//生存周期
	BYTE Protocol;//协议
	WORD Checksum;//头部校验和
	u_int SrcIP;//源IP
	u_int DstIP;//目的IP
} IPHeader_t;
typedef struct Data_t {	//包含帧首部和IP首部的数据包
	FrameHeader_t	FrameHeader;
	IPHeader_t		IPHeader;
} Data_t;
#pragma pack()	//恢复缺省对齐方式

void ip_protocol_packet_handle(const struct pcap_pkthdr* pkt_header, const u_char* pkt_data)
{
	IPHeader_t* IPHeader;
	IPHeader = (IPHeader_t*)(pkt_data + 14);
	sockaddr_in source, dest;
	char sourceIP[MAX_ADDR_LEN], destIP[MAX_ADDR_LEN];
	char str[16];
	source.sin_addr.s_addr = IPHeader->SrcIP;
	dest.sin_addr.s_addr = IPHeader->DstIP;
	strncpy_s(sourceIP, inet_ntop(AF_INET,&source.sin_addr,str,16), MAX_ADDR_LEN);
	strncpy_s(destIP, inet_ntop(AF_INET,&dest.sin_addr,str,16), MAX_ADDR_LEN);

	//开始输出
	cout << dec << "Version：" << (int)(IPHeader->Ver_HLen >> 4) << endl;
	cout << "Header Length：";
	cout << (int)((IPHeader->Ver_HLen & 0x0f) * 4) << " Bytes" << endl;
	cout << "Tos：" << (int)IPHeader->TOS << endl;
	cout << "Total Length：" << (int)ntohs(IPHeader->TotalLen) << endl;
	cout << "Identification：0x" << hex << setw(4) << setfill('0') << ntohs(IPHeader->ID) << endl;
	cout << "Flags：" << dec << (int)(ntohs(IPHeader->Flag_Segment)) << endl;
	cout << "Time to live：" << (int)IPHeader->TTL << endl;
	cout << "Protocol Type： ";
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
	cout << "Header checkSum：0x" << hex << setw(4) << setfill('0') << ntohs(IPHeader->Checksum) << endl;
	cout << "Source：" << sourceIP << endl;
	cout << "Destination：" << destIP << endl;

}
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

	cout<<"Ethernet type： ";
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
	//pcap_addr_t* a;
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
			
	for (d = alldevs; d != NULL; d = d->next) //显示接口列表
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
	
	cout << "监听：" << d->description << endl;
	pcap_freealldevs(alldevs);
	int cnt = -1;
	cout << "将要捕获数据包的个数：";
	cin >> cnt;
	pcap_loop(adhandle, cnt, ethernet_protocol_packet_handle, NULL);
	pcap_close(adhandle);

	system("pause");
	return 0;
}