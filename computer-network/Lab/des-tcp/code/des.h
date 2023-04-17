#ifndef DES_H_
#define DES_H_
#include<cstring>
typedef  int INT32;
typedef  char INT8;
typedef  unsigned char  ULONG8;
typedef  unsigned short ULONG16;
typedef  unsigned long  ULONG32;
static const ULONG8  pc_first[64] = {/*初始置换IP*/
     58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
     62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
     57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
     61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7 
};
static const ULONG8 pc_last[64] = { /*逆初始置换IP-1*/
    40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25 
};
static const ULONG32  pc_by_bit[64] = { /*按位取值或赋值*/
    0x80000000L,0x40000000L,0x20000000L,0x10000000L, 0x8000000L,
    0x4000000L, 0x2000000L, 0x1000000L, 0x800000L, 0x400000L,
    0x200000L, 0x100000L,  0x80000L, 0x40000L, 0x20000L,0x10000L,
    0x8000L, 0x4000L, 0x2000L, 0x1000L, 0x800L, 0x400L, 0x200L,
    0x100L, 0x80L,0x40L,0x20L, 0x10L, 0x8L, 0x4L, 0x2L, 0x1L,
    0x80000000L,0x40000000L,0x20000000L,0x10000000L, 0x8000000L,
    0x4000000L, 0x2000000L, 0x1000000L, 0x800000L, 0x400000L,
    0x200000L, 0x100000L,  0x80000L, 0x40000L, 0x20000L, 0x10000L,
    0x8000L, 0x4000L, 0x2000L, 0x1000L, 0x800L, 0x400L, 0x200L,
    0x100L, 0x80L, 0x40L,0x20L, 0x10L, 0x8L,  0x4L, 0x2L, 0x1L, 
};
static const ULONG8 des_P[32] = {/*置换运算P*/
    16,7,20,21, 29,12,28,17, 1,15,23,26,
    5,18,31,10, 2,8,24,14, 32,27,3,9,
    19,13,30,6, 22,11,4,25 
};
static const ULONG8 des_E[48] = {/*数据扩展*/
    32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,
    12,13,14,15,16,17,16,17,18,19,20,21,
    20,21,22,23,24,25,24,25,26,27,28,29,
    28,29,30,31,32,1 
};
static const ULONG8 des_S[8][64] = {/*数据压缩*/
    {
        0xe,0x0,0x4,0xf,0xd,0x7,0x1,0x4,0x2,0xe,0xf,0x2,0xb,
        0xd,0x8,0x1,0x3,0xa,0xa,0x6,0x6,0xc,0xc,0xb,0x5,0x9,
        0x9,0x5,0x0,0x3,0x7,0x8,0x4,0xf,0x1,0xc,0xe,0x8,0x8,
        0x2,0xd,0x4,0x6,0x9,0x2,0x1,0xb,0x7,0xf,0x5,0xc,0xb,
        0x9,0x3,0x7,0xe,0x3,0xa,0xa,0x0,0x5,0x6,0x0,0xd
    },
    {
        0xf,0x3,0x1,0xd,0x8,0x4,0xe,0x7,0x6,0xf,0xb,0x2,0x3,
        0x8,0x4,0xf,0x9,0xc,0x7,0x0,0x2,0x1,0xd,0xa,0xc,0x6,
        0x0,0x9,0x5,0xb,0xa,0x5,0x0,0xd,0xe,0x8,0x7,0xa,0xb,
        0x1,0xa,0x3,0x4,0xf,0xd,0x4,0x1,0x2,0x5,0xb,0x8,0x6,
        0xc,0x7,0x6,0xc,0x9,0x0,0x3,0x5,0x2,0xe,0xf,0x9
    },
    {
        0xa,0xd,0x0,0x7,0x9,0x0,0xe,0x9,0x6,0x3,0x3,0x4,0xf,
        0x6,0x5,0xa,0x1,0x2,0xd,0x8,0xc,0x5,0x7,0xe,0xb,0xc,
        0x4,0xb,0x2,0xf,0x8,0x1,0xd,0x1,0x6,0xa,0x4,0xd,0x9,
        0x0,0x8,0x6,0xf,0x9,0x3,0x8,0x0,0x7,0xb,0x4,0x1,0xf,
        0x2,0xe,0xc,0x3,0x5,0xb,0xa,0x5,0xe,0x2,0x7,0xc
    },
    {
        0x7,0xd,0xd,0x8,0xe,0xb,0x3,0x5,0x0,0x6,0x6,0xf,0x9,
        0x0,0xa,0x3,0x1,0x4,0x2,0x7,0x8,0x2,0x5,0xc,0xb,0x1,
        0xc,0xa,0x4,0xe,0xf,0x9,0xa,0x3,0x6,0xf,0x9,0x0,0x0,
        0x6,0xc,0xa,0xb,0xa,0x7,0xd,0xd,0x8,0xf,0x9,0x1,0x4,
        0x3,0x5,0xe,0xb,0x5,0xc,0x2,0x7,0x8,0x2,0x4,0xe
    },
    {
        0x2,0xe,0xc,0xb,0x4,0x2,0x1,0xc,0x7,0x4,0xa,0x7,0xb,
        0xd,0x6,0x1,0x8,0x5,0x5,0x0,0x3,0xf,0xf,0xa,0xd,0x3,
        0x0,0x9,0xe,0x8,0x9,0x6,0x4,0xb,0x2,0x8,0x1,0xc,0xb,
        0x7,0xa,0x1,0xd,0xe,0x7,0x2,0x8,0xd,0xf,0x6,0x9,0xf,
        0xc,0x0,0x5,0x9,0x6,0xa,0x3,0x4,0x0,0x5,0xe,0x3
    },
    {
        0xc,0xa,0x1,0xf,0xa,0x4,0xf,0x2,0x9,0x7,0x2,0xc,0x6,
        0x9,0x8,0x5,0x0,0x6,0xd,0x1,0x3,0xd,0x4,0xe,0xe,0x0,
        0x7,0xb,0x5,0x3,0xb,0x8,0x9,0x4,0xe,0x3,0xf,0x2,0x5,
        0xc,0x2,0x9,0x8,0x5,0xc,0xf,0x3,0xa,0x7,0xb,0x0,0xe,
        0x4,0x1,0xa,0x7,0x1,0x6,0xd,0x0,0xb,0x8,0x6,0xd
    },
    {
        0x4,0xd,0xb,0x0,0x2,0xb,0xe,0x7,0xf,0x4,0x0,0x9,0x8,
        0x1,0xd,0xa,0x3,0xe,0xc,0x3,0x9,0x5,0x7,0xc,0x5,0x2,
        0xa,0xf,0x6,0x8,0x1,0x6,0x1,0x6,0x4,0xb,0xb,0xd,0xd,
        0x8,0xc,0x1,0x3,0x4,0x7,0xa,0xe,0x7,0xa,0x9,0xf,0x5,
        0x6,0x0,0x8,0xf,0x0,0xe,0x5,0x2,0x9,0x3,0x2,0xc
    },
    {
        0xd,0x1,0x2,0xf,0x8,0xd,0x4,0x8,0x6,0xa,0xf,0x3,0xb,
        0x7,0x1,0x4,0xa,0xc,0x9,0x5,0x3,0x6,0xe,0xb,0x5,0x0,
        0x0,0xe,0xc,0x9,0x7,0x2,0x7,0x2,0xb,0x1,0x4,0xe,0x1,
        0x7,0x9,0x4,0xc,0xa,0xe,0x8,0x2,0xd,0x0,0xf,0x6,0xc,
        0xa,0x9,0xd,0x0,0xf,0x3,0x3,0x5,0x5,0x6,0x8,0xb
    } 
};
static const ULONG8 keyleft[28] = {/*等分密钥*/
    57,49,41,33,25,17,9,1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,19,11,3,60,52,44,36 
};

static const ULONG8 keyright[28] = {/*等分密钥*/
    63,55,47,39,31,23,15,7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,21,13,5,28,20,12,4 
};
static const ULONG8 lefttable[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };/*密钥移位*/
static const ULONG8 keychoose[48] = {/*密钥选取*/
    14,17,11,24,1,5,3,28,15,6,21,10,
    23,19,12,4,26,8,16,7,27,20,13,2,
    41,52,31,37,47,55,30,40,51,45,33,48,
    44,49,39,56,34,53,46,42,50,36,29,32 
};

class CDesOperator {
private:
    ULONG32 firstKey[2];//用于生成16个子秘钥
    ULONG32 subKey[16][2];//用与保存生成的16个子秘钥,并参与加密运算
    bool makeFirstKey(ULONG32* orgKey);//生成初始秘钥
    bool oneStepOfMakeSubKey(ULONG32* left, ULONG32* right, INT32 number);//16轮迭代生成子秘钥的每一轮运算 参数三为生成的子秘钥序号
    bool oneStepOfMakeData(ULONG32* left, ULONG32* right, INT32 number);//16轮迭代加密或解密的每一轮运算 参数三为子秘钥的选取
    bool handleData(ULONG32* data, ULONG8 choice);//用于完成一次独立的加密或解密过程
public:
    CDesOperator() {
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 2; j++) {
                subKey[i][j] = 0;
            }
        }
        for (int i = 0; i < 2; i++) {
            firstKey[i] = 0;
        }
    }
    //加密 参数依次为  明文  明文长度  密文 密文长度  秘钥 秘钥长度
    bool Encry(char* pPlaintext, int nPlaintextLength, char* pCipherBuffer, int& nCipherBufferLength, char* pKey, int nKeyLength);
    //解密 参数依次为 密文 密文长度 明文 明文长度   秘钥  密钥长度
    bool Decry(char* pCipher, int nCipherBufferLength, char* pPlaintextBuffer, int& nPlaintextBufferLength, char* pKey, int nKeyLength);
};
bool CDesOperator::makeFirstKey(ULONG32* orgKey) {  //输入的orgKey为64位 输出为firstKey[2];
    ULONG32 tempKey[2] = { 0 };
    ULONG32* pFirstKey = (ULONG32*)firstKey;
    ULONG32* pTempKey = (ULONG32*)tempKey;
    memset((ULONG8*)firstKey, 0, sizeof(firstKey));
    memcpy((ULONG8*)&tempKey, (ULONG8*)orgKey, 8);
    memset((ULONG8*)subKey, 0, sizeof(subKey));
    int  j=0;
    for(j=0;j<28;j++){                                                                                                        //循环28次   64---->56     但还是要用2个32位来存储
        if(keyleft[j]>32){                                                                                                   //第一个32位
            if(pTempKey[1]&pc_by_bit[keyleft[j]-1]){                                                //第一次出现这种pc_by_bit[],此后涉及到选取特定的位都将用到
                pFirstKey[0]|=pc_by_bit[j];                                                                     //其实原理很简单  先判断一下要选取的bit数组对应的位是否为1
            }                                                                                                                         //通过与上0x80000000(1000 0000 0000 0000...)等只有一bit为1的数即可判断
        }                                                                                                                          //再将相应的位 置1通过或上0x80000000(1000 0000 0000 0000...)等只有一bit为1的数即可
        else{
            if(pTempKey[0]&pc_by_bit[keyleft[j]-1]){
                            pFirstKey[0]|=pc_by_bit[j];
                        }
        }
        if(keyright[j]>32){                                                                                    //第二个32位
                    if(pTempKey[1]&pc_by_bit[keyright[j]-1]){
                        pFirstKey[1]|=pc_by_bit[j];
                    }
                }
                else{
                    if(pTempKey[0]&pc_by_bit[keyright[j]-1]){
                                    pFirstKey[1]|=pc_by_bit[j];
                                }
                }
    }
    for (j = 0; j < 16; j++) {
        oneStepOfMakeSubKey(&pFirstKey[0], &pFirstKey[1], j);            //firstKey已形成，循环调用oneStepOfMakeSubKe()形成子秘钥

    }
    return true;
}
bool CDesOperator::oneStepOfMakeSubKey(ULONG32* left, ULONG32* right, INT32 number) {
    ULONG32 tempKey[2] = { 0,0 };
    ULONG32* pTempKey = (ULONG32*)tempKey;
    ULONG32* pSubKey = (ULONG32*)subKey[number];
    ULONG32 helpData[3] = { 0x0,0x80000000,0xc0000000 };//辅助数据,通过与上它们可以得到相应数据的最高位,待会有奇用
    pTempKey[0] = *left & helpData[lefttable[number]];
    pTempKey[1] = *right & helpData[lefttable[number]];
    if (lefttable[number] == 1) {                                   //注意要达到循环左移的效果,没有相应的操作,只有先将其最高位保存下来，在想办法将其移到低位上去
        pTempKey[0] >>= 27;                                          //具体实现：
        pTempKey[1] >>= 27;                                                 //与0xc0000000(110000000...高位为1)等数据相与得到高位(其他位被置0)
    }
    else {                                                                              //相与后的数据右移 将高位移到低位   由于只有28位 移26或27位即可，不必移30或31位
        pTempKey[0] >>= 26;                                                 //至此 最高位被移到了最低位(相对于28位) 接下来与左移的数据相或即可
        pTempKey[1] >>= 26;
    }
    pTempKey[0] &= 0xfffffff0;     //本来只有28位却必须用32位存储,直接将低4位清0即可
    pTempKey[1] &= 0xfffffff0;
    *left <<= lefttable[number];                                              // 左移
    *right <<= lefttable[number];
    *left |= pTempKey[0];                                                      //相或
    *right |= pTempKey[1];
    pTempKey[0] = 0;
    pTempKey[1] = 0;                                                              //至此循环左移结束 接下来56-->48

    int j = 0;
    for (; j < 48; j++) {
        if (j < 24) {
            if (*left & pc_by_bit[keychoose[j] - 1]) {
                pSubKey[0] |= pc_by_bit[j];
            }
        }
        else {
            if (*right & pc_by_bit[keychoose[j] - 28]) {
                pSubKey[1] |= pc_by_bit[j - 24];
            }
        }
    }
    return true;
}
bool CDesOperator::oneStepOfMakeData(ULONG32* left, ULONG32* right, INT32 number) {
    ULONG32 oldRight = *right;
    ULONG8 useBySBox[8] = { 0 };
    ULONG32 exdesP[2] = { 0 };  //用于存放拓展后的数据
     //32---->48
    int j = 0;
    for (; j < 48; j++) {                               //只对right做拓展
        if (j < 24)
        {
            if (*right & pc_by_bit[des_E[j] - 1])
            {
                exdesP[0] |= pc_by_bit[j];
            }
        }
        else
        {
            if (*right & pc_by_bit[des_E[j] - 1])
            {
                exdesP[1] |= pc_by_bit[j - 24];
            }
        }

    }
    for (j = 0; j < 2; j++) {
        exdesP[j] ^= subKey[number][j]; //子秘钥参与的异或运算
    }
    //48------>32
    exdesP[1] >>= 8;      //24位存放再32的,所以左移8位到最低位
    useBySBox[7] = (ULONG8)(exdesP[1] & 0x0000003fL);        //与上00000...00111111  低6位全为1
    exdesP[1] >>= 6;                                         //左移6位
    useBySBox[6] = (ULONG8)(exdesP[1] & 0x0000003fL);
    exdesP[1] >>= 6;
    useBySBox[5] = (ULONG8)(exdesP[1] & 0x0000003fL);
    exdesP[1] >>= 6;
    useBySBox[4] = (ULONG8)(exdesP[1] & 0x0000003fL);


    exdesP[0] >>= 8;
    useBySBox[3] = (ULONG8)(exdesP[0] & 0x0000003fL);
    exdesP[0] >>= 6;
    useBySBox[2] = (ULONG8)(exdesP[0] & 0x0000003fL);
    exdesP[0] >>= 6;
    useBySBox[1] = (ULONG8)(exdesP[0] & 0x0000003fL);
    exdesP[0] >>= 6;
    useBySBox[0] = (ULONG8)(exdesP[0] & 0x0000003fL);
    exdesP[0] = 0;
    exdesP[1] = 0;                                                    //至此数据被分为8组,每组6位(尽管必须用8位存储)
    *right = 0;
    for (j = 0; j < 7; j++) {                                                   //查SBox表  6位变4位     即48---->32
        *right |= des_S[j][useBySBox[j]];
        *right <<= 4;
    }
    *right |= des_S[j][useBySBox[j]];
    ULONG32 tempData = 0;
    for (j = 0; j < 32; j++)                                                    //不必多说 换位
    {
        if (*right & pc_by_bit[des_P[j] - 1])
        {
            tempData |= pc_by_bit[j];
        }
    }
    *right = tempData;

    *right ^= *left;                           //传的是指针 ,用于迭代
    *left = oldRight;
    return true;
}
bool CDesOperator::handleData(ULONG32* left, ULONG8 choice) {

    int j = 0;
    ULONG32* right = &left[1];
    ULONG32 tempData[2] = { 0 };
    for (j = 0; j < 64; j++) {
        if (j < 32) {
            if (pc_first[j] > 32) {
                if (*right & pc_by_bit[pc_first[j] - 1]) {
                    tempData[0] |= pc_by_bit[j];
                }
            }
            else {
                if (*left & pc_by_bit[pc_first[j] - 1]) {
                    tempData[0] |= pc_by_bit[j];
                }
            }
        }
        else {                                   //j>32
            if (pc_first[j] > 32) {
                if (*right & pc_by_bit[pc_first[j] - 1]) {
                    tempData[1] |= pc_by_bit[j];
                }
            }
            else {
                if (*left & pc_by_bit[pc_first[j] - 1]) {
                    tempData[1] |= pc_by_bit[j];
                }
            }
        }
    }
    *left = tempData[0];
    *right = tempData[1];
    tempData[0] = 0;
    tempData[1] = 0;
    int number = 0;
    switch (choice) {
    case 0:                                                                                 //加密
        for (number = 0; number < 16; number++) {
            oneStepOfMakeData(left, right, (ULONG32)number);          //16轮迭代
        }
        break;
    case 1:                                                                               //解密
        for (number = 15; number >= 0; number--) {
            oneStepOfMakeData(left, right, (ULONG32)number);              //16轮迭代
        }
        break;
    default:break;
    }
    ULONG32 temp;
    temp = *left;
    *left = *right;
    *right = temp;
    for (j = 0; j < 64; j++)
    {
        if (j < 32)
        {
            if (pc_last[j] > 32)/*属于right*/
            {
                if (*right & pc_by_bit[pc_last[j] - 1])
                {
                    tempData[0] |= pc_by_bit[j];
                }
            }
            else
            {
                if (*left & pc_by_bit[pc_last[j] - 1])
                {
                    tempData[0] |= pc_by_bit[j];
                }
            }
        }
        else
        {
            if (pc_last[j] > 32)/*属于right*/
            {
                if (*right & pc_by_bit[pc_last[j] - 1])
                {
                    tempData[1] |= pc_by_bit[j];
                }
            }
            else
            {
                if (*left & pc_by_bit[pc_last[j] - 1])
                {
                    tempData[1] |= pc_by_bit[j];
                }
            }
        }
    }
    *left = tempData[0];
    *right = tempData[1];
    return true;
}
bool CDesOperator::Encry(char* pPlaintext, int nPlaintextLength, char* pCipherBuffer, int& nCipherBufferLength, char* pKey, int nKeyLength) {
    if (nKeyLength != 8) { return false; }
    makeFirstKey((ULONG32*)pKey);
    int length = ((nPlaintextLength + 7) / 8) * 2;      //length×4后肯定比原文长而且为64的整数倍
    if (nCipherBufferLength < length * 4) {                  //密文不够长
        nCipherBufferLength = length * 4;
    }
    memset(pCipherBuffer, 0, nCipherBufferLength);//密文置0
    ULONG32* output = (ULONG32*)pCipherBuffer;
    ULONG32* source;
    if (nPlaintextLength != sizeof(ULONG32) * length)     //确保明文64位对齐
    {
        source = new ULONG32[length];
        memset(source, 0, sizeof(ULONG32) * length);
        memcpy(source, pPlaintext, nPlaintextLength);
    }
    else
    {
        source = (ULONG32*)pPlaintext;
    }
    ULONG32 msg[2] = { 0,0 };
    for (int i = 0; i < (length / 2); i++) {                                     //每64位为一次加密
        msg[0] = source[2 * i];                 //64位为两个long，2位进行一次加密，下同
        msg[1] = source[2 * i + 1];
        handleData(msg, (ULONG8)0);                             //加密
        output[2 * i] = msg[0];                                //得到密文
        output[2 * i + 1] = msg[1];
    }
    if (pPlaintext != (char*)source)
    {
        delete[]source;
    }
    return true;
}
bool CDesOperator::Decry(char* pCipher, int nCipherBufferLength, char* pPlaintextBuffer, int& nPlaintextBufferLength, char* pKey, int nKeyLength) {
    if (nCipherBufferLength % 8 != 0)
    {
        return false;
    }
    if (nPlaintextBufferLength < nCipherBufferLength)            //与加密过程类似,不必多说
    {
        nPlaintextBufferLength = nCipherBufferLength;
        return false;
    }
    if (nKeyLength != 8)
    {
        return false;
    }
    makeFirstKey((ULONG32*)pKey);
    memset(pPlaintextBuffer, 0, nPlaintextBufferLength);
    ULONG32* pSouce = (ULONG32*)pCipher;
    ULONG32* pDest = (ULONG32*)pPlaintextBuffer;
    ULONG32 gp_msg[2] = { 0,0 };
    for (int i = 0; i < (nCipherBufferLength / 8); i++)
    {
        gp_msg[0] = pSouce[2 * i];
        gp_msg[1] = pSouce[2 * i + 1];
        handleData(gp_msg, (ULONG8)1);
        pDest[2 * i] = gp_msg[0];
        pDest[2 * i + 1] = gp_msg[1];
    }
    return true;
}
#endif /* DES_H_ */
#pragma once
