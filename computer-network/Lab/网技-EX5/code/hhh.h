#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <conio.h>
#include<iomanip>
#include<WS2tcpip.h>
#include<cstdlib>

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
typedef struct FrameHeader_t {	//帧首部
	BYTE	DesMAC[6];	// 目的地址+
	BYTE 	SrcMAC[6];	// 源地址+
	WORD	FrameType;	// 帧类型
} FrameHeader_t;
typedef struct ArpPacket {	//包含帧首部和ARP首部的数据包
	FrameHeader_t	ed;
	WORD HardwareType; //硬件类型
	WORD ProtocolType; //协议类型
	BYTE HardwareAddLen; //硬件地址长度
	BYTE ProtocolAddLen; //协议地址长度
	WORD OperationField; //操作类型，ARP请求（1），ARP应答（2），RARP请求（3），RARP应答（4）。
	BYTE SourceMacAdd[6]; //源mac地址
	DWORD SourceIpAdd; //源ip地址
	BYTE DestMacAdd[6]; //目的mac地址
	DWORD DestIpAdd; //目的ip地址
} ArpPacket;
typedef struct IPHeader_t {		//IP首部
	BYTE Ver_HLen; //IP协议版本和IP首部长度。高4位为版本，低4位为首部的长度(单位为4bytes)
	BYTE TOS;//服务类型+
	WORD TotalLen;//总长度+
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
#pragma pack()

