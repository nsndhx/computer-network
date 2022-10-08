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

	//创建客户端套接字，使用TCP协议
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockClient == INVALID_SOCKET)
	{
		cout << "套接字创建失败" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "套接字创建成功" << endl;
	}

	//addrStv
	SOCKADDR_IN addrSrv;
	//设置地址族为IPv4
	addrSrv.sin_family = AF_INET;
	//设置地址的端口号信息
	addrSrv.sin_port = htons(8888);
	//127.0.0.1一个特殊的IP地址，表示是本机的IP地址                                               
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//向一个服务端的socket发出建连请求
	int conn = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (conn == SOCKET_ERROR) {
		closesocket(sockClient);
		cout << "连接错误：" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "连接成功" << endl;
	}

	char mes[1024] = { "Hello" };
	int send_result = send(sockClient, mes, (int)strlen(mes), 0);
	if (send_result == SOCKET_ERROR) {
		cout << "发送失败：" << WSAGetLastError() << endl;
		closesocket(sockClient);
		WSACleanup();
		return 1;
	}
	cout << "发送消息：" << mes << endl;

	//
	char recvbuf[512] = { 0 };
	int recv_result;
	do {
		recv_result = recv(sockClient, recvbuf, 512, 0);
		if (recv_result > 0)
			cout << "收到信息：" << recvbuf << endl;
		else if (recv_result == 0)
			cout << "连接关闭" << endl;
		else
			cout << "收到失败：" << WSAGetLastError() << endl;
	} while (recv_result > 0);

	
	closesocket(sockClient);
	WSACleanup();
	return 0;
}