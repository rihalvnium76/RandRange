//RandRange_v1.5.5
//--std=c++11
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cstdint> // uint_64t
#include<ctime> // time()
#include<cmath> // pow()
#include<cctype> // isspace()...

#include<vector> // vector
#include<string> // string
#include<exception> // exception

#define RNDRG_VERSION "1.5.6 - CLI Demo"
// 调试输出
#define dbgt(T) const char *TAG = #T;
#define dbgoe(M, FMT, ...) fprintf(stderr, "[" #M "]%s#%d: " FMT, TAG, __LINE__, ##__VA_ARGS__);
#define dbgos(M, FMT, ...) printf("[" #M "]%s#%d: " FMT, TAG, __LINE__, ##__VA_ARGS__);
#define dbgv(FMT, ...) dbgoe(V, FMT, ##__VA_ARGS__)
#define dbgi(FMT, ...) dbgoe(I, FMT, ##__VA_ARGS__)
#define dbgd(FMT, ...) dbgoe(D, FMT, ##__VA_ARGS__)
#define dbgw(FMT, ...) dbgoe(W, FMT, ##__VA_ARGS__)
#define dbge(FMT, ...) dbgoe(E, FMT, ##__VA_ARGS__)

namespace Module {
    /* Random Generator Classes */
    class RNDC { // RandCustom
    public:
        // Algorithm : xoshiro256**
        struct xoshiro256ss {
            inline uint64_t rotl(const uint64_t x, int k) {
                return (x << k) | (x >> (64 - k));
            }

            uint64_t s[4]; // not be zero

            uint64_t next(void) {
                const uint64_t result = rotl(s[1] * 5, 7) * 9;

                const uint64_t t = s[1] << 17;

                s[2] ^= s[0];
                s[3] ^= s[1];
                s[1] ^= s[2];
                s[0] ^= s[3];

                s[2] ^= t;

                s[3] = rotl(s[3], 45);

                return result;
            }
        };

        // Algorithm : SplitMix64
        struct SplitMix64 {
            uint64_t x; /* The state can be seeded with any value. */
            uint64_t next() {
                uint64_t z = (x += 0x9e3779b97f4a7c15);
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
                z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
                return z ^ (z >> 31);
            }
        };
    }; // class RNDC

    class RNDRG : public RNDC { // RandomRange
        xoshiro256ss x256ss;
        unsigned int seed;
    public:
        // 设置种子
        RNDRG(unsigned int seed = time(NULL)) {
            Seed(seed);
        }
        void Seed(unsigned int seed) {
            SplitMix64 sm64;
            sm64.x = seed;
            this->seed = seed;
            for(int i=0; i<4; ++i) x256ss.s[i] = sm64.next();
        }
        unsigned int Seed() {
            return this->seed;
        }

        // 随机整数，[0,0xFFFFFFFF]
        inline uint64_t RI() {
            return x256ss.next();
        }
        // 随机浮点数，[0,1)
        inline double RF() {
            const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | RI() >> 12 };
            return u.d - 1.0;
        }

        // 返回范围为[a,b]的随机整数
        int ir(int a, int b) {
            return RI() % (b-a+1) + a;
        }
        // 返回n个范围为[a,b]的随机整数之和
        int irs(int a, int b, int n) {
            int s = 0;
            for(int i=0; i<n; ++i) s += ir(a, b);
            return s;
        }
        // 返回范围为[a,b)的随机浮点数
        double fr(double a, double b) {	
            return RF() * (b-a) + a;
        }
        // 返回n个范围为[a,b)的随机浮点数之和
        double frs(double a, double b, int n) {
            double s = 0;
            for(int i=0; i<n; ++i) s += fr(a, b);
            return s;
        }

        //精度限制。a数字，d小数位数,d<0不限制
        static double pl(double a,int d) {
            if(d>=0) {
                double s=pow(10,d);
                a=(int)(a*s)/s;
            }
            return a;
        }
    }; // class RNDRG

    /* Error Code Class */
    class ErrorCodeException : public std::exception {	
    protected:
        std::string ret;
        ErrorCodeException(std::string baseErrInfo) {
            ret = baseErrInfo;
        }
        ErrorCodeException(std::string baseErrInfo, std::string extErrInfo) {
            ret = baseErrInfo + "\n" + extErrInfo;
        }
    public:
        ErrorCodeException()
            : ErrorCodeException("FATAL: Unknown Error Code") {}

        const char *what() const throw() {
            return ret.c_str();
        }
    }; // class ErrorCodeException (Base Class)
    class OrderSyntaxException : public ErrorCodeException {
    public:
        OrderSyntaxException()
            : ErrorCodeException("ERROR: Instruction syntax error") {}
    }; // class OrderSyntaxException
    class InvalidOrderException : public ErrorCodeException {
    public:
        InvalidOrderException()
            : ErrorCodeException("ERROR: Invalid instruction") {}
    }; // class InvalidOrderException
    class ScriptInterpreterError : public ErrorCodeException {
    public:
        ScriptInterpreterError(std::string extErrInfo)
            : ErrorCodeException("ERROR: Script parsing error", extErrInfo) {}
    };

    /* Public Variant */
    RNDRG Dice;

    /* Tool Function/Class  */
    static inline void ClearStdin() { // 清空输入缓冲区
        scanf("%*[^'\n']%*c");
    }
    void Trim(char *s) { // 去首尾空白字符，参数不能为字符常量
        char *p = (char *)1, *q = 0, *c;
        for (c = s; *c; ++c) if (!isspace(*c)) p = q ? p : c, q = c;
        for (c = s; p <= q; *c++ = *p++);
        *c = 0;
    } // by Edgar
    class Split {
        char *str, *curPos, *nextPos;
    public:
        Split(const char *s) : str(NULL), curPos(NULL), nextPos(NULL) {
            int len = strlen(s);
            if(len>0) {
                curPos = str = new char[len];
                strcpy(str, s);
            }
        }
        ~Split() {
            if(str) delete[] str;
        }
        char *GetNextPos() { return nextPos; }
        // 切割字符串
        // 返回值：指向下个记号起始的指针
        // delim 分隔符
        // ignoreIncludedDelim 是否忽略包在括号或引号内的分隔符
        char *SplitByChar(char delim, bool ignoreIncludedDelim) {
            // 是否为有字符串或到结尾
            if(!curPos || !curPos[0]) return NULL;
            int balance[5]={}; // " ' ( [ {
            char c;
            // p1 curPos从分隔符扫到普通字符
            for(; *curPos && *curPos==delim; ++curPos) *curPos = '\0';
            // p2 nextPos从普通字符扫到分隔符
            for(nextPos = curPos; c = *nextPos; ++nextPos) {
                bool isBalance = !ignoreIncludedDelim || (ignoreIncludedDelim && !(balance[0] || balance[1] || balance[2] || balance[3] || balance[4]));
                if(c==delim && isBalance) {
                    *nextPos = '\0';
                    ++nextPos; break;
                }
                else if(ignoreIncludedDelim) {
                    if(c=='(') ++balance[2];
                    else if(c==')' && balance[2]>0) --balance[2];
                    else if(c=='[') ++balance[3];
                    else if(c==']' && balance[3]>0) --balance[3];
                    else if(c=='{') ++balance[4];
                    else if(c=='}' && balance[4]>0) --balance[4];
                    else if(c=='\"') balance[1] ^= 1;
                    else if(c=='\'') balance[0] ^= 1;
                }
            }
            char *rt = curPos; curPos = nextPos;
            return rt;
        }
        static void Test() {
            Split ss(" AA \"BB (CC\" )DD");
            char *t;
            do{
                t=ss.SplitByChar(0, true);
                printf("%s|\n",t);
            }while(t);
        }
    }; // class Split

    // Base64加解密
    // 代码源自https://github.com/zhicheng/base64
    namespace Base64Code {
        // NOTICE: str_len = size + 1
        static const char
            BASE64_PAD = '=', BASE64DE_FIRST = '+', BASE64DE_LAST = 'z';

        inline static unsigned int BASE64_ENCODE_OUT_SIZE(unsigned int s) {
            return (unsigned int)((((s) + 2) / 3) * 4 + 1);
        }
        inline static unsigned int BASE64_DECODE_OUT_SIZE(unsigned int s) {
            return (unsigned int)(((s) / 4) * 3);
        }

        /* BASE 64 encode table */
        static const char base64en[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/',
        };
        /* ASCII order for BASE 64 decode, 255 in unused character */
        static const unsigned char base64de[] = {
            /* nul, soh, stx, etx, eot, enq, ack, bel, */
               255, 255, 255, 255, 255, 255, 255, 255,
            /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
               255, 255, 255, 255, 255, 255, 255, 255,
            /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
               255, 255, 255, 255, 255, 255, 255, 255,
            /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
               255, 255, 255, 255, 255, 255, 255, 255,
            /*  sp, '!', '"', '#', '$', '%', '&', ''', */
               255, 255, 255, 255, 255, 255, 255, 255,
            /* '(', ')', '*', '+', ',', '-', '.', '/', */
               255, 255, 255,  62, 255, 255, 255,  63,
            /* '0', '1', '2', '3', '4', '5', '6', '7', */
                52,  53,  54,  55,  56,  57,  58,  59,
            /* '8', '9', ':', ';', '<', '=', '>', '?', */
                60,  61, 255, 255, 255, 255, 255, 255,
            /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
               255,   0,   1,  2,   3,   4,   5,    6,
            /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
                7,   8,   9,  10,  11,  12,  13,  14,
            /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
                15,  16,  17,  18,  19,  20,  21,  22,
            /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
                23,  24,  25, 255, 255, 255, 255, 255,
            /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
               255,  26,  27,  28,  29,  30,  31,  32,
            /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
                33,  34,  35,  36,  37,  38,  39,  40,
            /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
                41,  42,  43,  44,  45,  46,  47,  48,
            /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
                49,  50,  51, 255, 255, 255, 255, 255
        };

        // return value is the length of output string
        unsigned int
        base64_encode(const unsigned char *in, unsigned int inlen, char *out)
        {
            int s;
            unsigned int i;
            unsigned int j;
            unsigned char c;
            unsigned char l;
            s = 0;
            l = 0;
            for (i = j = 0; i < inlen; i++) {
                c = in[i];
                switch (s) {
                case 0:
                    s = 1;
                    out[j++] = base64en[(c >> 2) & 0x3F];
                    break;
                case 1:
                    s = 2;
                    out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
                    break;
                case 2:
                    s = 0;
                    out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
                    out[j++] = base64en[c & 0x3F];
                    break;
                }
                l = c;
            }
            switch (s) {
            case 1:
                out[j++] = base64en[(l & 0x3) << 4];
                out[j++] = BASE64_PAD;
                out[j++] = BASE64_PAD;
                break;
            case 2:
                out[j++] = base64en[(l & 0xF) << 2];
                out[j++] = BASE64_PAD;
                break;
            }
            out[j] = 0;
            return j;
        }
        unsigned int
        base64_decode(const char *in, unsigned int inlen, unsigned char *out)
        {
            unsigned int i;
            unsigned int j;
            unsigned char c;
            if (inlen & 0x3) {
                return 0;
            }
            for (i = j = 0; i < inlen; i++) {
                if (in[i] == BASE64_PAD) {
                    break;
                }
                if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST) {
                    return 0;
                }
                c = base64de[(unsigned char)in[i]];
                if (c == 255) {
                    return 0;
                }
                switch (i & 0x3) {
                case 0:
                    out[j] = (c << 2) & 0xFF;
                    break;
                case 1:
                    out[j++] |= (c >> 4) & 0x3;
                    out[j] = (c & 0xF) << 4; 
                    break;
                case 2:
                    out[j++] |= (c >> 2) & 0xF;
                    out[j] = (c & 0x3) << 6;
                    break;
                case 3:
                    out[j++] |= c;
                    break;
                }
            }
            return j;
        }
    } // namespace Base64Code

    // Baseplus64加解密
    // 依赖代码：上述的Base64加解密函数
    class Baseplus64Code {
        std::string str;
        inline static const char* bp64EnTable() {
            static const char enAlpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
            return enAlpha;
        }
        inline static const unsigned char* bp64DeTable() {
            static const unsigned char deAlpha[] = {
                /* nul, soh, stx, etx, eot, enq, ack, bel, */
                   255, 255, 255, 255, 255, 255, 255, 255,
                /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
                   255, 255, 255, 255, 255, 255, 255, 255,
                /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
                   255, 255, 255, 255, 255, 255, 255, 255,
                /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
                   255, 255, 255, 255, 255, 255, 255, 255,
                /*  sp, '!', '"', '#', '$', '%', '&', ''', */
                   255, 255, 255, 255, 255, 255, 255, 255,
                /* '(', ')', '*', '+', ',', '-', '.', '/', */
                   255, 255, 255, 255, 255, 255,  46, 255,
                /* '0', '1', '2', '3', '4', '5', '6', '7', */
                    35,  26,  27,  28,  29,  30,  31,  32,
                /* '8', '9', ':', ';', '<', '=', '>', '?', */
                    33,  34, 255, 255, 255, 255, 255, 255,
                /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
                   255,   0,   1,  2,   3,   4,   5,    6,
                /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
                    7,   8,   9,  10,  11,  12,  13,  14,
                /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
                    15,  16,  17,  18,  19,  20,  21,  22,
                /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
                    23,  24,  25, 255, 255, 255, 255,  95,
                /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
                   255,   0,   1,   2,   3,   4,   5,   6,
                /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
                    7,   8,   9,  10,  11,  12,  13,  14,
                /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
                    15,  16,  17,  18,  19,  20,  21,  22,
                /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
                    23,  24,  25, 255, 255, 255, 255, 255
            };
            return deAlpha;
        }
        inline static const char* vigenereTable() {
            static const char vAlpha[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210_.ZYXWVUTSRQPONMLKJIHGFEDCBA987654321_";
            return vAlpha;
        }
    public:
        // TODO 全局空字符串检测
        // s为需要处理的加/解密字符串
        Baseplus64Code(const char *s) : str(s) {}
        // 获取结果
        std::string GetResult() { return str; }

        // 文本->Base64
        Baseplus64Code& Base64Encode() {
            char *t; int len = str.length();
            t = new char[Base64Code::BASE64_ENCODE_OUT_SIZE(len)];
            Base64Code::base64_encode(reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str())), len, t);
            str = t;
            delete[] t;
            return *this;
        }
        // Base64->文本
        // 格式：等号数量标识符(1位，R0/W1/X2)+[标记小/大写字母位置模式(1位,AB,CD,EF...YZ循环)+小/大写字母位置(0-18位，除去作为分隔符的字母)+分隔符(1位,Z,Y,X...A循环)+全小/大写的Base64编码片段(35位)]*n
        Baseplus64Code& Base64Decode() {
            char *t; int len = str.length();
            t = new char[Base64Code::BASE64_DECODE_OUT_SIZE(len)];
            Base64Code::base64_decode(str.c_str(), len, reinterpret_cast<unsigned char*>(t));
            str = t;
            delete[] t;
            return *this;
        }
        // Base64->Bp64
        Baseplus64Code& ConvertBase64ToBp() {
            std::string ret;
            int len = str.length();
            if(!len) { str.clear(); return *this; } // 空字符串
            const char *s = str.c_str();
            /* 第1个字符 等号标记 */
            char padFlag = 'R';
            if(s[len-1]=='=') {
                padFlag = 'W';
                if(s[len-2]=='=') padFlag = 'X';
            }
            ret = padFlag;
            /* 每35个Base64字符一组 */
            // 大小写标记 分隔符标记
            int caseFlagIndex = 0, delimFlagIndex = 35; // 初始 A 9
            /* 按组写入大小写标记(1) 位置标记(0-17 AB BC) 分隔符(1) 全大写Base64字符(35) */
            for(int stPos = 0; stPos<len;) {
                // init
                int nxPos = stPos + 35; // 下一组开始位置
                bool isUpperMore; // 是否大写字母更多
                // 第一遍扫大小写数量取小者
                int upperNum = 0, lowerNum = 0;
                for(int i=stPos; i<nxPos; ++i) {
                    char c = s[i];
                    if(isupper(c)) ++upperNum;
                    else if(islower(c)) ++lowerNum;
                    else if(isdigit(c) || c=='+' || c=='/' || c=='=');
                    else if(c=='\0') break; // 结尾
                    else goto FuncFailed;
                }
                isUpperMore = upperNum>lowerNum;
                // 选择大写/小写数量少的记录
                ret.push_back(bp64EnTable()[isUpperMore? caseFlagIndex+1 :caseFlagIndex]);
                // 第二遍扫大小写位置和转换小写为大写
                std::string procStr; // 大写转换过的Base64字符
                for(int i=stPos; i<nxPos; ++i) {
                    char c = s[i];
                    int n;
                    if(c=='\0' || c=='=') break;
                    if(c=='+') c = '_';
                    else if(c=='/') c = '.';
                    procStr.push_back(toupper(c)); // 转换为大写并写入缓存
                    // 根据大小写标记记录大/小写字母位置
                    if((isUpperMore && islower(c)) || (!isUpperMore && isupper(c))) {
                        // 计算除开分隔符的偏移
                        n = i%35; n = n>=delimFlagIndex? n+1: n;
                        ret.push_back(bp64EnTable()[n]);
                    }
                }
                // 写入分隔符和全大写的Base64字符
                ret += bp64EnTable()[delimFlagIndex] + procStr;
                // next
                stPos = nxPos;
                if(++caseFlagIndex>=35) caseFlagIndex = 0;
                if(--delimFlagIndex<0) delimFlagIndex = 35;
            }
            goto EndFunc;

            FuncFailed:;
            ret.clear();

            EndFunc:;
            str = ret;
            return *this;
        } 
        // Bp64->Base64
        Baseplus64Code& ConvertBpToBase64() {
            std::string ret;
            int len = str.length();
            const char *s = str.c_str();
            // 大小写标志 分隔符标志
            int caseFlagIndex = 0, delimFlagIndex = 35;

            /* 第1个标记 等号数量 */
            char pads[3] = {}, c0 = toupper(s[0]);
            if(c0=='R') ;
            else if(c0=='W') pads[0] = '=';
            else if(c0=='X') { pads[0] = '='; pads[1] = '='; }
            else goto FuncFailed;
            // 解析 位置标记 分隔符 35位转换的原始字符
            for(int stPos=1; stPos<len;) {
                /* 大小写标记 */
                bool isUpper;
                int c1 = bp64DeTable()[(unsigned char)s[stPos]];
                if(c1==caseFlagIndex) isUpper = true;
                else if(c1==caseFlagIndex+1) isUpper = false;
                else goto FuncFailed;
                // 寻找分隔符
                int delimPos = -1;
                for(int i=stPos+1; i<len; ++i)
                    if(bp64DeTable()[(unsigned char)s[i]]==delimFlagIndex) {
                        delimPos = i;
                        break;
                    }
                if(delimPos==-1) goto FuncFailed;
                // 复制待转换待小写字符串
                std::string procStr;
                for(int i=delimPos+1, n=i+35; i<n && i<len; ++i) {
                    char c = s[i];
                    // 预处理
                    if(isalnum(c)) procStr.push_back(isUpper? tolower(c): toupper(c));
                    else if(c=='_') procStr.push_back('+');
                    else if(c=='.') procStr.push_back('/');
                    else goto FuncFailed;
                }
                // 还原大小写
                int procStrLen = procStr.size();
                for(int i=stPos+1; i<delimPos; ++i) { // 组第1个为大小写标记
                    int p = bp64DeTable()[(unsigned char)s[i]];
                    if(p<36 && p<procStrLen) {
                        // 还原偏移
                        p = p>delimFlagIndex? p-1: p;
                        char c = procStr[p];
                        procStr[p] = isUpper? toupper(c): tolower(c);
                    }
                    else goto FuncFailed;
                }
                // next
                ret += procStr;
                stPos = delimPos+36;
                if(++caseFlagIndex>=35) caseFlagIndex = 0;
                if(--delimFlagIndex<0) delimFlagIndex = 35;
            }
            ret += pads;
            goto EndFunc;

            FuncFailed:;
            ret.clear();

            EndFunc:;
            str = ret;
            return *this;
        }

        // Bp64维吉尼亚加密
        // TODO 加上预处理toupper()
        /* 方法
            明文、钥匙皆用Bp64编码
            使用变种维吉尼亚密码编码
            #ABCD....012...9
            AZYXWVU..987...0
        */
        Baseplus64Code& EncryptBp(const char *pwd) {
            int textLen = str.size();
            if(textLen==0 || !pwd || pwd[0]=='\0') { str.clear(); return *this; }
            std::string ret;
            Baseplus64Code bpPwd(pwd);
            const char *key = bpPwd.Base64Encode().ConvertBase64ToBp().GetResult().c_str();
            const char *text = str.c_str();
            int keyLen = strlen(key), tbLen = strlen(vigenereTable());

            for(int i=0, j=0; i<textLen; ++i, ++j) {
                if(j>=keyLen) j = 0; // 循环密钥
                int x = -1, y = -1;
                for(int k=0; k<tbLen; ++k) { // 扫字母表
                    char c = vigenereTable()[k];
                    if(x==-1 && c==key[j]) x = k;
                    if(y==-1 && c==text[i]) y = k;
                    if(x!=-1 && y!=-1) break;
                }
                if(x==-1 || y==-1) { str.clear(); return *this; }
                ret.push_back(vigenereTable()[x+y]);
            }

            str = ret;
            return *this;
        }
        // Bp64维吉尼亚解密
        Baseplus64Code& DecryptBp(const char *pwd) {
            int textLen = str.size();
            if(textLen==0 || !pwd || pwd[0]=='\0') { str.clear(); return *this; }
            std::string ret;
            Baseplus64Code bpPwd(pwd);
            const char *key = bpPwd.Base64Encode().ConvertBase64ToBp().GetResult().c_str();
            const char *text = str.c_str();
            int keyLen = strlen(key), tbLen = strlen(vigenereTable());

            for(int i=0, j=0; i<textLen; ++i, ++j) {
                if(j>=keyLen) j = 0; // 循环密钥
                int x = -1, y = -1;
                for(int k=0; k<tbLen; ++k) { // 扫字母表 长度36
                    char c = vigenereTable()[k];
                    if(x==-1 && c==key[j]) x = k;
                    if(x!=-1 && y==-1 && c==text[i]) y = k;
                    if(x!=-1 && y!=-1) break;
                }
                if(x==-1 || y==-1) { str.clear(); return *this; }
                ret.push_back(vigenereTable()[y-x]);
            }

            str = ret;
            return *this;
        }

        static void Test() {
            Baseplus64Code bs("OPenVPn测试");
            printf("%s|\n",bs.Base64Encode().GetResult().c_str());
            printf("%s|\n",bs.ConvertBase64ToBp().GetResult().c_str());
            printf("%s|\n",bs.EncryptBp("lzstupid").GetResult().c_str());
            printf("%s|\n",bs.DecryptBp("lzstupid").GetResult().c_str());
            printf("%s|\n",bs.ConvertBpToBase64().GetResult().c_str());
            printf("%s|\n",bs.Base64Decode().GetResult().c_str());
            puts("--END 1");
            Baseplus64Code bs2("QWERTYUIOPASDFGHJKLZXCVBNM123456789qwertyuiopasdfghjklzxcvbnm123456789");
            printf("%s|\n",bs2.Base64Encode().GetResult().c_str());
            printf("%s|\n",bs2.ConvertBase64ToBp().GetResult().c_str());
            printf("%s|\n",bs2.EncryptBp("lzstupid").GetResult().c_str());
            printf("%s|\n",bs2.DecryptBp("lzstupid").GetResult().c_str());
            printf("%s|\n",bs2.ConvertBpToBase64().GetResult().c_str());
            printf("%s|\n",bs2.Base64Decode().GetResult().c_str());
            puts("--END 2");
        }
    }; // class Baseplus64Code
} // namespace Module

namespace Core {
    // UI类
    class UI_v1c {
        friend bool CmdArgsParser(UI_v1c&, int&, char**&);
        /* 状态集合 */
        static const int IN_BUF_LEN = 512; // 输入缓冲区长度
        struct {
            char InputBuffer[UI_v1c::IN_BUF_LEN]; // 输入缓冲区
            bool StrictMode; // 严格模式
            bool DebugMode; // 调试模式
            bool ColonOrderMode; // 用:代替.进入命令模式
            bool ConsoleMode; // 命令行模式（特殊）
            Module::Split *Args; // 指令分割
        } state;
        // _parseIns() 退出码
        enum {
            PROGRAM_CONTINUE,
            PROGRAM_EXIT
        };
    public:
        // 指令帮助
        void orderHelp() {
            puts(
                "说明：\n指令 参数\n\t说明\n带<>的参数表示必填，带[]的参数表示可不填\n\n"
                "<a> <b> <n>\n.pl <d> <a> <b> <n>\n"
                "\t生成随机数\n\t参数：a:最小值，b最大值为b，n:生成个数，d:输出小数的位数\n"
                "\tn<0时则生成|n|个随机数并累加，a或b为小数时则生成随机浮点数\n"
                "\t说明：开启严格时禁用第一种不带指令的随机数生成方式\n"
                ".m<mode>\n\t设置环境模式\n\t参数：mode可为以下值:\n"
                "\t\ts\t开启/关闭严格模式\n\t\tc\t使用/取消:代替.进入命令模式\n\t\td\t开启/关闭调试模式\n"
                ".sd [seed]\n\t设置/显示随机数生成器种子\n\t参数：seed:新的种子值(uint类型)\n\t说明：省略seed显示当前种子值\n"
                ".r <code>\n\t【未启用】执行计算脚本\n"
                ".h[extension]\n\t显示帮助\n\t说明：省略extension则显示命令帮助\n"
                "\textension可为以下值：\n"
                "\t\tc\t【未启用】显示命令行帮助\n\t\tt\t显示可用的内置工具\n\t\ts\t【未启用】显示计算脚本语言语法\n"
                ".tl <tool> [args]\n\t运行内置工具\n\t参数：tool:工具名，args工具参数\n\t说明：可用内置工具列表见.ht命令\n"
                "#[string]\n\t注释\n\t参数：string:注释文字\n.q\n\t退出\n"
            );
        }
        // 工具帮助
        void toolHelp() {
            puts(
                "内置工具列表：\n\nosuc <玩家1分数>,<玩家1准确率>[,<玩家2分数>,<玩家2准确率>...]\n"
                "\t\t自制osu!随机数加权分数计算器\n"
                ".b64x <flag>,<string>,[password]\n\tBase64编码/解码\n\t参数：flag:顺序执行的命令。支持的命令：\n"
                "\t\ta\tBase64编码\n\t\tb\tBase64解码\n"
                "\t\tc\tBase64转Baseplus64\n\t\td\tBaseplus64转Base64\n"
                "\t\te\t对Baseplus64编码使用变种维吉尼亚密码加密\n\t\tf\t对Baseplus64编码使用变种维吉尼亚密码解码\n"
                "\tstring\t要编码的字符串，或要解码的字符串，要用双引号括起\n"
                "\tpassword\t执行维吉尼亚密码加解密所需的密钥，要用双引号括起\n"

            );
        }
        // 命令行帮助
        void CLIHelp() {
            puts(
                "[-d|-?|-h] \"ex[ress\"\n"
                "参数:\n\t-d\t开启调试模式\n"
                "\t-?,-h\t显示帮助\n"
                "\texpress\t执行表达式\n"
            );
        }
        // 脚本语言帮助
        void scriptHelp() {}
        // 快速随机数生成
        // 参数：d精度限制 num0第一个数字的字符串
        void quickRndGen(int d, char *num0) {
            using namespace Module;
            Split *argSet = state.Args;
            // 原始参数
            char *s[3] = {num0, argSet->SplitByChar(' ', false), argSet->SplitByChar(' ', false)};
            // 是否为浮点数
            bool isfloat = false;
            if((s[0] && strchr(s[0], '.')) || (s[1] && strchr(s[1], '.'))) isfloat = true;
            // 类型转换 默认值填充 保证a<b
            int n = s[2]? atoi(s[2]): 1;
            if(n==0) return; // 无输出直接返回
            union {int i; double d;} a, b;
            int ti; double td; // 临时变量
            if(isfloat) {
                a.d = s[0]? atof(s[0]): 1.0, b.d = s[1]? atof(s[1]): 100.0;
                if(a.d>b.d) td = a.d, a.d = b.d, b.d = td; // swap
            } else {
                a.i = s[0]? atoi(s[0]): 1, b.i = s[1]? atoi(s[1]): 100;
                if(a.i>b.i) ti = a.i, a.i = b.i, b.i = ti; // swap
            }
            // 精度限制 合成printf参数
            char pfarg[8]; 
            if(d<0 || d>15) strcpy(pfarg, "%f%c");
            else sprintf(pfarg,"%%.%df%%c", d);

            if(state.DebugMode)printf("[D]: %s| %s| %s| %s %f %f %d %d %s|\n",s[0],s[1],s[2],isfloat?"Float":"Int",isfloat?a.d:(double)a.i,isfloat?b.d:(double)b.i,n,d,pfarg);

            if(isfloat)
                if(n<0) printf(pfarg, Dice.frs(a.d, b.d, -n), '\n');
                else for(int i=0, z=n>0? n: -n; i<z; ++i) printf(pfarg, Dice.fr(a.d, b.d), i==z-1? '\n': ' ');
            else
                if(n<0) printf("%d\n", Dice.irs(a.i, b.i, -n));
                else for(int i=0, z=n>0? n: -n; i<z; ++i) printf("%d%c", Dice.ir(a.i, b.i), i==z-1? '\n': ' ');
        }
        // 计算脚本解释
        void eval() {}
        /* 内置模块 */
        // osu随机数加权分数计算
        void osucModule() {
            // 公式： Score = osu_score * rnd(avg(ACCs), MAX_RND_RATIO)
            // .tl osuc 1122223333,0.9743,1122333344,0.9801,1122334444,0.9856
            // .tl osuc 2434779,0.9409,2538793,0.9672,2657600,1
            using namespace Module;
            const static double MAX_RND_RATIO = 1.05; // 随机数最大值

            std::vector<double> arg;
            Split *argSet = state.Args;
            char *s;
            while(s = argSet->SplitByChar(',', false)) // 载入参数 s!=NULL则push
                arg.push_back(atof(s));
            int n = arg.size();
            if(n==0 || n%2==1) throw OrderSyntaxException(); // 不为偶数个参数则报错
            printf("--当前公式：Score = osu_score * rnd(avg(ACCs), %.2f)\n", MAX_RND_RATIO);
            // 求平均ACC
            double avg_acc = 0;
            for(int i=1; i<n; i+=2) avg_acc += arg[i];
            avg_acc /= n/2.0;
            printf("--ACC平均值：%f\n", avg_acc);
            // 计算结果
            for(int i=0; i<n; i+=2) {
                double origin = arg[i], rnd = Dice.fr(avg_acc, MAX_RND_RATIO), score = origin*rnd;
                printf("第%d位玩家的加权分数：%f\t(原始分：%d, ACC：%f, 权数：%f)\n", i/2+1, score, (int)origin, arg[i+1], rnd);
            }
        }
        // Base64/Baseplus64加解密（接口）
        // TEST .tl b64x acefdb,"QWERTYUIOPASDFGHJKLZXCVBNM123456789qwertyuiopasdfghjklzxcvbnm123456789","lzstupid"
        void base64xModule() {
            using namespace Module;
            Split *argSet = state.Args;
            char *flag = argSet->SplitByChar(',', false);
            if(!flag) throw OrderSyntaxException();
            // 获取str和去双引号
            char *str = argSet->SplitByChar(',', true);
            Trim(str);
            int sLen = strlen(str);
            if(str[0]=='\"' && str[sLen-1]=='\"') {
                str[sLen-1] = '\0';
                str = &str[1];
            }
            // 尝试获取pwd和去双引号
            char *pwd = argSet->SplitByChar(',', true);
            int pLen = strlen(pwd);
            Trim(pwd);
            if(pwd[0]=='\"' && pwd[pLen-1]=='\"') {
                pwd[pLen-1] = '\0';
                pwd = &pwd[1];
            }
            // 执行flag
            Baseplus64Code basex(str);
            for(int i=0,n=strlen(flag); i<n; ++i) {
                if(flag[i]=='a') basex.Base64Encode();
                else if(flag[i]=='b') basex.Base64Decode();
                else if(flag[i]=='c') basex.ConvertBase64ToBp();
                else if(flag[i]=='d') basex.ConvertBpToBase64();
                else if(flag[i]=='e') {
                    if(!pwd) throw OrderSyntaxException();
                    basex.EncryptBp(pwd);
                }
                else if(flag[i]=='f') {
                    if(!pwd) throw OrderSyntaxException();
                    basex.DecryptBp(pwd);
                }
                else if(isspace(flag[i]));
                else throw OrderSyntaxException();
            }
            // 输出
            puts(basex.GetResult().c_str());
        }
    public:
        UI_v1c() {
            // 初始化
            state.StrictMode = false;
            state.DebugMode = false;
            state.ColonOrderMode = false;
            state.ConsoleMode = false;
            state.Args = NULL;
        }
        // 启动
        void Start() {
            using namespace Module;
            if(!state.ConsoleMode)
                puts("RandRange " RNDRG_VERSION "\n输入.h查看帮助\n程序支持命令行参数启动\n");
            while(true) {
                if(!state.ConsoleMode) {
                    printf("> ");
                    state.InputBuffer[0] = '\0';
                    // 输入
                    fgets(state.InputBuffer, IN_BUF_LEN, stdin);
                }
                Trim(state.InputBuffer);
                state.Args = new Split(state.InputBuffer);
                try {
                    switch(parseIns()) {
                        case PROGRAM_EXIT:
                            delete state.Args; // 销毁Args
                            return; // 退出
                    }
                } catch(ErrorCodeException &e) {
                    puts(e.what());
                }
                delete state.Args; // 销毁Args
            }
        }
        // 指令解析
        int parseIns() {
            using namespace Module;
            Split *argSet = state.Args;
            char *arg = argSet->SplitByChar(' ', false);
            if(arg) { // if not then continue
                if(arg[0]=='#') ; // continue
                else if((!state.ColonOrderMode && arg[0]=='.' && !isdigit(arg[1])) || (state.ColonOrderMode && arg[0]==':'))
                {
                    /* 命令模式 */
                    // 退出
                    if(arg[1]=='q') return PROGRAM_EXIT;
                    // 帮助
                    else if(arg[1]=='h') {
                        if(arg[2]=='\0') orderHelp();
                        else if(arg[2]=='c' && arg[3]=='\0') CLIHelp();
                        else if(arg[2]=='t' && arg[3]=='\0') toolHelp();
                        else if(arg[2]=='s' && arg[3]=='\0') scriptHelp();
                        else throw InvalidOrderException();
                    }
                    // 脚本解释
                    else if(arg[1]=='r' && arg[2]=='\0') eval();
                    // 随机数生成(输出精度限制)
                    else if(arg[1]=='p' && arg[2]=='l' && arg[3]=='\0') {
                        char *d = state.Args->SplitByChar(' ', false);
                        if(!d) throw OrderSyntaxException();
                        quickRndGen(atoi(d), argSet->SplitByChar(' ', false));
                    }
                    // 设置环境模式
                    else if(arg[1]=='m')
                        for(int i=2; arg[i]; ++i) {
                            char c = arg[i];
                            switch(c) {
                                case 's':
                                    state.StrictMode = !state.StrictMode;
                                    printf("%s严格模式\n", state.StrictMode? "开启": "关闭");
                                    break;
                                case 'c':
                                    state.ColonOrderMode = !state.ColonOrderMode;
                                    printf("%s冒号命令模式\n", state.ColonOrderMode? "开启": "关闭");
                                    break;
                                case 'd':
                                    state.DebugMode = !state.DebugMode;
                                    printf("%s调试模式\n", state.DebugMode? "开启": "关闭");
                                    break;
                                default:
                                    if(!isspace(c)) throw InvalidOrderException();
                            }
                        }
                    // 获取/设置随机数种子
                    else if(arg[1]=='s' && arg[2]=='d' && arg[3]=='\0') {
                        char *s = argSet->SplitByChar(' ', false);
                        unsigned int sd;
                        if(s) {
                            sscanf(s, "%u", &sd);
                            Dice.Seed(sd);
                        }
                        printf("当前种子值：%u\n", Dice.Seed());
                    }
                    // 内置工具
                    else if(arg[1]=='t' && arg[2]=='l' && arg[3]=='\0') {
                        char *tool = argSet->SplitByChar(' ', false);
                        if(!strcmp(tool, "osuc"))
                            osucModule();
                        else if(!strcmp(tool, "b64x"))
                            base64xModule();
                        else
                            throw InvalidOrderException();
                    }
                    else
                        throw InvalidOrderException();
                }
                else if(!state.StrictMode && (isdigit(arg[0]) || (state.ColonOrderMode && arg[0]=='.' && isdigit(arg[1]))))
                    quickRndGen(-1, arg);
                else
                    throw InvalidOrderException();
            }

            return state.ConsoleMode? PROGRAM_EXIT: PROGRAM_CONTINUE;
        }

    }; // class UI_v1c

    // 命令行参数解析
    // 返回值：true 启动程序 false 解释完参数即退出
    // BUG
    /*bool CmdArgsParser(UI_v1c &UI, int &argc, char **&argv) {
        for(int i=1; i<argc; ++i) {
            char *arg = argv[i];
            if(arg[0]=='-')
                if(arg[1]=='d') // 调试模式
                    UI.state.DebugMode = true;
                if(arg[1]=='h' || arg[1]=='?') {
                    UI.CLIHelp();
                    goto EndFunc;
                }
            else {
                // 参数为表达式，或指令参数【未启用】
                UI.state.ConsoleMode = true;
                strcpy(UI.state.InputBuffer, arg);
            }
        }
        EndFunc:;
        return argc>1? false: true;
    }*/
} // namespace Core

int main(int argc, char **argv) {
    Core::UI_v1c UI;
    //if(Core::CmdArgsParser(UI, argc, argv))
    UI.Start();
    return 0;
}

/*
1.6版进一步模块化、接口化
1.7版新增内容
.r 表达式计算

四则运算(+ - * / % **（幂） //（整除）)
字符串输出
    ..字符串连接
    \ 转义
变量读取 [1]
变量存储 :[1]
    支持变量自加自减等 :+ :- ...
    [1]非负整数全局变量
    未启用：[-1]栈
        未启用：pop push / popt(尾) poph（头）...
    未启用：[1i]强制转换为int，[1f]强制转换为double
    未启用：nil无效值
    已弃用：@:表示不输出结果存储
nD[a~]b[~d]随机数表达式
#注释
    未启用：@# 结果后输出注释
    未启用：#* *# 块注释
;语句连接符
 _ 续行符
未启用：if 条件{}elif 条件{}...else{}
未启用：
    1) select 表达式 case [is 运算符] 表达式{}...[default{}] # case default至少有一
    2) select 表达式 {case [is 运算符] 表达式[;] ...[;] default[;] ... }
未启用：while 条件{}
未启用：for 前置;条件;后续{}
未启用：forc 变量(起始,结束,[步幅]),...{} #for next / for count
未启用：iif(条件,true返回,false返回)
未启用：sub/fnc 函数名
未启用：label:[;] goto
未启用：逻辑运算符：not and or > < <= >= <>（不等）
未启用：位运算符：<< >> !（按位取反） & | ^（按位异或）
未启用：break continue（在select中起穿透作用） return
未启用：@ 向解释器发送指令
未启用：@@ 当前语句不输出
未启用：@varn [preserve] size 预分配全部变量最大数量 #默认256
*/

/*
sub main(){
    999:[1]
    1+1d(1d5)~10:[2]
    "string"
    "val of [1]:"..[1] #输出var of [1]:999
    add([1],1):[3]
    if [3]==1000 {
        0:[4]
        for [4]<3{
            [3] #输出3次1000
        }
        0:[4]
        for{
            if [4]==3{break}
            [3]
            1:+[4] # 自加1
        }# 效果同上
        for [5]::(1,3,1){ # for [5]=1 to 3 step 1
            iif(1,[3],"false") # ?:三目运算符
        }# 效果同上
    }
}
fnc add(){
    return [-1]+[-2]
}
*/

/*
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, char tok) {
    int cb = 0, cq = 0;
    std::string t;
    std::vector<std::string> res;
    for (char c : s) {
        if (c == tok && !cb && !cq) {
            if (!t.empty()) {
                res.push_back(t);
                t.clear();
            }
        } else if (c == '\"')
            cq ^= 1;
        else {
            t.push_back(c);
            if (c == '(') ++cq;
            else if (c == ')') --cq;
        }
    }
    if (!t.empty()) res.push_back(t);
    return res;
}

int main() {
    for (auto &s : split("qwq qwq qaq (qwq qwq) \"qaq qaq \" qwq ", ' '))
        std::cout << '"' << s << '"' << std::endl;
}
*/