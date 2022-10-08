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
	SOCKET sockClient2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockClient2 == INVALID_SOCKET)
	{
		cout << "套接字创建失败" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	//else {
	//	cout << "套接字创建成功" << endl;
	//}

	//addrStv
	SOCKADDR_IN addrSrv;
	//设置地址族为IPv4
	addrSrv.sin_family = AF_INET;
	//设置地址的端口号信息
	addrSrv.sin_port = htons(8888);
	//127.0.0.1一个特殊的IP地址，表示是本机的IP地址                                               
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//向一个服务端的socket发出建连请求
	int conn = connect(sockClient2, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (conn == SOCKET_ERROR) {
		closesocket(sockClient2);
		cout << "连接服务端错误：" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	//else {
	//	cout << "连接服务端成功" << endl;
	//}
	cout << "连接成功" << endl;
	//确定和谁建立对话
	char mes[1024] = { 0 };
	char recvbuf[1024] = { 0 };
	int send_result = 0;
	int recv_result = 0;
	/*当客户端发送over时，结束发送消息；发送exit时，结束对话*/
	while (1) {
		while (1) {//客户端2接收1的消息
			memset(recvbuf, '\0', sizeof(recvbuf)); 
			recv_result = recv(sockClient2, recvbuf, 1024, 0);
			if (recv_result > 0){}
				//cout << "收到信息：" << recvbuf << endl;
			else if (recv_result == 0)
				cout << "连接关闭" << endl;
			else {
				cout << "收到失败：" << WSAGetLastError() << endl;
				closesocket(sockClient2);
				WSACleanup();
				return 1;
			}
			if (strcmp(recvbuf, "exit") == 0) {
				cout << "对方结束对话" << endl;
				closesocket(sockClient2);
				WSACleanup();
				return 1;
			}
			else if (strcmp(recvbuf, "over") == 0) {
				break;
			}
			cout << "client1：" << recvbuf << endl;
		}
		while (1) {//客户端2向1发消息
			cout << "client2：";
			memset(mes, '\0', sizeof(mes));
			cin.getline(mes, sizeof(mes));
			send_result = send(sockClient2, mes, (int)strlen(mes), 0);
			if (send_result == SOCKET_ERROR) {
				cout << "发送消息失败：" << WSAGetLastError() << endl;
				closesocket(sockClient2);
				WSACleanup();
				return 1;
			}
			//cout << "发送消息：" << mes << endl;
			if (strcmp(mes, "exit") == 0)
			{
				cout << "结束对话" << endl;
				closesocket(sockClient2);
				WSACleanup();
				return 1;
			}
			else if (strcmp(mes, "over") == 0) {
				break;
			}
		}
	}

	closesocket(sockClient2);
	WSACleanup();
	return 0;
}