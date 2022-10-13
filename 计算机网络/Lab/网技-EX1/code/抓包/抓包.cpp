#include<iostream>
#include "pcap.h"
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"packet.lib")
using namespace std;

#pragma pack(1)		//进入字节对齐方式
typedef struct FrameHeader_t {	//帧首部
	BYTE	DesMAC[6];	// 目的地址
	BYTE 	SrcMAC[6];	// 源地址
	WORD	FrameType;	// 帧类型
} FrameHeader_t;
typedef struct IPHeader_t {		//IP首部
	BYTE	Ver_HLen;
	BYTE	TOS;
	WORD	TotalLen;
	WORD	ID;
	WORD	Flag_Segment;
	BYTE	TTL;
	BYTE	Protocol;
	WORD	Checksum;
	ULONG	SrcIP;
	ULONG	DstIP;
} IPHeader_t;
typedef struct Data_t {	//包含帧首部和IP首部的数据包
	FrameHeader_t	FrameHeader;
	IPHeader_t		IPHeader;
} Data_t;
#pragma pack()	//恢复缺省对齐方式

typedef struct pcap_if pcap_if_t;
struct pcap_if {
	struct pcap_if* next;
	char* name;
	char* description;
	struct pcap_addr* addresses;
	u_int flags;
};
struct pcap_addr {
	struct pcap_addr* next;
	struct sockaddr* addr;
	struct sockaddr* netmask;
	struct sockaddr* broadaddr;
	struct sockaddr* dstaddr;
};
//获取设备列表
int pcap_findalldevs_ex(
		char *source,
		struct	pcap_rmtauth auth,
		pcap_if_t **alldevs,
		char *errbuf
);

int main() {
	//本机接口和IP地址的获取
	pcap_if_t* alldevs; 	               //指向设备链表首部的指针
	pcap_if_t* d;
	pcap_addr_t* a;
	char		errbuf[PCAP_ERRBUF_SIZE];	//错误信息缓冲区
	//获得本机的设备列表
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, 	//获取本机的接口设备
		NULL,			       //无需认证
		&alldevs, 		       //指向设备列表首部
		errbuf			      //出错信息保存缓存区
	) == -1)
	{
		//错误处理
		cout << "获取本机设备错误" << endl;
		exit(1);
	}
			
	for (d = alldevs; d != NULL; d = d->next)      //显示接口列表
	{
		//……	//利用d->name获取该网络接口设备的名字
			//……	//利用d->description获取该网络接口设备的描述信息
			//获取该网络接口设备的IP地址信息
			for (a = d->addresses; a != NULL; a = addr->next)
				if (a->addr->sa_family == AF_INET)  //判断该地址是否IP地址
				{
					//……	//利用a->addr获取IP地址
						//……	//利用a->netmask获取网络掩码
						//……	//利用a->broadaddr获取广播地址
						//……	//利用a->dstaddr)获取目的地址
				}
	}
	pcap_freealldevs(alldevs); //释放设备列表

	system("pause");
	return 0;
}