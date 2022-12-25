#include "define.h"

#define PORT 7878
#define ADDRSRV "127.0.0.1"
#define MAX_FILE_SIZE 100000000
double MAX_TIME = CLOCKS_PER_SEC;
static u_int base_stage = 0;
static int windowSize = 16;
char fileBuffer[MAX_FILE_SIZE];

bool acceptClient(SOCKET &socket, SOCKADDR_IN &addr) {

    char *buffer = new char[sizeof(PacketHead)];
    int len = sizeof(addr);
    recvfrom(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &len);

    ShowPacket((Packet *) buffer);

    if ((((PacketHead *) buffer)->flag & SYN) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead)) == 0))
        cout << "第一次握手成功" << endl;
    else
        return false;
    base_stage = ((PacketHead *) buffer)->seq;

    PacketHead head;
    head.flag |= ACK;
    head.flag |= SYN;
    head.windows = windowSize;
    head.checkSum = CheckPacketSum((u_short *) &head, sizeof(PacketHead));
    memcpy(buffer, &head, sizeof(PacketHead));
    if (sendto(socket, buffer, sizeof(PacketHead), 0, (sockaddr *) &addr, len) == -1) {
        return false;
    }

    ShowPacket((Packet *) buffer);

    cout << "第二次握手成功" << endl;
    u_long imode = 1;
    ioctlsocket(socket, FIONBIO, &imode);//非阻塞

    clock_t start = clock(); //开始计时
    while (recvfrom(socket, buffer, sizeof(head), 0, (sockaddr *) &addr, &len) <= 0) {
        if (clock() - start >= MAX_TIME) {
            sendto(socket, buffer, sizeof(buffer), 0, (sockaddr *) &addr, len);
            start = clock();
        }
    }

    ShowPacket((Packet *) buffer);

    if ((((PacketHead *) buffer)->flag & ACK) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead)) == 0)) {
        cout << "第三次握手成功" << endl;
    } else {
        return false;
    }
    imode = 0;
    ioctlsocket(socket, FIONBIO, &imode);//阻塞

    cout << "与用户端成功建立连接，准备接收文件" << endl;
    return true;
}

bool disConnect(SOCKET &socket, SOCKADDR_IN &addr) {
    int addrLen = sizeof(addr);
    char *buffer = new char[sizeof(PacketHead)];

    recvfrom(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, &addrLen);

    ShowPacket((Packet *) buffer);

    if ((((PacketHead *) buffer)->flag & FIN) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead) == 0))) {
        cout << "第一次挥手完成，用户端断开" << endl;
    } else {
        return false;
    }

    PacketHead closeHead;
    closeHead.flag = 0;
    closeHead.flag |= ACK;
    closeHead.checkSum = CheckPacketSum((u_short *) &closeHead, sizeof(PacketHead));
    memcpy(buffer, &closeHead, sizeof(PacketHead));
    sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);

    ShowPacket((Packet *) buffer);
    cout << "第二次挥手完成" << endl;

    closeHead.flag = 0;
    closeHead.flag |= FIN;
    closeHead.checkSum = CheckPacketSum((u_short *) &closeHead, sizeof(PacketHead));
    memcpy(buffer, &closeHead, sizeof(PacketHead));
    sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);

    ShowPacket((Packet *) buffer);
    cout << "第三次挥手完成" << endl;

    u_long imode = 1;
    ioctlsocket(socket, FIONBIO, &imode);
    clock_t start = clock();
    while (recvfrom(socket, buffer, sizeof(PacketHead), 0, (sockaddr *) &addr, &addrLen) <= 0) {
        if (clock() - start >= MAX_TIME) {
            memcpy(buffer, &closeHead, sizeof(PacketHead));
            sendto(socket, buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);
            start = clock();
        }
    }

    ShowPacket((Packet *) buffer);
    if ((((PacketHead *) buffer)->flag & ACK) && (CheckPacketSum((u_short *) buffer, sizeof(PacketHead) == 0))) {
        cout << "第四次挥手完成，链接关闭" << endl;
    } else {
        return false;
    }
    closesocket(socket);
    return true;
}

Packet makePacket(u_int ack) {
    Packet pkt;
    pkt.head.ack = ack;
    pkt.head.flag |= ACK;
    pkt.head.checkSum = CheckPacketSum((u_short *) &pkt, sizeof(Packet));

    return pkt;
}

u_long recvFSM(char *fileBuffer, SOCKET &socket, SOCKADDR_IN &addr) {
    u_long fileLen = 0;
    int addrLen = sizeof(addr);
    u_int expectedSeq = base_stage;
    int dataLen;

    char *pkt_buffer = new char[sizeof(Packet)];
    Packet recvPkt, sendPkt = makePacket(base_stage - 1);
    clock_t start;
    bool clockStart = false;

    while (true) {
        memset(pkt_buffer, 0, sizeof(Packet));
        recvfrom(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, &addrLen);
        memcpy(&recvPkt, pkt_buffer, sizeof(Packet));

        if (recvPkt.head.flag & END && CheckPacketSum((u_short *) &recvPkt, sizeof(PacketHead)) == 0) {
            cout << "传输完毕" << endl;
            PacketHead endPacket;
            endPacket.flag |= ACK;
            endPacket.checkSum = CheckPacketSum((u_short *) &endPacket, sizeof(PacketHead));
            memcpy(pkt_buffer, &endPacket, sizeof(PacketHead));
            sendto(socket, pkt_buffer, sizeof(PacketHead), 0, (SOCKADDR *) &addr, addrLen);
            return fileLen;
        }

        if (recvPkt.head.seq == expectedSeq && CheckPacketSum((u_short *) &recvPkt, sizeof(Packet)) == 0) {
            //correctly receive the expected seq
            dataLen = recvPkt.head.bufSize;
            memcpy(fileBuffer + fileLen, recvPkt.data, dataLen);
            fileLen += dataLen;

            //give back ack=seq
            sendPkt = makePacket(expectedSeq);
            expectedSeq = (expectedSeq + 1) % MAX_SEQ;

            memcpy(pkt_buffer, &sendPkt, sizeof(Packet));
            sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);
            continue;
        }
        memcpy(pkt_buffer, &sendPkt, sizeof(Packet));
        sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);
    }
}

int main() {
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        //加载失败
        cout << "加载DLL失败" << endl;
        return -1;
    }
    SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(PORT);
    addrSrv.sin_addr.S_un.S_addr = inet_addr(ADDRSRV);
    bind(sockSrv, (SOCKADDR *) &addrSrv, sizeof(SOCKADDR));

    SOCKADDR_IN addrClient;

    //三次握手建立连接
    if (!acceptClient(sockSrv, addrClient)) {
        cout << "连接失败" << endl;
        return 0;
    }

    //char fileBuffer[MAX_FILE_SIZE];
    //可靠数据传输过程
    u_long fileLen = recvFSM(fileBuffer, sockSrv, addrClient);
    //四次挥手断开连接
    if (!disConnect(sockSrv, addrClient)) {
        cout << "断开失败" << endl;
        return 0;
    }

    //写入复制文件
    string filename = R"(D:\wtx\computer-network\computer-network\Lab\lab3\3-3\code\test\out\1_recv.jpg)";
    ofstream outfile(filename, ios::binary);
    if (!outfile.is_open()) {
        cout << "打开文件出错" << endl;
        return 0;
    }
    cout << fileLen << endl;
    outfile.write(fileBuffer, fileLen);
    outfile.close();

    cout << "文件复制完毕" << endl;
    return 1;
}