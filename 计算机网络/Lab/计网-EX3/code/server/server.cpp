#include"define.h"

#pragma warning(disable : 6031)
#pragma warning(disable : 4996)

#define TIMEOUT 5  //超时，单位s，代表一组中的所有ack都正确接收
#define WINDOWSIZE 20 //滑动窗口大小

SOCKADDR_IN addrServer;   //服务器地址
SOCKADDR_IN addrClient;   //客户端地址

SOCKET sockServer;//服务器套接字
SOCKET sockClient;//客户端套接字

auto ack = vector<int>(WINDOWSIZE);
int totalack = 0;//正确确认的数据包个数
int curseq = 0;//当前发送的数据包的序列号
int curack = 0;//当前等待被确认的数据包的序列号（最小）

//初始化工作
void inithandler()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	//套接字加载时错误提示 
	int err;
	//版本 2.2 
	wVersionRequested = MAKEWORD(2, 2);
	//加载 dll 文件 Scoket 库   
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		//找不到 winsock.dll 
		cout << "WSAStartup failed with error: " << err << endl;
		return;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		cout << "Could not find a usable version of Winsock.dll" << endl;
		WSACleanup();
	}
	sockServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//设置套接字为非阻塞模式 
	int iMode = 1; //1：非阻塞，0：阻塞 
	ioctlsocket(sockServer, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置 

	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(8080);
	err = bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
	if (err) {
		err = GetLastError();
		cout << "Could  not  bind  the  port" << 8080 << "for  socket. Error  code is" << err << endl;
		WSACleanup();
		return;
	}
	else
	{
		cout << "服务器创建成功" << endl;
	}
	for (int i = 0; i < WINDOWSIZE; i++)
	{
		ack[i] = 1;//初始都标记为1
	}
}

//超时重传
/*
void timeouthandler()
{
	packet* pkt1 = new packet;
	pkt1->init_packet();
	for (int i = curack; i != curseq; i = (i++) % seqnumber)
	{
		memcpy(pkt1, &buffer[i % WINDOWSIZE], BUFFER);
		sendto(sockServer, (char*)pkt1, BUFFER, 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
		cout << "重传第 " << i << " 号数据包" << endl;
	}
	ssthresh = cwnd / 2;
	cwnd = 1;
	STATE = SLOWSTART;//检测到超时，就回到慢启动状态
	cout << "==========================检测到超时，回到慢启动阶段============================" << endl;
	cout << "cwnd=  " << cwnd << "     sstresh= " << ssthresh << endl << endl;
}
*/


//差错检测
void ackhandler(unsigned int a) {

}


//流量控制：停等机制

int main() {
	//初始化
	inithandler();
	char filepath[20];//文件路径
	int totalpacket = 0;
	//读取文件
	cout << "请输入要发送的文件路径：";
	cin >> filepath;
	ifstream is(filepath, ifstream::in | ios::binary);//以二进制方式打开文件
	if (!is.is_open()) {
		cout << "文件无法打开!" << endl;
		exit(1);
	}
	is.seekg(0, std::ios_base::end);  //将文件流指针定位到流的末尾
	int length1 = is.tellg();
	totalpacket = length1 / 1024 + 1;
	cout << "文件大小为" << length1 << "Bytes,总共有" << totalpacket << "个数据包" << endl;
	is.seekg(0, std::ios_base::beg);  //将文件流指针重新定位到流的开始
	//建立连接
	packet* pkt = new packet;
	char t[1024] = { 0 };
	int recv_result = 0;
	int send_result = 0;
	
	while (true) {
		 memset(pkt, '\0', sizeof(*pkt));
		 recv_result = recv(sockServer, (char*)pkt, sizeof(packet), 0);
		 int count = 0;
		 int waitcount = 0;
		 while (recv_result < 0)
		 {
			 count++;
			 Sleep(100);
			 if (count > 20)
			 {
				 cout << "当前没有客户端请求连接！" << endl;
				 count = 0;
				 break;
			 }
		 }
		 //服务器收到客户端发来的TAG=0的数据报，标识请求连接
		 if (pkt->tag == 0) {
			 clock_t st = clock();
			 cout << "开始建立连接" << endl;
			 int stage = 0;
			 //bool runFlag = true;
			 //int waitCount = 0;
			 while (1) {
				 if (stage == 0) {
					 //发送100(第二次握手)
					 pkt = connecthandler(100, totalpacket);
					 send(sockServer, (char*)pkt, sizeof(packet), 0);
					 Sleep(100);
					 stage = 1;
				 }
				 if (stage == 1) {
					 //等待接收200阶段(开始传输)
					 memset(pkt, '\0', sizeof(*pkt));
					 recv_result = recv(sockServer, (char*)pkt, sizeof(packet), 0);
					 if (recv_result < 0) {
						 //++waitCount;
						 //runFlag = false;
						 cout << "connected false" << endl;
						 break;
					 }
					 else {
						 if (pkt->tag == 200) {
							 //发送文件
							 pkt->init_packet();
							 cout << "sending file" << endl;
							 memcpy(pkt->data, filepath, strlen(filepath));
							 pkt->len = strlen(filepath);
							 send(sockServer, (char*)pkt, sizeof(packet), 0);
							 stage = 2;
						 }
					 }
				 }
				 if (stage == 2) {
					 //确认对方接受成功
					 if (totalack == totalpacket) {
						 pkt->init_packet();
						 pkt->tag = 88;
						 cout << "数据传输完成" << endl;
						 send(sockServer, (char*)pkt, sizeof(packet), 0);
						 exit(0);
						 break;
					 }
					 //等待确认接受的数据包
					 pkt->init_packet();
					 recv_result = recv(sockServer, (char*)pkt, sizeof(packet), 0);
					 if (recv_result < 0) {
						 waitcount++;
						 Sleep(200);
						 if (waitcount > 20)
						 {
							 pkt->init_packet();
							 cout << "sending file" << endl;
							 memcpy(pkt->data, filepath, strlen(filepath));
							 pkt->len = strlen(filepath);
							 send(sockServer, (char*)pkt, sizeof(packet), 0);

							 waitcount = 0;
						 }
					 }
					 else {
						 //确认应答
						 if () {

						 }
					 }
				 }

			 }
		 }
		 
	}
	system("pause");
	return 0;
}