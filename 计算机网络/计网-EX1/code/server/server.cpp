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

	//连接两个客户端
	SOCKADDR_IN addrClient1;
	SOCKADDR_IN addrClient2;
	int len1 = sizeof(addrClient1);
	int len2 = sizeof(addrClient2);
	SOCKET sockConn1 = accept(sockSrv, (SOCKADDR*)&addrClient1, &len1);
	if (sockConn1 == INVALID_SOCKET)
	{
		cout << "客户端1发出请求，服务器接收请求失败：" << WSAGetLastError() << endl;
		closesocket(sockConn1);
		WSACleanup();
		return 1;
	}
	else {
		cout << "服务器接收客户端1的请求" << endl;
	}
	SOCKET sockConn2 = accept(sockSrv, (SOCKADDR*)&addrClient2, &len2);
	if (sockConn2 == INVALID_SOCKET)
	{
		cout << "客户端2发出请求，服务器接收请求失败：" << WSAGetLastError() << endl;
		closesocket(sockConn2);
		WSACleanup();
		return 1;
	}
	else {
		cout << "服务器接收客户端2的请求" << endl;
	}

	
	char recvBuf[1024] = { 0 };
	int recv1_result = 0;
	int send1_result = 0;
	int recv2_result = 0;
	int send2_result = 0;
	
	while (1) {
		while (1) {//从客户端1接收消息，发送给客户端2
			memset(recvBuf, '\0', sizeof(recvBuf));
			recv1_result = recv(sockConn1, recvBuf, 1024, 0);
			if (recv1_result == SOCKET_ERROR) {
				cout << "服务器没有收到客户端1的消息：" << WSAGetLastError() << endl;
				closesocket(sockConn1);
				WSACleanup();
				return 1;
			}
			else {
				cout << "服务器收到客户端1的消息：" << recvBuf << endl;
			}
			send2_result = send(sockConn2, recvBuf, 1024, 0);
			if (send2_result == SOCKET_ERROR) {
				cout << "服务器向客户端2发送消息失败：" << WSAGetLastError() << endl;
				closesocket(sockConn2);
				WSACleanup();
				return 1;
			}
			else {
				cout << "服务器向客户端2发送消息：" << recvBuf << endl;
			}
			if (strcmp(recvBuf, "exit") == 0) {
				closesocket(sockConn1);
				closesocket(sockConn2);
				WSACleanup();
				return 1;
			}
			if (strcmp(recvBuf, "over") == 0) {
				break;
			}
		}
		while (1) {//从客户端2接收消息，发送给客户端1
			memset(recvBuf, '\0', sizeof(recvBuf));
			recv2_result = recv(sockConn2, recvBuf, 1024, 0);
			if (recv2_result == SOCKET_ERROR) {
				cout << "服务器没有收到客户端2的消息：" << WSAGetLastError() << endl;
				closesocket(sockConn2);
				WSACleanup();
				return 1;
			}
			else {
				cout << "服务器收到客户端2的消息：" << recvBuf << endl;
			}
			send1_result = send(sockConn1, recvBuf, 1024, 0);
			if (send1_result == SOCKET_ERROR) {
				cout << "服务器向客户端1发送消息失败：" << WSAGetLastError() << endl;
				closesocket(sockConn1);
				WSACleanup();
				return 1;
			}
			else {
				cout << "服务器向客户端1发送消息：" << recvBuf << endl;
			}
			if (strcmp(recvBuf, "exit") == 0) {
				closesocket(sockConn1);
				closesocket(sockConn2);
				WSACleanup();
				return 1;
			}
			if (strcmp(recvBuf, "over") == 0) {
				break;
			}
		}
	}
	
	closesocket(sockSrv);
	WSACleanup();
	return 0;
}