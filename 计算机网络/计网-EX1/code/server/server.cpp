#include<iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 6031)
#pragma warning(disable : 4996)
using namespace std;

int main() {

	//初始化Socket DLL，使用2.2版本的socket
	WORD wVersionRequested;
	WSADATA wsaData;
	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET)
	{
		cout << "套接字创建失败" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	//addrStv
	SOCKADDR_IN addrSrv;
	//设置地址族为IPv4
	addrSrv.sin_family = AF_INET;
	//设置地址的端口号信息
	addrSrv.sin_port = htons(8888);                                             
	addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;

	int bind_redult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (bind_redult == SOCKET_ERROR) {
		cout << "绑定失败：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		return 1;
	}

	//开始监听
	int lis = listen(sockSrv, 100);
	if (lis == SOCKET_ERROR) {
		cout << "出现错误：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		WSACleanup();
		return 1;
	}

	//等待连接
	SOCKADDR_IN addrClient;
	int len = sizeof(addrClient);
	SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
	if (sockConn == INVALID_SOCKET)
	{
		cout << "客户端发出请求，服务器接收请求失败：" << WSAGetLastError() << endl;
		closesocket(sockConn);
		WSACleanup();
		return 1;
	}
	else {
		cout << "服务器接收请求" << endl;
	}

	char recvBuf[512] = { 0 };
	int relen = recv(sockConn, recvBuf, 512, 0);
	if (relen == SOCKET_ERROR) {
		cout << "服务器没有收到消息：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		WSACleanup();
		return 1;
	}
	else {
		cout << "服务器收到消息：" << recvBuf << endl;
	}
	int slen = send(sockConn, recvBuf, relen, 0);
	if (slen == SOCKET_ERROR) {
		cout << "服务器发送消息失败：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		WSACleanup();
		return 1;
	}
	else {
		cout << "服务器发送消息：" << recvBuf << endl;
	}
	/*
	while (1)
	{
		while (1) {

		}
		send(sockConn, sendBuf, strlen, 0);
		int recv_result;
		do {
			recv_result = recv(sockSrv, recvbuf, 512, 0);
			if (recv_result > 0)
				cout << "收到信息：" << recv_result;
			else if (recv_result == 0)
				cout << "连接关闭";
			else
				cout << "收到失败：" << WSAGetLastError();
		} while (recv_result > 0);
		closesocket(sockConn);
	}*/
	
	closesocket(sockSrv);
	WSACleanup();

}