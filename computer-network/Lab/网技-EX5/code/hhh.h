#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <conio.h>
#include <iomanip>
#include <WS2tcpip.h>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream> 

#define HAVE_REMOTE
#include <pcap.h>

#include <WS2tcpip.h>
//#include <remote-ext.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "packet.lib")
#pragma comment(lib, "wpcap.lib")
#define _AFXDLL
#define LINE_LEN 16
#define MAX_ADDR_LEN 16

using namespace std;

#pragma pack(1)
#pragma pack(1)
typedef struct FrameHeader_t {//帧首部
	BYTE DesMAC[6];//目的地址
	BYTE SrcMAC[6];//源地址
	WORD FrameType;//帧类型
}FrameHeader_t;

typedef struct ARPFrame_t {
	FrameHeader_t FrameHeader;//帧首部
	WORD HardwareType;//硬件类型
	WORD ProtocolType;//协议类型
	BYTE HLen;//硬件地址长度
	BYTE PLen;//协议地址
	WORD Operation;//操作
	BYTE SendHa[6];//发送方MAC
	DWORD SendIP;//发送方IP
	BYTE RecvHa[6];//接收方MAC
	DWORD RecvIP;//接收方IP
}ARPFrame_t;

typedef struct IPHeader_t {//IP首部
	BYTE Ver_HLen;
	BYTE TOS;
	WORD TotalLen;
	WORD ID;
	WORD Flag_Segment;
	BYTE TTL;//生命周期
	BYTE Protocol;
	WORD Checksum;//校验和
	ULONG SrcIP;//源IP
	ULONG DstIP;//目的IP
}IPHeader_t;

typedef struct Data_t {//包含帧首部和IP首部的数据包
	FrameHeader_t FrameHeader;//帧首部
	IPHeader_t IPHeader;//IP首部
}Data_t;

typedef struct ICMP_t {//包含帧首部和IP首部的数据包
	FrameHeader_t FrameHeader;
	IPHeader_t IPHeader;
	char buf[0x80];
}ICMP_t;
#pragma pack()
