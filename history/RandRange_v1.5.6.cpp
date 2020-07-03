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
// �������
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
        // ��������
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

        // ���������[0,0xFFFFFFFF]
        inline uint64_t RI() {
            return x256ss.next();
        }
        // �����������[0,1)
        inline double RF() {
            const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | RI() >> 12 };
            return u.d - 1.0;
        }

        // ���ط�ΧΪ[a,b]���������
        int ir(int a, int b) {
            return RI() % (b-a+1) + a;
        }
        // ����n����ΧΪ[a,b]���������֮��
        int irs(int a, int b, int n) {
            int s = 0;
            for(int i=0; i<n; ++i) s += ir(a, b);
            return s;
        }
        // ���ط�ΧΪ[a,b)�����������
        double fr(double a, double b) {	
            return RF() * (b-a) + a;
        }
        // ����n����ΧΪ[a,b)�����������֮��
        double frs(double a, double b, int n) {
            double s = 0;
            for(int i=0; i<n; ++i) s += fr(a, b);
            return s;
        }

        //�������ơ�a���֣�dС��λ��,d<0������
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
    static inline void ClearStdin() { // ������뻺����
        scanf("%*[^'\n']%*c");
    }
    void Trim(char *s) { // ȥ��β�հ��ַ�����������Ϊ�ַ�����
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
        // �и��ַ���
        // ����ֵ��ָ���¸��Ǻ���ʼ��ָ��
        // delim �ָ���
        // ignoreIncludedDelim �Ƿ���԰������Ż������ڵķָ���
        char *SplitByChar(char delim, bool ignoreIncludedDelim) {
            // �Ƿ�Ϊ���ַ����򵽽�β
            if(!curPos || !curPos[0]) return NULL;
            int balance[5]={}; // " ' ( [ {
            char c;
            // p1 curPos�ӷָ���ɨ����ͨ�ַ�
            for(; *curPos && *curPos==delim; ++curPos) *curPos = '\0';
            // p2 nextPos����ͨ�ַ�ɨ���ָ���
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

    // Base64�ӽ���
    // ����Դ��https://github.com/zhicheng/base64
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

    // Baseplus64�ӽ���
    // �������룺������Base64�ӽ��ܺ���
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
        // TODO ȫ�ֿ��ַ������
        // sΪ��Ҫ����ļ�/�����ַ���
        Baseplus64Code(const char *s) : str(s) {}
        // ��ȡ���
        std::string GetResult() { return str; }

        // �ı�->Base64
        Baseplus64Code& Base64Encode() {
            char *t; int len = str.length();
            t = new char[Base64Code::BASE64_ENCODE_OUT_SIZE(len)];
            Base64Code::base64_encode(reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str())), len, t);
            str = t;
            delete[] t;
            return *this;
        }
        // Base64->�ı�
        // ��ʽ���Ⱥ�������ʶ��(1λ��R0/W1/X2)+[���С/��д��ĸλ��ģʽ(1λ,AB,CD,EF...YZѭ��)+С/��д��ĸλ��(0-18λ����ȥ��Ϊ�ָ�������ĸ)+�ָ���(1λ,Z,Y,X...Aѭ��)+ȫС/��д��Base64����Ƭ��(35λ)]*n
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
            if(!len) { str.clear(); return *this; } // ���ַ���
            const char *s = str.c_str();
            /* ��1���ַ� �Ⱥű�� */
            char padFlag = 'R';
            if(s[len-1]=='=') {
                padFlag = 'W';
                if(s[len-2]=='=') padFlag = 'X';
            }
            ret = padFlag;
            /* ÿ35��Base64�ַ�һ�� */
            // ��Сд��� �ָ������
            int caseFlagIndex = 0, delimFlagIndex = 35; // ��ʼ A 9
            /* ����д���Сд���(1) λ�ñ��(0-17 AB BC) �ָ���(1) ȫ��дBase64�ַ�(35) */
            for(int stPos = 0; stPos<len;) {
                // init
                int nxPos = stPos + 35; // ��һ�鿪ʼλ��
                bool isUpperMore; // �Ƿ��д��ĸ����
                // ��һ��ɨ��Сд����ȡС��
                int upperNum = 0, lowerNum = 0;
                for(int i=stPos; i<nxPos; ++i) {
                    char c = s[i];
                    if(isupper(c)) ++upperNum;
                    else if(islower(c)) ++lowerNum;
                    else if(isdigit(c) || c=='+' || c=='/' || c=='=');
                    else if(c=='\0') break; // ��β
                    else goto FuncFailed;
                }
                isUpperMore = upperNum>lowerNum;
                // ѡ���д/Сд�����ٵļ�¼
                ret.push_back(bp64EnTable()[isUpperMore? caseFlagIndex+1 :caseFlagIndex]);
                // �ڶ���ɨ��Сдλ�ú�ת��СдΪ��д
                std::string procStr; // ��дת������Base64�ַ�
                for(int i=stPos; i<nxPos; ++i) {
                    char c = s[i];
                    int n;
                    if(c=='\0' || c=='=') break;
                    if(c=='+') c = '_';
                    else if(c=='/') c = '.';
                    procStr.push_back(toupper(c)); // ת��Ϊ��д��д�뻺��
                    // ���ݴ�Сд��Ǽ�¼��/Сд��ĸλ��
                    if((isUpperMore && islower(c)) || (!isUpperMore && isupper(c))) {
                        // ��������ָ�����ƫ��
                        n = i%35; n = n>=delimFlagIndex? n+1: n;
                        ret.push_back(bp64EnTable()[n]);
                    }
                }
                // д��ָ�����ȫ��д��Base64�ַ�
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
            // ��Сд��־ �ָ�����־
            int caseFlagIndex = 0, delimFlagIndex = 35;

            /* ��1����� �Ⱥ����� */
            char pads[3] = {}, c0 = toupper(s[0]);
            if(c0=='R') ;
            else if(c0=='W') pads[0] = '=';
            else if(c0=='X') { pads[0] = '='; pads[1] = '='; }
            else goto FuncFailed;
            // ���� λ�ñ�� �ָ��� 35λת����ԭʼ�ַ�
            for(int stPos=1; stPos<len;) {
                /* ��Сд��� */
                bool isUpper;
                int c1 = bp64DeTable()[(unsigned char)s[stPos]];
                if(c1==caseFlagIndex) isUpper = true;
                else if(c1==caseFlagIndex+1) isUpper = false;
                else goto FuncFailed;
                // Ѱ�ҷָ���
                int delimPos = -1;
                for(int i=stPos+1; i<len; ++i)
                    if(bp64DeTable()[(unsigned char)s[i]]==delimFlagIndex) {
                        delimPos = i;
                        break;
                    }
                if(delimPos==-1) goto FuncFailed;
                // ���ƴ�ת����Сд�ַ���
                std::string procStr;
                for(int i=delimPos+1, n=i+35; i<n && i<len; ++i) {
                    char c = s[i];
                    // Ԥ����
                    if(isalnum(c)) procStr.push_back(isUpper? tolower(c): toupper(c));
                    else if(c=='_') procStr.push_back('+');
                    else if(c=='.') procStr.push_back('/');
                    else goto FuncFailed;
                }
                // ��ԭ��Сд
                int procStrLen = procStr.size();
                for(int i=stPos+1; i<delimPos; ++i) { // ���1��Ϊ��Сд���
                    int p = bp64DeTable()[(unsigned char)s[i]];
                    if(p<36 && p<procStrLen) {
                        // ��ԭƫ��
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

        // Bp64ά�����Ǽ���
        // TODO ����Ԥ����toupper()
        /* ����
            ���ġ�Կ�׽���Bp64����
            ʹ�ñ���ά�������������
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
                if(j>=keyLen) j = 0; // ѭ����Կ
                int x = -1, y = -1;
                for(int k=0; k<tbLen; ++k) { // ɨ��ĸ��
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
        // Bp64ά�����ǽ���
        Baseplus64Code& DecryptBp(const char *pwd) {
            int textLen = str.size();
            if(textLen==0 || !pwd || pwd[0]=='\0') { str.clear(); return *this; }
            std::string ret;
            Baseplus64Code bpPwd(pwd);
            const char *key = bpPwd.Base64Encode().ConvertBase64ToBp().GetResult().c_str();
            const char *text = str.c_str();
            int keyLen = strlen(key), tbLen = strlen(vigenereTable());

            for(int i=0, j=0; i<textLen; ++i, ++j) {
                if(j>=keyLen) j = 0; // ѭ����Կ
                int x = -1, y = -1;
                for(int k=0; k<tbLen; ++k) { // ɨ��ĸ�� ����36
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
            Baseplus64Code bs("OPenVPn����");
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
    // UI��
    class UI_v1c {
        friend bool CmdArgsParser(UI_v1c&, int&, char**&);
        /* ״̬���� */
        static const int IN_BUF_LEN = 512; // ���뻺��������
        struct {
            char InputBuffer[UI_v1c::IN_BUF_LEN]; // ���뻺����
            bool StrictMode; // �ϸ�ģʽ
            bool DebugMode; // ����ģʽ
            bool ColonOrderMode; // ��:����.��������ģʽ
            bool ConsoleMode; // ������ģʽ�����⣩
            Module::Split *Args; // ָ��ָ�
        } state;
        // _parseIns() �˳���
        enum {
            PROGRAM_CONTINUE,
            PROGRAM_EXIT
        };
    public:
        // ָ�����
        void orderHelp() {
            puts(
                "˵����\nָ�� ����\n\t˵��\n��<>�Ĳ�����ʾ�����[]�Ĳ�����ʾ�ɲ���\n\n"
                "<a> <b> <n>\n.pl <d> <a> <b> <n>\n"
                "\t���������\n\t������a:��Сֵ��b���ֵΪb��n:���ɸ�����d:���С����λ��\n"
                "\tn<0ʱ������|n|����������ۼӣ�a��bΪС��ʱ���������������\n"
                "\t˵���������ϸ�ʱ���õ�һ�ֲ���ָ�����������ɷ�ʽ\n"
                ".m<mode>\n\t���û���ģʽ\n\t������mode��Ϊ����ֵ:\n"
                "\t\ts\t����/�ر��ϸ�ģʽ\n\t\tc\tʹ��/ȡ��:����.��������ģʽ\n\t\td\t����/�رյ���ģʽ\n"
                ".sd [seed]\n\t����/��ʾ���������������\n\t������seed:�µ�����ֵ(uint����)\n\t˵����ʡ��seed��ʾ��ǰ����ֵ\n"
                ".r <code>\n\t��δ���á�ִ�м���ű�\n"
                ".h[extension]\n\t��ʾ����\n\t˵����ʡ��extension����ʾ�������\n"
                "\textension��Ϊ����ֵ��\n"
                "\t\tc\t��δ���á���ʾ�����а���\n\t\tt\t��ʾ���õ����ù���\n\t\ts\t��δ���á���ʾ����ű������﷨\n"
                ".tl <tool> [args]\n\t�������ù���\n\t������tool:��������args���߲���\n\t˵�����������ù����б��.ht����\n"
                "#[string]\n\tע��\n\t������string:ע������\n.q\n\t�˳�\n"
            );
        }
        // ���߰���
        void toolHelp() {
            puts(
                "���ù����б�\n\nosuc <���1����>,<���1׼ȷ��>[,<���2����>,<���2׼ȷ��>...]\n"
                "\t\t����osu!�������Ȩ����������\n"
                ".b64x <flag>,<string>,[password]\n\tBase64����/����\n\t������flag:˳��ִ�е����֧�ֵ����\n"
                "\t\ta\tBase64����\n\t\tb\tBase64����\n"
                "\t\tc\tBase64תBaseplus64\n\t\td\tBaseplus64תBase64\n"
                "\t\te\t��Baseplus64����ʹ�ñ���ά�������������\n\t\tf\t��Baseplus64����ʹ�ñ���ά�������������\n"
                "\tstring\tҪ������ַ�������Ҫ������ַ�����Ҫ��˫��������\n"
                "\tpassword\tִ��ά����������ӽ����������Կ��Ҫ��˫��������\n"

            );
        }
        // �����а���
        void CLIHelp() {
            puts(
                "[-d|-?|-h] \"ex[ress\"\n"
                "����:\n\t-d\t��������ģʽ\n"
                "\t-?,-h\t��ʾ����\n"
                "\texpress\tִ�б��ʽ\n"
            );
        }
        // �ű����԰���
        void scriptHelp() {}
        // �������������
        // ������d�������� num0��һ�����ֵ��ַ���
        void quickRndGen(int d, char *num0) {
            using namespace Module;
            Split *argSet = state.Args;
            // ԭʼ����
            char *s[3] = {num0, argSet->SplitByChar(' ', false), argSet->SplitByChar(' ', false)};
            // �Ƿ�Ϊ������
            bool isfloat = false;
            if((s[0] && strchr(s[0], '.')) || (s[1] && strchr(s[1], '.'))) isfloat = true;
            // ����ת�� Ĭ��ֵ��� ��֤a<b
            int n = s[2]? atoi(s[2]): 1;
            if(n==0) return; // �����ֱ�ӷ���
            union {int i; double d;} a, b;
            int ti; double td; // ��ʱ����
            if(isfloat) {
                a.d = s[0]? atof(s[0]): 1.0, b.d = s[1]? atof(s[1]): 100.0;
                if(a.d>b.d) td = a.d, a.d = b.d, b.d = td; // swap
            } else {
                a.i = s[0]? atoi(s[0]): 1, b.i = s[1]? atoi(s[1]): 100;
                if(a.i>b.i) ti = a.i, a.i = b.i, b.i = ti; // swap
            }
            // �������� �ϳ�printf����
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
        // ����ű�����
        void eval() {}
        /* ����ģ�� */
        // osu�������Ȩ��������
        void osucModule() {
            // ��ʽ�� Score = osu_score * rnd(avg(ACCs), MAX_RND_RATIO)
            // .tl osuc 1122223333,0.9743,1122333344,0.9801,1122334444,0.9856
            // .tl osuc 2434779,0.9409,2538793,0.9672,2657600,1
            using namespace Module;
            const static double MAX_RND_RATIO = 1.05; // ��������ֵ

            std::vector<double> arg;
            Split *argSet = state.Args;
            char *s;
            while(s = argSet->SplitByChar(',', false)) // ������� s!=NULL��push
                arg.push_back(atof(s));
            int n = arg.size();
            if(n==0 || n%2==1) throw OrderSyntaxException(); // ��Ϊż���������򱨴�
            printf("--��ǰ��ʽ��Score = osu_score * rnd(avg(ACCs), %.2f)\n", MAX_RND_RATIO);
            // ��ƽ��ACC
            double avg_acc = 0;
            for(int i=1; i<n; i+=2) avg_acc += arg[i];
            avg_acc /= n/2.0;
            printf("--ACCƽ��ֵ��%f\n", avg_acc);
            // ������
            for(int i=0; i<n; i+=2) {
                double origin = arg[i], rnd = Dice.fr(avg_acc, MAX_RND_RATIO), score = origin*rnd;
                printf("��%dλ��ҵļ�Ȩ������%f\t(ԭʼ�֣�%d, ACC��%f, Ȩ����%f)\n", i/2+1, score, (int)origin, arg[i+1], rnd);
            }
        }
        // Base64/Baseplus64�ӽ��ܣ��ӿڣ�
        // TEST .tl b64x acefdb,"QWERTYUIOPASDFGHJKLZXCVBNM123456789qwertyuiopasdfghjklzxcvbnm123456789","lzstupid"
        void base64xModule() {
            using namespace Module;
            Split *argSet = state.Args;
            char *flag = argSet->SplitByChar(',', false);
            if(!flag) throw OrderSyntaxException();
            // ��ȡstr��ȥ˫����
            char *str = argSet->SplitByChar(',', true);
            Trim(str);
            int sLen = strlen(str);
            if(str[0]=='\"' && str[sLen-1]=='\"') {
                str[sLen-1] = '\0';
                str = &str[1];
            }
            // ���Ի�ȡpwd��ȥ˫����
            char *pwd = argSet->SplitByChar(',', true);
            int pLen = strlen(pwd);
            Trim(pwd);
            if(pwd[0]=='\"' && pwd[pLen-1]=='\"') {
                pwd[pLen-1] = '\0';
                pwd = &pwd[1];
            }
            // ִ��flag
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
            // ���
            puts(basex.GetResult().c_str());
        }
    public:
        UI_v1c() {
            // ��ʼ��
            state.StrictMode = false;
            state.DebugMode = false;
            state.ColonOrderMode = false;
            state.ConsoleMode = false;
            state.Args = NULL;
        }
        // ����
        void Start() {
            using namespace Module;
            if(!state.ConsoleMode)
                puts("RandRange " RNDRG_VERSION "\n����.h�鿴����\n����֧�������в�������\n");
            while(true) {
                if(!state.ConsoleMode) {
                    printf("> ");
                    state.InputBuffer[0] = '\0';
                    // ����
                    fgets(state.InputBuffer, IN_BUF_LEN, stdin);
                }
                Trim(state.InputBuffer);
                state.Args = new Split(state.InputBuffer);
                try {
                    switch(parseIns()) {
                        case PROGRAM_EXIT:
                            delete state.Args; // ����Args
                            return; // �˳�
                    }
                } catch(ErrorCodeException &e) {
                    puts(e.what());
                }
                delete state.Args; // ����Args
            }
        }
        // ָ�����
        int parseIns() {
            using namespace Module;
            Split *argSet = state.Args;
            char *arg = argSet->SplitByChar(' ', false);
            if(arg) { // if not then continue
                if(arg[0]=='#') ; // continue
                else if((!state.ColonOrderMode && arg[0]=='.' && !isdigit(arg[1])) || (state.ColonOrderMode && arg[0]==':'))
                {
                    /* ����ģʽ */
                    // �˳�
                    if(arg[1]=='q') return PROGRAM_EXIT;
                    // ����
                    else if(arg[1]=='h') {
                        if(arg[2]=='\0') orderHelp();
                        else if(arg[2]=='c' && arg[3]=='\0') CLIHelp();
                        else if(arg[2]=='t' && arg[3]=='\0') toolHelp();
                        else if(arg[2]=='s' && arg[3]=='\0') scriptHelp();
                        else throw InvalidOrderException();
                    }
                    // �ű�����
                    else if(arg[1]=='r' && arg[2]=='\0') eval();
                    // ���������(�����������)
                    else if(arg[1]=='p' && arg[2]=='l' && arg[3]=='\0') {
                        char *d = state.Args->SplitByChar(' ', false);
                        if(!d) throw OrderSyntaxException();
                        quickRndGen(atoi(d), argSet->SplitByChar(' ', false));
                    }
                    // ���û���ģʽ
                    else if(arg[1]=='m')
                        for(int i=2; arg[i]; ++i) {
                            char c = arg[i];
                            switch(c) {
                                case 's':
                                    state.StrictMode = !state.StrictMode;
                                    printf("%s�ϸ�ģʽ\n", state.StrictMode? "����": "�ر�");
                                    break;
                                case 'c':
                                    state.ColonOrderMode = !state.ColonOrderMode;
                                    printf("%sð������ģʽ\n", state.ColonOrderMode? "����": "�ر�");
                                    break;
                                case 'd':
                                    state.DebugMode = !state.DebugMode;
                                    printf("%s����ģʽ\n", state.DebugMode? "����": "�ر�");
                                    break;
                                default:
                                    if(!isspace(c)) throw InvalidOrderException();
                            }
                        }
                    // ��ȡ/�������������
                    else if(arg[1]=='s' && arg[2]=='d' && arg[3]=='\0') {
                        char *s = argSet->SplitByChar(' ', false);
                        unsigned int sd;
                        if(s) {
                            sscanf(s, "%u", &sd);
                            Dice.Seed(sd);
                        }
                        printf("��ǰ����ֵ��%u\n", Dice.Seed());
                    }
                    // ���ù���
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

    // �����в�������
    // ����ֵ��true �������� false ������������˳�
    // BUG
    /*bool CmdArgsParser(UI_v1c &UI, int &argc, char **&argv) {
        for(int i=1; i<argc; ++i) {
            char *arg = argv[i];
            if(arg[0]=='-')
                if(arg[1]=='d') // ����ģʽ
                    UI.state.DebugMode = true;
                if(arg[1]=='h' || arg[1]=='?') {
                    UI.CLIHelp();
                    goto EndFunc;
                }
            else {
                // ����Ϊ���ʽ����ָ�������δ���á�
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
1.6���һ��ģ�黯���ӿڻ�
1.7����������
.r ���ʽ����

��������(+ - * / % **���ݣ� //��������)
�ַ������
    ..�ַ�������
    \ ת��
������ȡ [1]
�����洢 :[1]
    ֧�ֱ����Լ��Լ��� :+ :- ...
    [1]�Ǹ�����ȫ�ֱ���
    δ���ã�[-1]ջ
        δ���ã�pop push / popt(β) poph��ͷ��...
    δ���ã�[1i]ǿ��ת��Ϊint��[1f]ǿ��ת��Ϊdouble
    δ���ã�nil��Чֵ
    �����ã�@:��ʾ���������洢
nD[a~]b[~d]��������ʽ
#ע��
    δ���ã�@# ��������ע��
    δ���ã�#* *# ��ע��
;������ӷ�
 _ ���з�
δ���ã�if ����{}elif ����{}...else{}
δ���ã�
    1) select ���ʽ case [is �����] ���ʽ{}...[default{}] # case default������һ
    2) select ���ʽ {case [is �����] ���ʽ[;] ...[;] default[;] ... }
δ���ã�while ����{}
δ���ã�for ǰ��;����;����{}
δ���ã�forc ����(��ʼ,����,[����]),...{} #for next / for count
δ���ã�iif(����,true����,false����)
δ���ã�sub/fnc ������
δ���ã�label:[;] goto
δ���ã��߼��������not and or > < <= >= <>�����ȣ�
δ���ã�λ�������<< >> !����λȡ���� & | ^����λ���
δ���ã�break continue����select����͸���ã� return
δ���ã�@ �����������ָ��
δ���ã�@@ ��ǰ��䲻���
δ���ã�@varn [preserve] size Ԥ����ȫ������������� #Ĭ��256
*/

/*
sub main(){
    999:[1]
    1+1d(1d5)~10:[2]
    "string"
    "val of [1]:"..[1] #���var of [1]:999
    add([1],1):[3]
    if [3]==1000 {
        0:[4]
        for [4]<3{
            [3] #���3��1000
        }
        0:[4]
        for{
            if [4]==3{break}
            [3]
            1:+[4] # �Լ�1
        }# Ч��ͬ��
        for [5]::(1,3,1){ # for [5]=1 to 3 step 1
            iif(1,[3],"false") # ?:��Ŀ�����
        }# Ч��ͬ��
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