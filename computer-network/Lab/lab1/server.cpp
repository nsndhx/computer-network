#include<iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <time.h>

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

	//服务端用于将把用于通信的地址和端口绑定到 socket 上
	int bind_redult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (bind_redult == SOCKET_ERROR) {
		cout << "绑定失败：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		return 1;
	}

	//开始监听
	int lis = listen(sockSrv, 5);
	if (lis == SOCKET_ERROR) {
		cout << "出现错误：" << WSAGetLastError() << endl;
		closesocket(sockSrv);
		WSACleanup();
		return 1;
	}

	//实现群聊
	//连接客户端（最高100个）
	int amount = 0;
	cin >> amount;
	SOCKADDR_IN addrClient[100];
	SOCKET sockConn[100];

	for (int i = 1; i <= amount; i++) {
		int len = sizeof(addrClient[i]);
		sockConn[i] = accept(sockSrv, (SOCKADDR*)&addrClient[i], &len);
		if (sockConn[i] == INVALID_SOCKET)
		{
			cout << "客户端" << i << "发出请求，服务器接收请求失败：" << WSAGetLastError() << endl;
			closesocket(sockConn[i]);
			WSACleanup();
			return 1;
		}
		else {
			cout << "服务器接收客户端" << i << "的请求" << endl;
		}
	}
	//向所有客户端传达有多少人参与群聊以及各自的序号
	for (int i = 1; i <= amount; i++) {
		char buf[2];
		buf[0] = (char)i;
		buf[1] = (char)amount;
		int send_r = send(sockConn[i], buf, 1024, 0);
		if (send_r == SOCKET_ERROR) {
			cout << "服务器向客户端" << i << "发送消息失败：" << WSAGetLastError() << endl;
			closesocket(sockConn[i]);
			WSACleanup();
			return 1;
		}
	}

	//处理消息
	char recvBuf[1024] = { 0 };
	char t[1024] = { 0 };
	int recv_result = 0;
	int send_result = 0;
	int i = 1;
	while (1) {
		memset(recvBuf, '\0', sizeof(recvBuf));
		memset(t, '\0', sizeof(t));
		recv_result = recv(sockConn[i], recvBuf, 1024, 0);
		if (recv_result == SOCKET_ERROR) {
			cout << "服务器没有收到客户端" << i << "的消息：" << WSAGetLastError() << endl;
			closesocket(sockConn[i]);
			WSACleanup();
			return 1;
		}
		else {
			cout << "服务器收到客户端" << i << "的消息：" << recvBuf << endl;
			string buf_dosoming(recvBuf);
			strcpy(t, (buf_dosoming.substr(buf_dosoming.length() - 4, buf_dosoming.length() - 1).c_str()));
		}
		for (int j = 1; j <= amount; j++) {
			if (j != i) {
				send_result = send(sockConn[j], recvBuf, 1024, 0);
				if (send_result == SOCKET_ERROR) {
					cout << "服务器向客户端" << j << "发送消息失败：" << WSAGetLastError() << endl;
					closesocket(sockConn[j]);
					WSACleanup();
					return 1;
				}
				else {
					cout << "服务器向客户端" << j << "发送消息：" << recvBuf << endl;
				}
			}
		}
		if (strcmp(t, "over") == 0) {
			if (i != amount) {
				i++;
			}
			else {
				i = 1;
			}
		}
		else if (strcmp(t, "exit") == 0) {
			for (int j = 1; j <= amount; j++) {
				closesocket(sockConn[j]);
			}
			WSACleanup();
			return 1;
		}
	}
	closesocket(sockSrv);
	WSACleanup();
	return 0;
}