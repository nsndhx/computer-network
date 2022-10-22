#include<iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include<time.h> 
#include <sstream>

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
	int conn = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (conn == SOCKET_ERROR) {
		closesocket(sockClient);
		cout << "连接服务端错误：" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	//else {
	//	cout << "连接服务端成功" << endl;
	//}
	cout << "连接成功" << endl;

	char mes[1024] = { 0 };
	char recvbuf[1024] = { 0 };
	char buf[1024] = { 0 };
	int send_result = 0;
	int recv_result = 0;
	recv_result = recv_result = recv(sockClient, buf, 1024, 0);
	int id = (int)buf[0];
	int amount = (int)buf[1];
	cout << id << endl;
	cout << amount << endl;
	/*当客户端发送over时，结束发送消息；发送exit时，结束对话*/
	int i = 1;//i来标记目前是哪个客户端发消息
	while (1) {
		if (1 == id) {//最先发送消息
			while (1) {//客户端发消息
				cout << "client1：";
				memset(mes, '\0', sizeof(mes));
				time_t  t;
				char  buf[128];
				memset(buf, 0, sizeof(buf));
				struct tm* tmp;
				t = time(NULL);
				tmp = localtime(&t);
				strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmp);
				cin.getline(mes, sizeof(mes));
				strcat(buf, " ");
				strcat(buf, mes);
				send_result = send(sockClient, buf, (int)strlen(buf), 0);
				if (send_result == SOCKET_ERROR) {
					cout << "发送消息失败：" << WSAGetLastError() << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
				//cout << "发送消息：" << mes << endl;
				if (strcmp(mes, "exit") == 0)
				{
					cout << "结束群聊" << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
				else if (strcmp(mes, "over") == 0) {
					i++;
					break;
				}
			}
			while (1) {//客户端接收消息
				memset(recvbuf, '\0', sizeof(recvbuf));
				recv_result = recv(sockClient, recvbuf, 1024, 0);
				if (recv_result > 0) {
					if (strcmp(recvbuf, "exit") == 0) {
						cout << "客户端" << i << "结束群聊" << endl;
						closesocket(sockClient);
						WSACleanup();
						return 1;
					}
					else if (strcmp(recvbuf, "over") == 0) {
						if (i != amount) {
							i++;
						}
						else {
							i = 1;
							break;
						}
					}
					cout << "client" << i << "：" << recvbuf << endl;
				}
				//cout << "收到信息：" << recvbuf << endl;
				else if (recv_result == 0)
					cout << "连接关闭" << endl;
				else {
					cout << "收到失败：" << WSAGetLastError() << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
			}
		}
		else {//常规情况
			while (1) {//客户端先接收消息
				memset(recvbuf, '\0', sizeof(recvbuf));
				recv_result = recv(sockClient, recvbuf, 1024, 0);
				if (recv_result > 0) {
					if (strcmp(recvbuf, "exit") == 0) {
						cout << "客户端" << i << "结束群聊" << endl;
						closesocket(sockClient);
						WSACleanup();
						return 1;
					}
					else if (strcmp(recvbuf, "over") == 0) {
						if (i == id - 1) {
							i = id;
							break;
						}
						else {
							if (i == amount) {
								i = 1;
							}
							else {
								i++;
							}
						}
					}
					else {
						cout << "client" << i << "：" << recvbuf << endl;
					}
				}
				//cout << "收到信息：" << recvbuf << endl;
				else if (recv_result == 0)
					cout << "连接关闭" << endl;
				else {
					cout << "收到失败：" << WSAGetLastError() << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
			}
			while (1) {//客户端再发送消息
				cout << "client" << id << "：";
				memset(mes, '\0', sizeof(mes));
				cin.getline(mes, sizeof(mes));
				send_result = send(sockClient, mes, (int)strlen(mes), 0);
				if (send_result == SOCKET_ERROR) {
					cout << "发送消息失败：" << WSAGetLastError() << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
				//cout << "发送消息：" << mes << endl;
				if (strcmp(mes, "exit") == 0)
				{
					cout << "结束群聊" << endl;
					closesocket(sockClient);
					WSACleanup();
					return 1;
				}
				else if (strcmp(mes, "over") == 0) {
					if (i == amount) {
						i = 1;
					}
					else {
						i++;
					}
					break;
				}
			}
		}
	}

	closesocket(sockClient);
	WSACleanup();
	return 0;
}