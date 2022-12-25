# 实验3：基于UDP服务设计可靠传输协议并编程实现

**2011269 王楠舟**

**实验3-3：**在实验3-2的基础上，选择实现一种拥塞控制算法，也可以是改进的算法，完成给定测试文件的传输。

## 程序功能介绍

1. 在实验3-2的基础上，实现**RENO拥塞控制算法**；
5. 发送端使用双线程，分别负责报文的发送和接收。

## 协议设计

### 数据报文结构

<img src="Lab3_2\img\image-20221125160247325.png" alt="image-20221125160247325" style="zoom:67%;" />

**差错检测--计算校验和；**

**面向连接的数据传输：建立连接与断开连接；**

**传输协议：使用基于滑动窗口的流量控制机制的rdt3.0；**

**累积确认。**

## 拥塞控制算法的实现：

<img src=".\Lab3_3\img\image-20221130205218453.png" alt="image-20221130205218453" style="zoom:80%;" />

初始发送端处于**慢启动**阶段，窗口大小设置为`cwnd = 1MSS`，`ssthresh = 32MSS`。

- 如果收到新的ACK，窗口大小增大1MSS。即在慢启动阶段每过一个RTT，cwnd翻倍，窗口大小呈指数增长；
- 如果收到被冗余的ACK报文，重复收到三次则认为出现了丢包，发送端将丢失报文重传，进入**快速恢复**阶段；
- 如果`cwnd > ssthresh`，发送端进入**拥塞避免**阶段；
- 如果超时没有收到新的ACK，重新进入**慢启动**阶段；

进入**拥塞避免**阶段后：

- 每次收到新的ACK，`cwnd = cwnd + MSS*(MSS/cwnd)`。即在拥塞避免阶段，每过一个RTT，cwnd加1，窗口大小线性增长；
- 如果收到被冗余的ACK报文，重复收到三次则认为出现了丢包，发送端将丢失报文重传，进入**快速恢复**阶段；
- 如果超时没有收到新的ACK，说明拥塞十分严重，`cwnd = 1MSS, ssthresh = cwnd.2`，发送端进入**慢启动阶段**；

进入**快速恢复**阶段后：

- 每次冗余的ACK，`cwnd += 1MSS`；
- 知道收到新的ACK，`cwnd = ssthresh`，将窗口大小恢复成阈值，然后进入**拥塞避免**阶段；
- 如果超时没有收到新的ACK，说明拥塞十分严重，`cwnd = 1MSS, ssthresh = cwnd.2`，发送端进入**慢启动阶段**；

## 算法代码实现

```C++
static u_long cwnd = MSS;
static u_long ssthresh = 10 * MSS;
static int dupACKCount = 0;

//sender缓冲区
static u_long lastSendByte = 0, lastAckByte = 0;
static Packet sendPkts[20]{};
```

对RENO算法的一些宏定义：

```c++
enum {
    START_UP, AVOID, RECOVERY
};

//Client端初始化的阶段
static int RENO_STAGE = START_UP;
```

算法代码的实现主要是在`Client`端的接收线程上修改：

线程函数：

```c++
DWORD WINAPI ACKHandler(LPVOID param) {
    SOCKET *clientSock = (SOCKET *) param;
    char recvBuffer[sizeof(Packet)];
    Packet recvPacket;

    while (true) {
        if (recvfrom(*clientSock, recvBuffer, sizeof(Packet), 0, (SOCKADDR *) &addrSrv, &addrLen) > 0) {
            memcpy(&recvPacket, recvBuffer, sizeof(Packet));
			......
```

如果接收到的ACK报文无误，且`base < (recvPacket.head.ack + 1)`是一个新的ACK，`Client`端首先进行窗口的滑动，然后根据目前所处在的RENO阶段来做处理：

- 如果处于**慢启动**阶段，`cwnd += d * MSS`，并判断cwnd是否超过阈值，如果超过则将RENO状态更新至**阻塞避免**阶段；
- 如果处于**阻塞避免**阶段，则`cwnd += d * MSS * MSS / cwnd`；
- 如果处于**快速恢复**阶段，`cwnd = ssthresh`，并将RENO状态更新进入**阻塞避免**阶段。

```
	if (CheckPacketSum((u_short *) &recvPacket, sizeof(Packet)) == 0 && recvPacket.head.flag & ACK) {
                mutexLock.lock();
                if (base < (recvPacket.head.ack + 1)) {
                    int d = recvPacket.head.ack + 1 - base;
                    //move the windows:
                    for (int i = 0; i < d; i++) {
                        lastAckByte += sendPkts[i].head.bufSize;
                    }
                    for (int i = 0; i < (int) waitingNum(nextSeqNum) - d; i++) {
                        sendPkts[i] = sendPkts[i + d];
                    }

                    switch (RENO_STAGE) {
                        case START_UP:
                            cwnd += d * MSS;
                            dupACKCount = 0;
                            if (cwnd >= ssthresh)
                                RENO_STAGE = AVOID;
                            break;
                        case AVOID:
                            cwnd += d * MSS * MSS / cwnd;
                            dupACKCount = 0;
                            break;
                        case RECOVERY:
                            cwnd = ssthresh;
                            dupACKCount = 0;
                            RENO_STAGE = AVOID;
                            break;
                    }
                    window = min(cwnd, windowSize);
                    base = (recvPacket.head.ack + 1) % MAX_SEQ;
                }
```

如果收到的ACK报文是冗余的，则进入`else`分支：`dupACKCount++`，并且如果处于`START_UP || AVOID`阶段，就会重传报文，并进入`RECOVERY`**快速恢复**阶段；如果处于`RECOVERY`阶段，则`cwnd += MSS`，增大窗口。

```c++
				else {
                    //duplicate ACK
                    dupACKCount++;
                    if (RENO_STAGE == START_UP || RENO_STAGE == AVOID) {
                        if (dupACKCount == 3) {
                            ssthresh = cwnd / 2;
                            cwnd = ssthresh + 3 * MSS;
                            RENO_STAGE = RECOVERY;

                            //retransmit missing segment
                            fastResend = true;
                        }
                    } else {
                        cwnd += MSS;
                    }
                    
                }
                mutexLock.unlock();
```

**在发送线程：**每次循环首先判断是否需要重传缓冲区：

```c++
void sendFSM(u_long len, char *fileBuffer, SOCKET &socket, SOCKADDR_IN &addr) {

    int packetDataLen;

    char *data_buffer = new char[sizeof(Packet)], *pkt_buffer = new char[sizeof(Packet)];
    nextSeqNum = base;
    cout << "本次文件数据长度为" << len << "Bytes" << endl;

    HANDLE ackhandler = CreateThread(nullptr, 0, ACKHandler, LPVOID(&socket), 0, nullptr);
    while (true) {

        if (lastAckByte == len) {
			//结束发送
            CloseHandle(ackhandler);
            ......
        }

        if (fastResend)
            goto GBN;
            ....
            
            
        GBN:
        mutexLock.lock();
        resendPacketNum = nextSeqNum - 1;
        for (int i = 0; i < nextSeqNum - base; i++) {
            memcpy(pkt_buffer, &sendPkts[i], sizeof(Packet));
            sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);
        }
        fastResend = false;
        mutexLock.unlock();
        start = clock();
        stopTimer = false;
```

如果是正常的发送窗口：计算出每次发送报文的数据段长度`packetDataLen`，取值为`min(MSS,窗口剩余大小,文件剩余大小)`，虽然保存到`sendPkts`缓冲区中。

```c++
mutexLock.lock();
window = min(cwnd, windowSize);
if ((lastSendByte < lastAckByte + window) && (lastSendByte < len)) {
    packetDataLen = min(lastAckByte + window - lastSendByte, MSS);
    packetDataLen = min(packetDataLen, len - lastSendByte);
    memcpy(data_buffer, fileBuffer + lastSendByte, packetDataLen);

    sendPkts[nextSeqNum - base] = makePacket(nextSeqNum, data_buffer, packetDataLen);
    memcpy(pkt_buffer, &sendPkts[nextSeqNum - base], sizeof(Packet));

    sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);

    if (base == nextSeqNum) {
        start = clock();
        stopTimer = false;
    }
    nextSeqNum = (nextSeqNum + 1) % MAX_SEQ;
    lastSendByte += packetDataLen;
}
mutexLock.unlock();
```

如果当前窗口超时：发送端将缓冲区中内容全部重传一次，然后进入`START_UP`阶段。

```c++
time_out:
if (!stopTimer && clock() - start >= MAX_TIME) {
    mutexLock.lock();
    ssthresh = cwnd / 2;
    cwnd = MSS;
    dupACKCount = 0;
    RENO_STAGE = START_UP;
    cout << "[time out!]resend" << endl;
    for (int i = 0; i < nextSeqNum - base; i++) {
        memcpy(pkt_buffer, &sendPkts[i], sizeof(Packet));
        sendto(socket, pkt_buffer, sizeof(Packet), 0, (SOCKADDR *) &addr, addrLen);
    }
    mutexLock.unlock();
    start = clock();
    stopTimer = false;
}
continue;
```

## 实验结果展示

启动`Router`程序，设置如下:

<img src="Lab3_1\img\image-20221113191054513.png" alt="image-20221113191054513" style="zoom: 67%;" />

```
//client端设置
#define PORT 7879

//server端设置
#define PORT 7878
#define ADDRSRV "127.0.0.1"
```

**三次握手测试：**

<img src="Lab3_3\img\image-20221201104626851.png" alt="image-20221201104626851" style="zoom: 80%;" />

<img src="F:\Computer_network\Computer_Network\Lab3\Lab3_3\img\image-20221201104723695.png" alt="image-20221201104723695" style="zoom:80%;" />

**文件传输过程：**

<img src="F:\Computer_network\Computer_Network\Lab3\Lab3_3\img\image-20221201104744858.png" alt="image-20221201104744858" style="zoom:80%;" />

Lab3_3中的日志输出内容主要是窗口大小的变化以及窗口的滑动。可以发现，在`START_UP`阶段，cwnd的值迅速增大，当`cwnd>=ssthresh`后进入了`AVOID`阶段，cwnd增速减缓。

<img src="Lab3_3\img\image-20221201105002806.png" alt="image-20221201105002806" style="zoom:80%;" />

当在`AVOID`阶段，`Duplicate ACK == 3`开始快速重传，发送端进入`RECOVERY`阶段，在`RECOVERY`阶段每收到一个冗余的ACK值`cwnd += MSS`，直到收到一个`New ACK`，发送端进入`AVOID`阶段。`cwnd,ssthresh`的值的变化都体现在上述过程中。

<img src="Lab3_2\img\image-20221125180432372.png" alt="image-20221125180432372" style="zoom:67%;" />

**对应接收端点的状态：**

<img src="Lab3_3\img\image-20221201105402823.png" alt="image-20221201105402823" style="zoom:80%;" />

序列号为`16`报文可能在传输过程发生丢包，没有按序到达，所以接收端始终在等候，抛弃其他错序到达的报文。而发送端在接收到三次重复的`ACK = 15`就触发了快速重传进入`RECOVERY`快速恢复阶段，而不是等待`TIME_OUT`发生。最后等到重传时发送端传输过来的`16`号报文，接收端应答`ACK=16`，发送端恢复到`AVOID`阶段。

**传输结果对比：**

<img src="Lab3_1\img\image-20221113192640466.png" alt="image-20221113192640466" style="zoom:67%;" />

![image-20221119133314159](Lab3_1\img\image-20221119133314159.png)

![image-20221119133336063](Lab3_1\img\image-20221119133336063.png)

![image-20221119133348934](Lab3_1\img\image-20221119133348934.png)

可见无论哪种类型的文件，传输前后都是一致的，验证了传输的可靠性。

## GitHub仓库

[仓库链接](https://github.com/Stupid-wangnz/Computer_Network/tree/main/Lab3/Lab3_3)