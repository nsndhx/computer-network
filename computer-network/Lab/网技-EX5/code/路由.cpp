#include "hhh.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define HAVE_REMOTE
#define ETH_ARP 0x0806
#define ARP_HARDWARE 1
#define ETH_IP 0x0800
#define ARP_REQUEST 1
#define ARP_REPLY 2
#define MAX_WORK_TIME 60


#pragma warning(disable:4996)

char** myip = (char**)malloc(sizeof(char*) * 2);
char** mynetmask = (char**)malloc(sizeof(char*) * 2);
char** mynet = (char**)malloc(sizeof(char*) * 2);

BYTE mymac[2][6];
BYTE broadcastmac[6];
pcap_if_t* alldevs;
pcap_t* adhandle;
pcap_addr_t myaddr[2];


void iptostr(u_long addr, char* str)
{
	static char str1[3 * 4 + 3 + 1];//3 bytes of numbers and 3 dots and a '\0'
	u_char* p = (u_char*)&addr;
	sprintf_s(str1, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	memcpy(str, str1, 16);
	return;
}

//路由表项
class RouteTableItem {
public:
	DWORD netmask;
	DWORD dstnet;//destination net
	DWORD nextip;
	int type;////0为直接连接，1为用户添加
	RouteTableItem* nextitem;//采用链表形式存储
	RouteTableItem() {
		memset(this, 0, sizeof(*this));
	}
	RouteTableItem(DWORD netmask, DWORD dstnet, int type, DWORD nextip = 0) {
		this->netmask = netmask;
		this->dstnet = dstnet;
		this->nextip = nextip;
		this->type = type;
	}
	void print() {
		char* str = (char*)malloc(sizeof(char) * 16);
		iptostr(netmask, str);
		printf("Netmask: %s\n", str);
		iptostr(dstnet, str);
		printf("Destination: %s\n", str);
		iptostr(nextip, str);
		printf("Next ip: %s\n", str);
		printf("Type: %d\n", type);
	}
};
//路由表
class RouteTable {
public:
	RouteTableItem* head;
	RouteTableItem* tail;
	int num;//how many items
	RouteTable() {
		DWORD netmask = inet_addr(mynetmask[0]);
		DWORD dstnet = (inet_addr(myip[0])) & (inet_addr(mynetmask[0]));
		int type = 0;
		head = new RouteTableItem(netmask, dstnet, type);
		tail = new RouteTableItem;//empty
		head->nextitem = tail;
		RouteTableItem* tmp = new RouteTableItem;
		tmp->dstnet = (inet_addr(myip[1])) & (inet_addr(mynetmask[1]));
		tmp->netmask = inet_addr(mynetmask[1]);
		tmp->type = 0;
		//tmp->nextip = (inet_addr(myip[1]));
		add(tmp);
		num = 2;//2 ip
	}
	//添加表项（直接投递在最前，前缀长的在前面）
	void add(RouteTableItem* newitem) {
		num++;
		//directly send
		if (newitem->type == 0) {
			newitem->nextitem = head->nextitem;//tail
			head->nextitem = newitem;
			return;
		}
		//add according to the len of the netmask
		RouteTableItem* cur = head;
		while (cur->nextitem != tail) {
			if (cur->nextitem->type != 0 && cur->nextitem->netmask < newitem->netmask) {
				break;
			}
			cur = cur->nextitem;
		}
		//insert between cur and cur->next
		newitem->nextitem = cur->nextitem;
		cur->nextitem = newitem;
	}
	//删除表项
	void remove(int index) {
		if (index >= num) {
			printf("路由表项%d超过范围!\n",index);
			return;
		}
		if (index == 0) { //delete head
			if (head->type == 0) {
				printf("该路由表项不可删除!\n");
			}
			else {
				head = head->nextitem;
			}
			return;
		}
		RouteTableItem* cur = head;
		int i = 0;
		while (i < index - 1 && cur->nextitem != tail) { //delete cur->next
			i++;
			cur = cur->nextitem;
		}
		if (cur->nextitem->type == 0) {
			printf("该路由表项不可删除!\n");
		}
		else {
			cur->nextitem = cur->nextitem->nextitem;
		}

	}
	//路由表打印
	void print() {
		printf("Route Table:\n");
		RouteTableItem* cur = head;
		int i = 1;
		while (cur != tail) {
			printf("No.%d:\n", i);
			cur->print();
			cur = cur->nextitem;
			i++;
		}
	}
	//查找，最长前缀,返回下一跳的ip
	DWORD lookup(DWORD dstip) {
		DWORD res;
		RouteTableItem* cur = head;
		while (cur != tail) {
			//printf("xxx\n");
			res = dstip & cur->netmask;
			//printf("res:%d\n", res);
			//printf("dstip: %d\n", dstip);
			//printf("cur->dstnet: %d\n", cur->dstnet);
			if (res == cur->dstnet) {
				if (cur->type != 0) {
					return cur->nextip;//need forward
				}
				else {
					return 0;//directly send
				}
			}
			cur = cur->nextitem;
		}
		printf("没有找到对应的路由表项!\n");
		return -1;
	}
};

class ARPTableItem {
public:
	DWORD IP;
	BYTE MAC[6];
	static int num;
	ARPTableItem() {
	}
	ARPTableItem(DWORD IP, BYTE MAC[6]) {
		this->IP = IP;
		for (int i = 0; i < 6; i++) {
			this->MAC[i] = MAC[i];
		}
		num = 0;
	}
	void print() {
		char* str = (char*)malloc(sizeof(char) * 16);
		iptostr(IP, str);
		printf("IP: %s\n", str);
		printf("MAC: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC[0], MAC[1], MAC[2],
			MAC[3], MAC[4], MAC[5]);
	}
	//插入
	static void insert(DWORD IP, BYTE MAC[6]) {
		arptable[num] = ARPTableItem(IP, MAC);
		num++;
	}
	//查询
	static bool lookup(DWORD ip, BYTE mac[6]) {
		memset(mac, 0, sizeof(mac));
		int i = 0;
		for (i; i < num; i++) {
			if (arptable[i].IP == ip) {
				for (int j = 0; j < 6; j++) {
					mac[j] = arptable[i].MAC[j];
				}
				return true;
			}
		}
		if (i == num) {
			printf("Error: no match ARP item!\n");
		}
		return false;
	}

}arptable[100];
int ARPTableItem::num = 0;

//初始化
void get_device() {

	char errbuf[PCAP_ERRBUF_SIZE];
	//获取网卡列表
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, 	//获取本机的接口设备
		NULL,			       //无需认证
		&alldevs, 		       //指向设备列表首部
		errbuf			      //出错信息保存缓存区
	) == -1)
	{
		//错误处理
		printf("Error: find devices failed! %s\n", errbuf);
		pcap_freealldevs(alldevs);
	}
	else {
		pcap_if_t* cur = alldevs;
		//打印信息
		printf("All devices:\n");
		for (int i = 1; cur != NULL; i++) {
			printf("No.%d: %s\n", i, cur->description);
			cur = cur->next;
		}
		//选择网口号
		printf("Choose an adapter: \n");
		int num = 0;
		scanf("%d", &num);
		for (int i = 0; i < num - 1; i++) {
			alldevs = alldevs->next;
		}
		if (alldevs == NULL)
		{
			printf("Error: cannot find the adapter!\n");
			pcap_freealldevs(alldevs);
			return;
		}
		//打开网口
		if ((adhandle = pcap_open(alldevs->name,          // 设备名
			65536,            // 要捕捉的数据包的部分
							  // 65535保证能捕获到不同数据链路层上的每个数据包的全部内容
			PCAP_OPENFLAG_PROMISCUOUS,    // 混杂模式
			1000,             // 读取超时时间
			NULL,             // 远程机器验证
			errbuf            // 错误缓冲池
		)) == NULL) 
		{
			printf("Error: adapter %s cannot be accessed!\n", alldevs->name);
			pcap_freealldevs(alldevs);
			return;
		}
		else {
			printf("Successfully open!\n");
			printf("Listening on %s...\n\n", alldevs->description);
		}
	}
}

//获取ip地址
void get_ip_netmask() {
	int i = 0;
	pcap_addr_t* addr = alldevs->addresses;
	for (addr; addr != NULL; addr = addr->next) {
		switch (addr->addr->sa_family)
		{
		case AF_INET:
			myaddr[i] = *addr;
			if (addr->addr) {
				//printf("myip[0]:%d\n", myip[0]);
				char* ip_str = (char*)malloc(sizeof(char) * 16);
				iptostr(((struct sockaddr_in*)addr->addr)->sin_addr.s_addr, ip_str);
				printf("My IP: %s\n", ip_str);
				myip[i] = (char*)malloc(sizeof(char) * 16);
				memcpy(myip[i], ip_str, 16);
			}
			if (addr->netmask) {
				char* netmask_str = (char*)malloc(sizeof(char) * 16);
				iptostr(((struct sockaddr_in*)addr->netmask)->sin_addr.s_addr, netmask_str);
				printf("My netmask: %s\n", netmask_str);
				mynetmask[i] = (char*)malloc(sizeof(char) * 16);
				memcpy(mynetmask[i], netmask_str, 16);
			}
			i++;
			break;
		case AF_INET6:
			break;
		}
	}
}

//arp请求
void ARP_request(DWORD sendip, DWORD recvip, BYTE sendmac[6]) {
	ARPFrame_t packet;
	memset(packet.FrameHeader.DesMAC, 0xff, 6);//broadcast
	memcpy(packet.FrameHeader.SrcMAC, sendmac, 6);
	memcpy(packet.SendHa, sendmac, 6);
	memset(packet.RecvHa, 0x00, 6);
	packet.FrameHeader.FrameType = htons(ETH_ARP);
	packet.HardwareType = htons(ARP_HARDWARE);
	packet.ProtocolType = htons(ETH_IP);
	packet.HLen = 6;
	packet.PLen = 4;
	packet.Operation = htons(ARP_REQUEST);
	packet.SendIP = sendip;
	packet.RecvIP = recvip;
	if (pcap_sendpacket(adhandle, (u_char*)&packet, sizeof(packet)) == -1)
	{
		printf("Sent ARP packet failed! Error: %d\n", GetLastError());
		return;
	}
	printf("Sent ARP packet succeed!\n");
	return;
}
void ARP_reply(DWORD recvip, BYTE mac[6]) {
	struct pcap_pkthdr* pkt_header;
	const u_char* pkt_data;
	memset(mac, 0, sizeof(mac));
	int i = 0;
	while ((pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0)
	{
		//find mac
		ARPFrame_t* tmp = (ARPFrame_t*)pkt_data;
		if (tmp->Operation == htons(ARP_REPLY)
			&& tmp->SendIP == recvip)
		{
			for (i = 0; i < 6; i++) {
				mac[i] = tmp->SendHa[i];
			}
			printf("Successfully get MAC!\n");
			printf("MAC: %02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2],
				mac[3], mac[4], mac[5]);
			char* ipstr = (char*)malloc(sizeof(char) * 16);
			iptostr(recvip, ipstr);
			printf("IP: %s\n", ipstr);
			break;
		}
	}
	if (i != 6)
	{
		printf("Failed to get MAC!\n");
	}
}
void get_other_mac(int index, char* ip, BYTE mac[6]) {
	ARP_request(inet_addr(myip[index]), inet_addr(ip), mymac[index]);
	ARP_reply(inet_addr(ip), mac);
}

//获取自身mac地址
void get_my_mac(int index) {
	BYTE sendmac[6] = { 1,1,1,1,1,1 };
	DWORD sendip = inet_addr("100.100.100.100");

	DWORD recvip = ((struct sockaddr_in*)myaddr[index].addr)->sin_addr.s_addr;
	ARP_request(sendip, recvip, sendmac);
	struct pcap_pkthdr* pkt_header;
	const u_char* pkt_data;
	int i = 0;
	while ((pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0)
	{
		//find my own mac
		ARPFrame_t* tmp = (ARPFrame_t*)pkt_data;
		if (tmp->Operation == htons(ARP_REPLY)
			&& tmp->RecvIP == sendip
			&& tmp->SendIP == recvip)
		{
			for (i = 0; i < 6; i++) {
				mymac[index][i] = tmp->SendHa[i];
			}
			//printf("Successfully get my MAC!\n");
			printf("My MAC: %02x-%02x-%02x-%02x-%02x-%02x\n\n", mymac[index][0], mymac[index][1], mymac[index][2],
				mymac[index][3], mymac[index][4], mymac[index][5]);
			break;
		}
	}
	if (i != 6)
	{
		printf("Failed to get my MAC!\n");
	}
}

bool checkchecksum(Data_t* data) {
	unsigned int sum = 0;
	WORD* word = (WORD*)&data->IPHeader;
	for (int i = 0; i < sizeof(IPHeader_t) / 2; i++) {
		sum += word[i];
		while (sum >= 0x10000) {
			int tmp = sum >> 16;
			sum -= 0x10000;
			sum += tmp;
		}
	}
	if (sum == 65535) {
		return true;
	}
	printf("错误的校验和!\n");
	return false;
}
//填充校验和
void setchecksum(Data_t* data) {
	data->IPHeader.Checksum = 0;
	unsigned int sum = 0;
	WORD* word = (WORD*)&data->IPHeader;
	for (int i = 0; i < sizeof(IPHeader_t) / 2; i++) {
		sum += word[i];
		while (sum >= 0x10000) {
			int tmp = sum >> 16;
			sum -= 0x10000;
			sum += tmp;
		}
	}
	data->IPHeader.Checksum = ~sum;
}

//change MACs
void sendpacket(ICMP_t data, BYTE dstmac[6]) {
	Data_t* tmp = (Data_t*)&data;
	memcpy(tmp->FrameHeader.SrcMAC, tmp->FrameHeader.DesMAC, 6);
	memcpy(tmp->FrameHeader.DesMAC, dstmac, 6);
	tmp->IPHeader.TTL--;
	if (tmp->IPHeader.TTL < 0) {
		printf("TTL invalid!\n");
		return;
	}
	setchecksum(tmp);
	if (pcap_sendpacket(adhandle, (const u_char*)tmp, 74) == 0) {
		printf("Forward an IP message:\n");
		printf("Src MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
			tmp->FrameHeader.SrcMAC[0], tmp->FrameHeader.SrcMAC[1],
			tmp->FrameHeader.SrcMAC[2], tmp->FrameHeader.SrcMAC[3],
			tmp->FrameHeader.SrcMAC[4], tmp->FrameHeader.SrcMAC[5]);
		printf("Des MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
			tmp->FrameHeader.DesMAC[0], tmp->FrameHeader.DesMAC[1],
			tmp->FrameHeader.DesMAC[2], tmp->FrameHeader.DesMAC[3],
			tmp->FrameHeader.DesMAC[4], tmp->FrameHeader.DesMAC[5]);
		char* src = (char*)malloc(sizeof(char) * 16);
		char* dst = (char*)malloc(sizeof(char) * 16);
		iptostr(tmp->IPHeader.SrcIP, src);
		iptostr(tmp->IPHeader.DstIP, dst);
		printf("Src IP: %s\n", src);
		printf("Des IP: %s\n", dst);
		printf("TTL: %d\n\n", tmp->IPHeader.TTL);
	}
}

bool MACcmp(BYTE MAC1[], BYTE MAC2[]) {
	for (int i = 0; i < 6; i++) {
		if (MAC1[i] != MAC2[i]) {
			return false;
		}
	}
	return true;
}

void work(RouteTable* routetable) {
	memset(broadcastmac, 0xff, 6);
	//printf("Begin to work...\n");
	clock_t start, end;
	start = clock();
	while (true) {
		end = clock();
		printf("time=%f\n", (double)(end - start) / CLK_TCK);
		if ((double)(end - start) / CLK_TCK > MAX_WORK_TIME) {
			printf("Timed out!\n");
			break;
		}
		pcap_pkthdr* pkt_header;
		const u_char* pkt_data;
		//捕获数据包
		//判断目的mac是否为自己的mac
		while (true) {
			if (pcap_next_ex(adhandle, &pkt_header, &pkt_data) > 0) {
				//printf("Get a packet!\n");
				FrameHeader_t* tmp = (FrameHeader_t*)pkt_data;
				if (MACcmp(tmp->DesMAC, mymac[0]) && (ntohs(tmp->FrameType) == ETH_IP)) {
					break;
				}
				continue;
			}
		}
		FrameHeader_t* frame_header = (FrameHeader_t*)pkt_data;

		if (MACcmp(frame_header->DesMAC, mymac[0])) {
			if (ntohs(frame_header->FrameType) == ETH_IP) {
				//printf("Get a packet!\n");
				Data_t* data = (Data_t*)pkt_data;

				//TODO:write to log ip("recieve",data)
				printf("\nRecieve an IP message:\n");
				printf("Src MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
					data->FrameHeader.SrcMAC[0], data->FrameHeader.SrcMAC[1],
					data->FrameHeader.SrcMAC[2], data->FrameHeader.SrcMAC[3],
					data->FrameHeader.SrcMAC[4], data->FrameHeader.SrcMAC[5]);
				printf("Des MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
					data->FrameHeader.DesMAC[0], data->FrameHeader.DesMAC[1],
					data->FrameHeader.DesMAC[2], data->FrameHeader.DesMAC[3],
					data->FrameHeader.DesMAC[4], data->FrameHeader.DesMAC[5]);
				char* src = (char*)malloc(sizeof(char) * 16);
				char* dst = (char*)malloc(sizeof(char) * 16);
				iptostr(data->IPHeader.SrcIP, src);
				iptostr(data->IPHeader.DstIP, dst);
				printf("Src IP: %s\n", src);
				printf("Des IP: %s\n", dst);
				printf("TTL: %d\n\n", data->IPHeader.TTL);


				DWORD dstip = data->IPHeader.DstIP;
				DWORD midip = routetable->lookup(dstip);//查找路由表中是否有对应表项
				if (midip == -1) {//如果没有则直接丢弃或直接递交至上层
					//printf("Error: no match item in route table!\n");
					continue;//do nothing
				}

				if (checkchecksum(data)) {//如果校验和不正确，则直接丢弃不进行处理
					if (data->IPHeader.DstIP != inet_addr(myip[0])
						&& data->IPHeader.DstIP != inet_addr(myip[1])) {
						//不是广播消息
						int res1 = MACcmp(data->FrameHeader.DesMAC, broadcastmac);
						int res2 = MACcmp(data->FrameHeader.SrcMAC, broadcastmac);
						if (!res1 && !res2) {
							//ICMP报文包含IP数据包报头和其它内容
							ICMP_t* icmp_ptr = (ICMP_t*)pkt_data;
							ICMP_t icmp = *icmp_ptr;
							BYTE* mac = (BYTE*)malloc(sizeof(BYTE) * 6);
							if (midip == 0) { //直接投递，查找目的IP的MAc
								//find arp
								if (ARPTableItem::lookup(dstip, mac) == 0) {
									printf("Cannot find matched ARP!\n");
									char* dst = (char*)malloc(sizeof(char) * 16);
									iptostr(dstip, dst);
									get_other_mac(0, dst, mac);
									ARPTableItem::insert(dstip, mac);
								}
								printf("\nnexthop: %s\n", dst);
								sendpacket(icmp, mac);
							}
							else if (midip != -1) { //非直接投递，查找下一条IP的MAC

								//find arp for midip								
								if (ARPTableItem::lookup(midip, mac) == 0) {
									printf("Cannot find matched ARP!\n");
									char* dst = (char*)malloc(sizeof(char) * 16);
									iptostr(midip, dst);
									//printf("999 %s\n", dst);
									get_other_mac(0, dst, mac);
									ARPTableItem::insert(midip, mac);
								}
								printf("\nnexthop: %s\n", dst);
								sendpacket(icmp, mac);
							}
						}
					}
				}
				else {
					printf("Error: wrong checksum!\n");
				}
			}
		}
	}
}

void endwork() {
	pcap_freealldevs(alldevs);
}

int main() {
	get_device();
	get_ip_netmask();
	RouteTable* routetable = new RouteTable();
	get_my_mac(0);

	routetable->print();


	printf("添加路由表项：\n");
	printf("Dst net: ");
	char dstnet[1024] = { 0 };
	scanf("%s", dstnet);
	printf("\nNetmask: ");
	char netmask[1024] = { 0 };
	scanf("%s", netmask);
	printf("\nNext Hop: ");
	char nexthop[1024] = { 0 };
	scanf("%s", nexthop);
	RouteTableItem* newitem = new RouteTableItem();
	newitem->dstnet = inet_addr(dstnet);
	newitem->netmask = inet_addr(netmask);
	newitem->nextip = inet_addr(nexthop);
	newitem->type = 1;
	routetable->add(newitem);
	printf("成功添加路由表项!\n");

	int d = 0;
	while (true) {
		printf("是否删除路由表项：");
		scanf("%d", &d);
		if (d != 0) {
			routetable->remove(d - 1);
			routetable->print();
		}
		else {
			break;
		}
	}

	ARPTableItem::insert(inet_addr(myip[0]), mymac[0]);
	ARPTableItem::insert(inet_addr(myip[1]), mymac[1]);

	routetable->print();
	for (int i = 0; i < ARPTableItem::num; i++) {
		arptable[i].print();
	}

	//char recvip[] = "192.168.0.101";
	//BYTE* mac = (BYTE*)malloc(sizeof(BYTE) * 6);
	//get_other_mac(0, recvip, mac);

	work(routetable);

	endwork();
	return 0;
}
