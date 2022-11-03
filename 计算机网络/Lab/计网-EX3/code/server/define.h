#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <stdlib.h> 
#include <time.h> 
#include <WinSock2.h> 
#include <fstream> 
#include <iostream>
#include <cstdint>
#include <vector>
#include<iostream>

#pragma comment(lib,"ws2_32.lib") 

#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory RtlZeroMemory

using namespace std;
using std::cout;
using std::endl;
using std::cin;
using std::vector;

struct packet {
	unsigned char tag;//连接建立、断开标识 
	unsigned int seq;//序列号 
	unsigned int ack;//确认号
	unsigned short len;//数据部分长度
	unsigned short checksum;//校验和
	unsigned short window;//窗口
	char data[1024];//数据长度

	void init_packet()
	{
		this->tag = -1;
		this->seq = -1;
		this->ack = -1;
		this->len = -1;
		this->checksum = -1;
		this->window = -1;
		memset(this->data, '\0', sizeof(*this->data));
	}
};

packet* connecthandler(int tag, int packetnum)
{
	packet* pkt = new packet;
	pkt->tag = tag;
	pkt->len = packetnum;
	return pkt;
}

#endif