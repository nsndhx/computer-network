#include "define.h"

#define PORT 7879
#define ADDRSRV "127.0.0.1"

static SOCKADDR_IN addrSrv;
static int addrLen = sizeof(addrSrv);
double MAX_TIME = CLOCKS_PER_SEC;
static int windowSize = 16;
static unsigned long int base = 0;//握手阶段确定的初始序列号
static unsigned long int nextSeqNum = 0;
static Packet *sendPkt;
static unsigned long int sendIndex = 0, recvIndex = 0;
static bool stopTimer = false;
static clock_t start;
static int packetNum;

u_int waitingNum(u_int nextSeq) {
    if (nextSeq >= base)
        return nextSeq - base;
    return nextSeq + MAX_SEQ - base;
}

bool connectToServer(SOCKET &socket, SOCKADDR_IN &addr) {
    int len = sizeof(addr);

    PacketHead head;
    head.flag |= SYN;
    head.seq = base;
    head.checkSum = CheckPacketSum((u_short *) &head, sizeof(head));

    char *buffer = new char[sizeof(head)];
    memcpy(buffer, &head, sizeof(head));
    sendto(socket, buffer, sizeof(head), 0, (sockaddr *) &addr, len);
    //ShowPacket((Packet *) &head);
    cout << "第一次握手成功" << endl;

    clock_t start_connect = clock(); //开始计时
    while (recvfrom(socket, buffer, sizeof(head), 0, (sockaddr *) &addr, &len) <= 0) {
        if (clock() - start_connect >= MAX_TIME) {
            memcpy(buffer, &head, sizeof(head));
            sendto(socket, buffer, sizeof(buffer), 0, (sockaddr *) &addr, len);
            start_connect = clock();
        }
    }

    memcpy(&head, buffer, sizeof(head));
    ShowPacket((Packet *) &head);
    if ((head.flag & ACK) && (CheckPacketSum((u_short *) &head, sizeof(head)) == 0) && (head.flag & SYN)) {
        cout << "第二次握手成功" << endl;
    } else {
        return false;
    }

    windowSize = head.windows;
    //服务器建立连接
    head.flag = 0;
    head.flag |= ACK;
    head.checkSum = 0;
    head.checkSum = (CheckPacketSum((u_short *) &head, sizeof(head)));
    memcpy(buffer, &head, sizeof(head));
    sendto(socket, buffer, sizeof(head), 0, (sockaddr *) &addr, len);
    ShowPacket((Packet *) &head);

    //等待两个MAX_TIME，如果没有收到消息说明ACK没有丢包
    start_connect = clock();
    while (clock() - start_connect <= 2 * MAX_TIME) {
        if (recvfrom(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &len) <= 0)
            continue;
        //说明这个ACK丢了
        memcpy(buffer, &head, sizeof(head));
        sendto(socket, buffer, sizeof(head), 0, (sockaddr *) &addr, len);
        start_connect = clock();
    }
    cout << "三次握手成功" << endl;
    cout << "成功与服务器建立连接，准备发送数据" << endl;
    return true;
}

bool disConnect(SOCKET &socket, SOCKADDR_IN &addr) {
    addrLen = sizeof(addr);
    char *buffer = new char[sizeof(PacketHead)];
    PacketHead closeHead;
    closeHead.flag |= FIN;
    closeHead.checkSum = CheckPacketSum((u_short *) &closeHead, sizeof(PacketHead));

    memcpy(buffer, &closeHead, sizeof(PacketHead));
    if (sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen) != SOCKET_ERROR)
        cout << "第一次挥手成功" << endl;
    else
        return false;

    start = clock();
    while (recvfrom(socket, buffer, sizeof(PacketHead), 0, (sockaddr *) &addr, &addrLen) <= 0) {
        if (clock() - start >= MAX_TIME) {
            memcpy(buffer, &closeHead, sizeof(PacketHead));
            sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);
            start = clock();
        }
    }

    if ((((PacketHead *) buffer)->flag & ACK) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead) == 0))) {
        cout << "第二次挥手成功，客户端已经断开" << endl;
    } else {
        return false;
    }

    u_long mode = 0;
    ioctlsocket(socket, FIONBIO, &mode);//阻塞

    recvfrom(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &addrLen);

    if ((((PacketHead *) buffer)->flag & FIN) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead) == 0))) {
        cout << "第三次挥手成功,服务器已经断开" << endl;
    } else {
        return false;
    }

    mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);

    closeHead.flag = 0;
    closeHead.flag |= ACK;
    closeHead.checkSum = CheckPacketSum((u_short *) &closeHead, sizeof(PacketHead));

    memcpy(buffer, &closeHead, sizeof(PacketHead));
    sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);
    start = clock();
    while (clock() - start <= 2 * MAX_TIME) {
        if (recvfrom(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &addrLen) <= 0)
            continue;
        //说明这个ACK丢了
        memcpy(buffer, &closeHead, sizeof(PacketHead));
        sendto(socket, buffer, sizeof(PacketHead), 0, (sockaddr *) &addr, addrLen);
        start = clock();
    }

    cout << "第四次挥手成功，连接已关闭" << endl;
    closesocket(socket);
    return true;
}

Packet makePacket(u_int seq, char *data, int len) {
    Packet pkt;
    pkt.head.seq = seq;
    pkt.head.bufSize = len;
    memcpy(pkt.data, data, len);
    pkt.head.checkSum = CheckPacketSum((u_short *) &pkt, sizeof(Packet));
    return pkt;
}

bool inWindows(u_int seq) {
    if (seq >= base && seq < base + windowSize)
        return true;
    if (seq < base && seq < ((base + windowSize) % MAX_SEQ))
        return true;

    return false;
}

void sendFSM(u_long len, char *fileBuffer, SOCKET &socket, SOCKADDR_IN &addr) {

    packetNum = int(len / MAX_DATA_SIZE) + (len % MAX_DATA_SIZE ? 1 : 0);
    //sendIndex==packetNum时，不再发送；recvIndex==packetNum，收到全部ACK，结束传输
    sendIndex = 0, recvIndex = 0;
    int packetDataLen;
    addrLen = sizeof(addr);

    stopTimer = false;//是否停止计时
    char *data_buffer = new char[packetDataLen], *pkt_buffer = new char[sizeof(Packet)];
    Packet recvPkt;
    nextSeqNum = base;

    sendPkt[windowSize];
    cout << "本次文件数据长度为" << len << "Bytes,需要传输" << packetNum << "个数据包" << endl;

    while (true) {
        if (recvIndex == packetNum) {
            //recv全部ACK，结束传输，发送END报文
            PacketHead endPacket;
            endPacket.flag |= END;
            endPacket.checkSum = CheckPacketSum((u_short *) &endPacket, sizeof(PacketHead));
            memcpy(pkt_buffer, &endPacket, sizeof(PacketHead));
            sendto(socket, pkt_buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);

            while (recvfrom(socket, pkt_buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &addrLen) <= 0) {
                if (clock() - start >= MAX_TIME) {
                    start = clock();
                    goto resend;
                }
            }

            if ((((PacketHead *) (pkt_buffer))->flag & ACK) &&
                (CheckPacketSum((u_short *) pkt_buffer, sizeof(PacketHead)) == 0)) {
                cout << "文件传输完成" << endl;
                return;
            }

            resend:
            continue;
        }

        packetDataLen = min(MAX_DATA_SIZE, len - sendIndex * MAX_DATA_SIZE);

        //如果下一个序列号在滑动窗口中
        if (inWindows(nextSeqNum) && sendIndex < packetNum) {

            memcpy(data_buffer, fileBuffer + sendIndex * MAX_DATA_SIZE, packetDataLen);
            //缓存进入数组
            sendPkt[(int) waitingNum(nextSeqNum)] = makePacket(nextSeqNum, data_buffer, packetDataLen);
            memcpy(pkt_buffer, &sendPkt[(int) waitingNum(nextSeqNum)], sizeof(Packet));
            //发送给接收端
            sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);

            //如果目前窗口中只有一个数据报，开始计时（整个窗口共用一个计时器）
            if (base == nextSeqNum) {
                start = clock();
                stopTimer = false;
            }
            nextSeqNum = (nextSeqNum + 1) % MAX_SEQ;
            sendIndex++;
            //cout << sendIndex << "号数据包已经发送" << endl;
        }
        //判断是否有ACK到来
        while (recvfrom(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, &addrLen) > 0) {
            memcpy(&recvPkt, pkt_buffer, sizeof(Packet));
            //corrupt
            if (CheckPacketSum((u_short *) &recvPkt, sizeof(Packet)) != 0 || !(recvPkt.head.flag & ACK))
                goto time_out;
            //not corrupt
            if (base < (recvPkt.head.ack + 1)) {
                //不是窗口外的ACK
                int d = recvPkt.head.ack + 1 - base;
                for (int i = 0; i < (int) waitingNum(nextSeqNum) - d; i++) {
                    sendPkt[i] = sendPkt[i + d];
                }
                recvIndex += d;
                cout << "[window move]base:" << base << "\tnextSeq:" << nextSeqNum << "\tendWindow:"
                     << base + windowSize << endl;
            }
            base = (max((recvPkt.head.ack + 1), base)) % MAX_SEQ;
            //当窗口为空，停止计时
            if (base == nextSeqNum)
                stopTimer = true;
            else {
                start = clock();
                stopTimer = false;
            }

        }
        //超时发生，将数组中缓存的数据报全部重传一次，这就是Go Back N
        time_out:
        if (!stopTimer && clock() - start >= MAX_TIME) {
            //cout << "resend" << endl;
            for (int i = 0; i < (int) waitingNum(nextSeqNum); i++) {
                memcpy(pkt_buffer, &sendPkt[i], sizeof(Packet));
                sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);
            }
            cout << "第" << base << "号数据包超时重传" << endl;
            start = clock();
            stopTimer = false;
        }
    }
}

int main() {
    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        //加载失败
        cout << "加载DLL失败" << endl;
        return -1;
    }
    SOCKET sockClient = socket(AF_INET, SOCK_DGRAM, 0);

    u_long imode = 1;
    ioctlsocket(sockClient, FIONBIO, &imode);//非阻塞

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(PORT);
    addrSrv.sin_addr.S_un.S_addr = inet_addr(ADDRSRV);

    if (!connectToServer(sockClient, addrSrv)) {
        cout << "连接失败" << endl;
        return 0;
    }
    sendPkt = new Packet[windowSize];
    string filename = R"(D:\wtx\computer-network\computer-network\Lab\lab3\3-2\code\test\in\1.jpg)";

    ifstream infile(filename, ifstream::binary);

    if (!infile.is_open()) {
        cout << "无法打开文件" << endl;
        return 0;
    }

    infile.seekg(0, infile.end);
    u_long fileLen = infile.tellg();
    infile.seekg(0, infile.beg);
    cout << fileLen << endl;

    char *fileBuffer = new char[fileLen];
    infile.read(fileBuffer, fileLen);
    infile.close();
    cout << "开始传输" << endl;

    clock_t start_time = clock();
    sendFSM(fileLen, fileBuffer, sockClient, addrSrv);
    clock_t end_time = clock();
    cout << "传输总时间为:" << (end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
    cout << "吞吐率为:" << ((float) fileLen) / ((float) (end_time - start_time) / CLOCKS_PER_SEC) << "byte/s" << endl;

    if (!disConnect(sockClient, addrSrv)) {
        cout << "断开失败" << endl;
        return 0;
    }
    cout << "文件传输完成" << endl;
    return 1;
}