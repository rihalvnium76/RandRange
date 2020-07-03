//RandRange_v1.5.7
//encoding:gb2312
//--std=c++11
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint> // uint_64t
#include <ctime> // time()
#include <cmath> // pow()
#include <cctype> // isspace()...

#include <vector> // vector
#include <string> // string
#include <exception> // exception
#include <unordered_map> // unordered_map

#define RNDRG_VERSION "1.5.7 - CLI Demo"
// Debug Ouput
static bool *DBG_C_PTR = NULL;
#define dbgt(T) const char *TAG = #T;
#define dbgc(C_PTR) DBG_C_PTR = (C_PTR);
#define dbgo(M, FMT, ...) if(!DBG_C_PTR || *DBG_C_PTR) fprintf(stderr, "[" #M "]%s#%d: " FMT, TAG, __LINE__, ##__VA_ARGS__);
#define dbgv(FMT, ...) dbgo(V, FMT, ##__VA_ARGS__)
#define dbgi(FMT, ...) dbgo(I, FMT, ##__VA_ARGS__)
#define dbgd(FMT, ...) dbgo(D, FMT, ##__VA_ARGS__)
#define dbgw(FMT, ...) dbgo(W, FMT, ##__VA_ARGS__)
#define dbge(FMT, ...) dbgo(E, FMT, ##__VA_ARGS__)

/*
    DECLARE
*/
namespace Module {
    namespace General {
        static constexpr class SplitTable {
            char tb[256];
        public:
            constexpr SplitTable() : tb() {
                tb['('] = 1, tb['['] = 2, tb['{'] = 3;
                tb[')'] = 4, tb[']'] = 5, tb['}'] = 6;
                tb['\"'] =7, tb['\''] = 8;
            }
            constexpr int operator [] (int i) const {
                return tb[i];
            }
        } splitTable;
        class Split {
            char *str, *cur;
            char *que; int qpos;
            std::string ret;
            //static SplitTable& tb = splitTable;

            void qpush(char c) { que[++qpos] = c; }
            void qpop() { que[qpos--] = '\0'; }
            char qtop() { return que[qpos]; }
            bool qempty() { return qpos==-1; }

            void release() {
                if(str) delete[] str;
                if(que) delete[] que;
            }
        public:
            Split(const char *s) : str(NULL), que(NULL) {
                //dbgt(Split)
                //dbgi("Constructor called\n")
                Reset(s);
            }
            Split() : Split(NULL) {}
            ~Split() {
                release();
            }

            void Reset(const char *s) {
                release();
                que = str = cur = NULL;
                qpos = -1;

                int len = s==NULL? 0: strlen(s);
                if(len>0) {
                    cur = str = new char[len + 1]; // Notice!!!
                    que = new char[len + 1];
                    strcpy(str, s);
                }
            }
            // �и��ַ���
            // ����ֵ��ָ���¸��Ǻ���ʼ��ָ��
            // delim �ָ���
            // ignoreDelim �Ƿ���԰������Ż������ڵķָ���
            // useEscape �Ƿ�ʹ��ת���
            // escape ת�����Ĭ��Ϊ ��б��
            std::string SplitByChar(char delim, bool ignoreDelim = false, bool useEscape = false, char escape = '\\') {
                ret.clear();
                if(!cur || !*cur) return ret; // null string or EOF

                // scan from delim to normal char
                for(; *cur && *cur==delim; ++cur);
                // scan from normal char to next delim
                bool skip = false;
                for(char c; c = *cur; ++cur) {
                    if(c==delim && (!ignoreDelim || (ignoreDelim && qempty())))
                        break;
                    else if(ignoreDelim) {
                        int v = splitTable[c];
                        if(skip==true) {
                            skip = false;
                            goto endchk;
                        }
                        if(useEscape && c==escape) {
                            skip = true;
                            continue;
                        }
                        if(v>=1 && v<=3 && (qempty() || (qtop()!='\'' && qtop()!='\"'))) qpush(c);
                        else if(v>=4 && v<=6 && !qempty() && splitTable[qtop()] == v-3) qpop(); // match
                        else if(c=='\'' || c=='\"')
                            if(!qempty() && c==qtop()) qpop();
                            else qpush(c); // match
                    }
                    endchk:;
                    ret.push_back(c);
                }
                return ret;
            }

            // �����и��ַ���
            std::vector<std::string> SplitAll(char delim, bool ignoreDelim = false, bool useEscape = false, char escape = '\\') {
                std::vector<std::string> rt;
                std::string s;
                while(true) {
                    s = SplitByChar(delim, ignoreDelim, useEscape, escape);
                    if(s.empty()) break;
                    else rt.push_back(s);
                }
                return rt;
            }

            static void Test() {
                Split ss("1 100 5   \"99  ( 88\"  100");
                std::string ret;
                while(!(ret = ss.SplitByChar(' ', true)).empty())
                    puts(ret.c_str());
                puts("--END1");
                Split ss2("1 100 5   \"99  ( 88\"  100");
                for(std::string s : ss2.SplitAll(' ', true))
                    puts(s.c_str());
                puts("--END2");
            }
        }; // class Split
    }
    namespace Extension {
        class IExtension;
    }
}
namespace UI {
    struct State {
        bool StrictMode; // �ϸ�ģʽ
        bool DebugMode; // ����ģʽ
        bool ColonOrderMode; // ��:����.��������ģʽ
        bool ConsoleMode; // ������ģʽ�����⣩
        Module::General::Split Args; // ����ָ�
        State();
    };
    class Application {
        State state;
        char inputBuffer[512]; // public input buffer
        static const int IN_BUF_LEN = sizeof(inputBuffer) / sizeof(inputBuffer[0]);
        std::unordered_map<std::string, Module::Extension::IExtension*> moduleList; // module function list
    public:
        Application();
        void Start();
        void Start(int&, char**&);
        bool ParseCmd(const char *);
        void RegisterCmd(const std::string&, Module::Extension::IExtension *);
    private:
        void help_command();
        void help_module();
        void help_cli();
        void help_script();
        void eval();
        void quickRndGen(int, const char *);
    };

}


namespace Module {

    namespace General {

        // Algorithm : xoshiro256**
        // Source : http://prng.di.unimi.it/xoshiro256starstar.c
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
        }; // struct xoshiro256ss

        // Algorithm : SplitMix64
        // Source : http://prng.di.unimi.it/splitmix64.c
        struct SplitMix64 {
            uint64_t x; /* The state can be seeded with any value. */
            uint64_t next() {
                uint64_t z = (x += 0x9e3779b97f4a7c15);
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
                z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
                return z ^ (z >> 31);
            }
        }; // struct SplitMix64

        // RandRange Core
        // Dependence : struct xoshiro256ss , struct SplitMix64
        class RNDRG {
            xoshiro256ss xs;
            SplitMix64 sm;
            unsigned int seed;

        public:
            // ��������
            RNDRG(unsigned int seed = time(NULL)) {
                Seed(seed);
            }
            void Seed(unsigned int seed) {
                sm.x = seed;
                this->seed = seed;
                for(int i=0; i<4; ++i) xs.s[i] = sm.next();
            }
            // ���ص�ǰ����
            unsigned int Seed() { return seed; }

            // ���������[0,0xFFFFFFFF]
            inline uint64_t RI() {
                return xs.next();
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

        namespace Base64Code {
            // Notice: str_len = size + 1
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

        static inline void ClearStdin() { // ������뻺����
            scanf("%*[^'\n']%*c");
        }
        void Trim(char *s) { // ȥ��β�հ��ַ�����������Ϊ�ַ�����
            char *p = (char *)1, *q = 0, *c;
            for (c = s; *c; ++c) if (!isspace(*c)) p = q ? p : c, q = c;
            for (c = s; p <= q; *c++ = *p++);
            *c = 0;
        } // by Edgar
    } // namespace General

    // Body of Extension module code
    namespace Custom {

        General::RNDRG Dice;

        class ArgParserException : public std::exception {
        protected:
            std::string ret;
        public:
            ArgParserException(std::string baseErrInfo) {
                ret = baseErrInfo;
            }
            ArgParserException(std::string baseErrInfo, std::string extErrInfo) {
                ret = baseErrInfo + "\n" + extErrInfo;
            }
            ArgParserException()
            : ArgParserException("FATAL: Unknown error code") {}

            const char *what() const throw() {
                return ret.c_str();
            }
        }; // class ArgParserException (Base Class)
        class SyntaxErrorException : public ArgParserException {
        public:
            SyntaxErrorException()
            : ArgParserException("ERROR: Command syntax error") {}
        }; // class SyntaxErrorException
        class InvalidCommandException : public ArgParserException {
        public:
            InvalidCommandException()
            : ArgParserException("ERROR: Invalid command") {}
        }; // class InvalidOrderException

        // DEBUGMODE����
        // Base64x�ӽ���
        // �������룺Module::Gernal::Base64Code
        class Base64xCode {
            std::string str;
            const char *alpha, *alpha_v;
            const int alphaVLen; // ��ĸ����

            int findFirstOf(char c) {
                int i;
                if(isupper(c))
                    i = c - 'A';
                else if(isdigit(c))
                    i = 26 + c - '0'; // Notice the offset
                else if(c=='-')
                    i = 36;
                else if(c=='_')
                    i = 37;
                else if(c=='\0')
                    i = -1;
                else // Unknown Character
                    i = -2;
                return i;
            }
        public:
            // sΪ��Ҫ����ļ�/�����ַ���
            Base64xCode(const char *s)
                : str(s)
                , alpha("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
                , alpha_v("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-")
                , alphaVLen(strlen(alpha_v))
            {}
            inline void SetText(const char *s) {
                str = s;
            }
            // ��ȡ���
            std::string GetResult() { return str; }

            // �ı�->Base64
            Base64xCode& EncodeTextToBase64() {
                if(str.empty()) return *this;
                using namespace Module::General;
                char *t; int len = str.length();
                t = new char[Base64Code::BASE64_ENCODE_OUT_SIZE(len)];
                Base64Code::base64_encode(reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str())), len, t);
                str = t;
                delete[] t;
                return *this;
            }
            // Base64->�ı�
            Base64xCode& DecodeBase64ToText() {
                if(str.empty()) return *this;
                using namespace Module::General;
                char *t; int len = str.length();
                t = new char[Base64Code::BASE64_DECODE_OUT_SIZE(len)];
                Base64Code::base64_decode(str.c_str(), len, reinterpret_cast<unsigned char*>(t));
                str = t;
                delete[] t;
                return *this;
            }

            /*
                Base64ȫ��/Сдת����
                ��ĸ��ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
                ��ʽ���Ⱥ�������ʶ��(1λ��R0/W1/X2)+[���С/��д��ĸλ��ģʽ(1λ,AB,CD,EF...89ѭ��)+С/��д��ĸλ��(0-18λ����ȥ��Ϊ�ָ�������ĸ)+�ָ���(1λ,Z,Y,X...Aѭ��)+ȫС/��д��Base64����Ƭ��(35λ)]*n!
                + -> - ; / -> _
            */

            // Base64->Baseplus64
            Base64xCode& EncodeBase64ToBp() {
                if(str.empty()) return *this;
                dbgt(EnBx)

                int len = str.length();
                const char *s = str.c_str();
                std::string ret = "R"; // pad_c = 0
                // pad count
                if(s[len-1]=='=') {
                    ret = "W"; // pad_c = 1
                    if(s[len-2]=='=') ret = "X"; // pad_c = 2
                }
                // main
                std::string upperFlags, lowerFlags, cvrtStr; // λ�ñ�� ת������ĸ
                int caseFlagIndex = 0, delimFlagIndex = 35; // ��Сд������� �ָ����������
                int p = 0; // ��ǰɨ��λ��
                while(true) {
                    // �������ɨ�賤��
                    int scanLen = len - p;
                    if(scanLen>35) scanLen = 35;
                    else if(0>=scanLen) break;
                    // ɨ��һ��(35λ)�ַ�
                    for(int i=0; i<scanLen; ++i) {
                        char c = s[p+i];
                        if(isupper(c)) {
                            cvrtStr.push_back(toupper(c)); // ȫ��ת��Ϊ��д��ĸ
                            upperFlags.push_back(alpha[i<delimFlagIndex? i: i+1]); // λ�ü�¼
                        }
                        else if(islower(c)) {
                            cvrtStr.push_back(toupper(c)); // ȫ��ת��Ϊ��д��ĸ
                            lowerFlags.push_back(alpha[i<delimFlagIndex? i: i+1]);
                        }
                        else if(isdigit(c))
                            cvrtStr.push_back(c);
                        else if(c=='+')
                            cvrtStr.push_back('-');
                        else if(c=='/')
                            cvrtStr.push_back('_');
                        else if(c=='=')
                            break;
                        else {
                            dbge("Illegal Character: %c (0x%X)\n", c, c)
                            str.clear();
                            return *this;
                        }
                    }
                    // д��
                    if(upperFlags.length()<=lowerFlags.length())
                        ret += alpha[caseFlagIndex+1] + upperFlags;
                    else
                        ret += alpha[caseFlagIndex] + lowerFlags;
                    ret += alpha[delimFlagIndex] + cvrtStr;
                    // ����״̬
                    if(++caseFlagIndex>=35) caseFlagIndex = 0;
                    if(--delimFlagIndex<0) delimFlagIndex = 35;
                    p += 35;
                    upperFlags.clear();
                    lowerFlags.clear();
                    cvrtStr.clear();
                }
                str = ret;
                return *this;
            } 
            // Baseplus64->Base64
            Base64xCode& DecodeBpToBase64() {
                if(str.empty()) return *this;
                dbgt(DeBx)

                int len = str.length();
                const char *s = str.c_str();
                std::string ret;
                // pad
                char pads[3]={};
                switch(toupper(s[0])) {
                    case 'R': break;
                    case 'X': pads[1] = '=';
                    case 'W': pads[0] = '='; break;
                    default:
                        dbge("Illegal Character: %c (0x%X)\n", s[0], s[0])
                        str.clear();
                        return *this;
                }
                // main
                std::string caseFlags, cvrtStr; // λ�ñ�� ��ת���ַ���
                int caseFlagIndex = 0, delimFlagIndex = 35; // ��Сд������� �ָ����������
                int p = 1; // ��ǰɨ��λ��
                while(true) {
                    // ʶ���Сд���
                    bool isUpperFlag;
                    if(p<len) {
                        char c = toupper(s[p]);
                        if(c==alpha[caseFlagIndex])
                            isUpperFlag = false;
                        else if(c==alpha[caseFlagIndex+1])
                            isUpperFlag = true;
                        else {
                            dbge("Illegal Case Flag: %c (0x%X)\n", c, c)
                            str.clear();
                            return *this;
                        }
                        ++p; // ������Сд���
                    }
                    else
                        break;
                    // ɨ��λ�ñ��
                    for(; p<len; ++p) {
                        char c = toupper(s[p]); // ȫ��ת��д
                        if(c==alpha[delimFlagIndex]) break; // �����ָ�����ֹɨ��
                        if(isalnum(c))
                            caseFlags.push_back(c);
                        else {
                            dbge("Illegal Position Flag: %c (0x%X)\n", c, c)
                            str.clear();
                            return *this;
                        }
                    }
                    ++p; // �����ָ���
                    // ɨ���ת���ַ���
                    // �������ɨ�賤��
                    int scanLen = len - p;
                    if(scanLen>35) scanLen = 35;
                    for(int i=0; i<scanLen; ++i) {
                        char c = s[p+i];
                        if(isalnum(c))
                            cvrtStr.push_back(isUpperFlag? tolower(c): toupper(c)); // ȫ��ת����һ����̬
                        else if(c=='-')
                            cvrtStr.push_back('+');
                        else if(c=='_')
                            cvrtStr.push_back('/');
                        else {
                            dbge("Illegal Position Flag: %c (0x%X)\n", c, c)
                            str.clear();
                            return *this;
                        }
                    }
                    // ��ԭ��Сд
                    auto iter = caseFlags.begin();
                    int cStrLen = cvrtStr.length();
                    for(; iter!=caseFlags.end(); ++iter) {
                        char c = *iter;
                        int i = findFirstOf(c); // index
                        switch(c) {
                            case -1:
                                goto end_for;
                            case -2:
                            case 36:
                            case 37:
                                dbge("Illegal Character(Code ERROR): %c (0x%X)\n", c, c)
                                str.clear();
                                return *this;
                        }
                        // �ָ���ƫ�ƻ�ԭ
                        i = i>delimFlagIndex? i-1: i;
                        // Խ����
                        if(i>=cStrLen) {
                            dbge("Illegal Position Flag(Out Of Bound): %c (0x%X)  %d/%d\n", c, c, i, cStrLen-1)
                            str.clear();
                            return *this;
                        }
                        // ת��
                        char c2 = cvrtStr[i];
                        if(isalpha(c2))
                            cvrtStr[i] = isUpperFlag? toupper(c2): tolower(c2);
                        else {
                            dbge("Illegal Conversion Character: %c (0x%X)", c2, c2)
                            str.clear();
                            return *this;
                        }
                    }
                    end_for:;
                    // ����״̬
                    ret += cvrtStr;
                    if(++caseFlagIndex>=35) caseFlagIndex = 0;
                    if(--delimFlagIndex<0) delimFlagIndex = 35;
                    p += 35;
                    caseFlags.clear();
                    cvrtStr.clear();
                }
                ret += pads;
                str = ret;
                return *this;
            }

            // Bp64ά�����Ǽ���
            // ���ġ�Կ�׽���Bp64����
            // ʹ��ά�������������
            Base64xCode& EncryptBp(std::string pwd) {
                if(str.empty() || pwd.empty()) return *this;
                dbgt(EnBp)

                std::string ret = str; // ����ԭstr
                // ��������
                str = pwd;
                pwd = EncodeTextToBase64().EncodeBase64ToBp().GetResult();
                str = ret; // ��ԭstr
                ret.clear();

                const char *s = str.c_str(), *pw = pwd.c_str();
                const int strLen = str.length(), pwdLen = pwd.length();

                int strIndex = 0, pwdIndex = 0;
                while(true) {
                    int y = findFirstOf(pw[pwdIndex]), x = findFirstOf(s[strIndex]);
                    if(y<0 || x<0) {
                        dbge("Array Index Out Of Bounds: x= %d, y= %d\n",x,y)
                        str.clear();
                        return *this;
                    } else if(x+y >= alphaVLen) {
                        dbge("Array Index Out Of Bounds(Code ERROR): x= %d, y= %d, x+y= %d\n",x,y,x+y)
                        str.clear();
                        return *this;
                    }
                    ret.push_back(alpha_v[x+y]);

                    // ״̬����
                    if(++strIndex>=strLen) break;
                    if(++pwdIndex>=pwdLen) pwdIndex = 0;
                }

                str = ret;
                return *this;
            }
            // Bp64ά�����ǽ���
            Base64xCode& DecryptBp(std::string pwd) {
                if(str.empty() || pwd.empty()) return *this;
                dbgt(DeBp)

                std::string ret = str; // ����ԭstr
                // ��������
                str = pwd;
                pwd = EncodeTextToBase64().EncodeBase64ToBp().GetResult();
                str = ret; // ��ԭstr
                ret.clear();

                const char *s = str.c_str(), *pw = pwd.c_str();
                const int strLen = str.length(), pwdLen = pwd.length();

                int strIndex = 0, pwdIndex = 0;
                while(true) {
                    int y = findFirstOf(pw[pwdIndex]), x = 38 + findFirstOf(s[strIndex]); // findFirstOf findSecondOf
                    if(y<0 || x<0) {
                        dbge("Array Index Out Of Bounds: x= %d, y= %d\n",x,y)
                        str.clear();
                        return *this;
                    } else if(x-y<0) {
                        dbge("Array Index Out Of Bounds(Code ERROR): x= %d, y= %d, x+y= %d\n",x,y,x+y)
                        str.clear();
                        return *this;
                    }
                    ret.push_back(alpha_v[x-y]);

                    // ״̬����
                    if(++strIndex>=strLen) break;
                    if(++pwdIndex>=pwdLen) pwdIndex = 0;
                }

                str = ret;
                return *this;
            }

            static void Test() {
                Base64xCode bs("OPenVPn����");
                printf("%s|\n",bs.EncodeTextToBase64().GetResult().c_str());
                printf("%s|\n",bs.EncodeBase64ToBp().GetResult().c_str());
                printf("%s|\n",bs.EncryptBp("lzstupid").GetResult().c_str());
                printf("%s|\n",bs.DecryptBp("lzstupid").GetResult().c_str());
                printf("%s|\n",bs.DecodeBpToBase64().GetResult().c_str());
                printf("%s|\n",bs.DecodeBase64ToText().GetResult().c_str());
                puts("--END 1");
                Base64xCode bs2("QWERTYUIOPASDFGHJKLZXCVBNM123456789qwertyuiopasdfghjklzxcvbnm123456789����");
                printf("%s|\n",bs2.EncodeTextToBase64().GetResult().c_str());
                printf("%s|\n",bs2.EncodeBase64ToBp().GetResult().c_str());
                printf("%s|\n",bs2.EncryptBp("lzstupid").GetResult().c_str());
                printf("%s|\n",bs2.DecryptBp("lzstupid").GetResult().c_str());
                printf("%s|\n",bs2.DecodeBpToBase64().GetResult().c_str());
                printf("%s|\n",bs2.DecodeBase64ToText().GetResult().c_str());
                puts("--END 2");
            }
            static void Test2() {
                Base64xCode bs("aBBBa��");
                printf("%s|\n",bs.EncodeBase64ToBp().GetResult().c_str());
                printf("%s|\n",bs.DecodeBpToBase64().GetResult().c_str());
                puts("--END3");
                Base64xCode bs2("QWERTYUIOPASDFGHJKLZXCVBNM123456789qwertyu//+++iopasdfghjkl+zxcvbnm12345678/9+");
                printf("%s|\n",bs2.EncodeBase64ToBp().GetResult().c_str());
                printf("%s|\n",bs2.DecodeBpToBase64().GetResult().c_str());
                puts("--END4");
                Base64xCode bs3("��wee�Ĺ��E�϶��Ҹղ��̺۴���ά�����˰�����˷������������������������// ����վ������ǰ���ǣ��²��ո����������jb��ʬ������յۡ�������ʩ֮��������������");
                printf("%s|\n",bs3.EncodeTextToBase64().GetResult().c_str());
                printf("%s|\n",bs3.EncodeBase64ToBp().GetResult().c_str());
                printf("%s|\n",bs3.EncryptBp("tixŮװ").GetResult().c_str());
                printf("%s|\n",bs3.DecryptBp("tixŮװ").GetResult().c_str());
                printf("%s|\n",bs3.DecodeBpToBase64().GetResult().c_str());
                printf("%s|\n",bs3.DecodeBase64ToText().GetResult().c_str());
                puts("--END5");
            }
        }; // class Baseplus64Code
    
    } // namespace Custom

    // Extension module entrance with parsing code
    namespace Extension {

        class IExtension {
        protected:
            void Register(UI::Application& app, const std::string& name) {
                app.RegisterCmd(name, this);
            }
        public:
            virtual void Entry(const UI::State& state, const char *arg) = 0;
        }; // class IExtension (interface)

        // osu�������Ȩ��������
        class OsuRndScoreCalcModule : public IExtension {
        public:
            OsuRndScoreCalcModule(UI::Application& app) { Register(app, "osuc"); }
            void Entry(const UI::State& state, const char *arg) override {
                using namespace Module::General;
                using namespace Module::Custom;
                dbgt(osuc)
                const static double MAX_RND_RATIO = 1.05; // ��������ֵ

                std::vector<double> num;
                Split args(arg);
                for(std::string s : args.SplitAll(',')) {
                    dbgd("arg: %s| %f\n", s.c_str(), atof(s.c_str()))
                    num.push_back(atof(s.c_str()));
                }
                int n = num.size();
                if(n==0 || n%2==1) throw SyntaxErrorException(); // ��Ϊż���������򱨴�
                printf("--��ǰ��ʽ��Score = osu_score * rnd(avg(ACCs), %.2f)\n", MAX_RND_RATIO);
                // ��ƽ��ACC
                double avg_acc = 0;
                for(int i=1; i<n; i+=2) avg_acc += num[i];
                avg_acc /= n/2.0;
                printf("--ACCƽ��ֵ��%f\n", avg_acc);
                // ������
                for(int i=0; i<n; i+=2) {
                    double origin = num[i], rnd = Dice.fr(avg_acc, MAX_RND_RATIO), score = origin*rnd;
                    printf("��%dλ��ҵļ�Ȩ������%f\t(ԭʼ�֣�%d, ACC��%f, Ȩ����%f)\n", i/2+1, score, (int)origin, num[i+1], rnd);
                }
            }
        }; // class OsuRndScoreCalc

        // Base64x�����
        // ������<flags>,<text>,[password]
        // flags(˳��ִ�в���):
        //    B Text->Base64 ; b Base64->Text
        //    X Base64->Bp64 ; x Bp64->Base64
        //    V Bp64 Encrypt ; v Bp64 Decrypt (Vigen��re cipher)
        class Base64xModule : public IExtension {
        public:
            Base64xModule(UI::Application& app) { Register(app, "b64x"); }
            void Entry(const UI::State& state, const char *arg) override {
                using namespace Module::Custom;
                using namespace Module::General;
                Split split(arg);
                std::vector<std::string> args = split.SplitAll(',');
                int n = args.size();
                switch(n) {
                    case 3: // ȥ����β����
                        if(args[2].front()=='\"') args[2].erase(0, 1);
                        if(args[2].back()=='\"') args[2].pop_back();
                    case 2:
                        if(args[1].front()=='\"') args[1].erase(0, 1);
                        if(args[1].back()=='\"') args[1].pop_back();
                        break;
                    default: throw SyntaxErrorException();
                }
                Base64xCode bs(args[1].c_str());
                for(auto c : args[0])
                    if(c=='B') bs.EncodeTextToBase64();
                    else if(c=='X') bs.EncodeBase64ToBp();
                    else if(c=='V') 
                        if(n>=3) bs.EncryptBp(args[2]);
                        else throw SyntaxErrorException();
                    else if(c=='b') bs.DecodeBase64ToText();
                    else if(c=='x') bs.DecodeBpToBase64();
                    else if(c=='v')
                        if(n>=3) bs.DecryptBp(args[2]);
                        else throw SyntaxErrorException();
                puts(bs.GetResult().c_str());
            }
        }; // class Base64xTools
    } // namespace Extension
} // namespace Module

namespace UI {

    /*struct State {
        bool StrictMode; // �ϸ�ģʽ
        bool DebugMode; // ����ģʽ
        bool ColonOrderMode; // ��:����.��������ģʽ
        bool ConsoleMode; // ������ģʽ�����⣩
        Module::General::Split Args; // ����ָ�
        State()
            : StrictMode(false)
            , DebugMode(false)
            , ColonOrderMode(false)
            , ConsoleMode(false)
            , Args() // init table
        {}
    }; // struct State*/
    State::State()
        : StrictMode(false)
        , DebugMode(false)
        , ColonOrderMode(false)
        , ConsoleMode(false)
        , Args() // init table
    {}

    Application::Application() : state() {
        dbgc(&state.DebugMode)
    }

    void Application::Start() {
        puts("RandRange " RNDRG_VERSION "\n����.h�鿴����\n����֧�������в�������\n");
        do {
            printf("> ");
            inputBuffer[0] = '\0'; // clear
            // input
            fgets(inputBuffer, IN_BUF_LEN, stdin);
            Module::General::Trim(inputBuffer);
        } while(ParseCmd(inputBuffer));
    }
    void Application::Start(int& argc, char **& argv) {
        if(argc==1) Start();
        else {
            state.ConsoleMode = true;
            int expressIndex = 1;
            if(argc>=2 && argv[1][0]=='-') {
                expressIndex = 2;
                switch(argv[1][1]) {
                    case 'd': state.DebugMode = true; break;
                    case '?':
                    case 'h': ParseCmd(".hc"); return;
                    default:
                        puts("ERROR: Unknown Argument");                    
                }
            }
            if(argc-1 == expressIndex) ParseCmd(argv[expressIndex]);
        }
    }
    // �������
    // ����ֵ��false�˳����� true�������
    // ������state.Args
    bool Application::ParseCmd(const char *str) {
        using namespace Module::Custom;
        using namespace Module::General;
        dbgt(ParserCmd)
        Split &args = state.Args;
        args.Reset(str);
        const char *arg = args.SplitByChar(' ').c_str();
        if(arg[0]!='\0' && arg[0]!='#') {
            try {
                // ����ģʽ
                if((!state.ColonOrderMode && arg[0]=='.' && !isdigit(arg[1])) || (state.ColonOrderMode && arg[0]==':'))
                {
                    // �˳�
                    if(arg[1]=='q') return false;
                    // ����
                    else if(arg[1]=='h') {
                        if(arg[2]=='\0') help_command();
                        else if(arg[2]=='c' && arg[3]=='\0') help_cli();
                        else if(arg[2]=='t' && arg[3]=='\0') help_module();
                        else if(arg[2]=='s' && arg[3]=='\0') help_script();
                        else throw InvalidCommandException();
                    }
                    // �ű�����
                    else if(arg[1]=='r' && arg[2]=='\0') eval();
                    // ���������(�����������)
                    else if(arg[1]=='p' && arg[2]=='l' && arg[3]=='\0') {
                        std::string d = args.SplitByChar(' ');
                        if(d.empty()) throw SyntaxErrorException();
                        dbgd("pl: arg1= %s| %d", d.c_str(), atoi(d.c_str()))
                        quickRndGen(atoi(d.c_str()), args.SplitByChar(' ').c_str());
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
                                    if(!isspace(c)) throw InvalidCommandException();
                            }
                        }
                    // ��ȡ/�������������
                    else if(arg[1]=='s' && arg[2]=='d' && arg[3]=='\0') {
                        const char *s = args.SplitByChar(' ', false).c_str();
                        unsigned int sd;
                        if(s) {
                            sscanf(s, "%u", &sd);
                            Dice.Seed(sd);
                        }
                        printf("��ǰ����ֵ��%u\n", Dice.Seed());
                    }
                        // ����ģ��
                    else if(arg[1]=='t' && arg[2]=='l' && arg[3]=='\0') {
                        const char *name = args.SplitByChar(' ', false).c_str();
                        auto rs = moduleList.find(name);
                        if(rs != moduleList.end())
                            rs->second->Entry(state, args.SplitByChar('\0').c_str());
                        else
                            throw InvalidCommandException();
                    }
                    else
                        throw InvalidCommandException();
                }
                else if(!state.StrictMode && (isdigit(arg[0]) || (state.ColonOrderMode && arg[0]=='.' && isdigit(arg[1]))))
                    quickRndGen(-1, arg);
                else
                    throw InvalidCommandException();
            } catch(ArgParserException &e) {
                puts(e.what());
            }
        }
        return !state.ConsoleMode;
    }
    // ע������
    void Application::RegisterCmd(const std::string& name, Module::Extension::IExtension *mod) {
        moduleList[name] = mod;
    }
    // �����������
    void Application::help_command() {
        puts(
            "˵����\n���� ����\n\t˵��\n��<>�Ĳ�����ʾ�����[]�Ĳ�����ʾ�ɲ���\n\n"
            "<a> <b> <n>\n.pl <d> <a> <b> <n>\n"
            "\t���������\n\t������a:��Сֵ��b���ֵΪb��n:���ɸ�����d:���С����λ��\n"
            "\tn<0ʱ������|n|����������ۼӣ�a��bΪС��ʱ���������������\n"
            "\t˵���������ϸ�ʱ���õ�һ�ֲ���ָ�����������ɷ�ʽ\n"
            ".m<mode>\n\t���û���ģʽ\n\t������mode��Ϊ����ֵ:\n"
            "\t\ts\t����/�ر��ϸ�ģʽ\n\t\tc\tʹ��/ȡ��:����.��������ģʽ\n\t\td\t����/�رյ���ģʽ\n"
            ".sd [seed]\n\t����/��ʾ���������������\n\t������seed:�µ�����ֵ(uint����)\n\t˵����ʡ��seed��ʾ��ǰ����ֵ\n"
            ".r <code>\n\t��δ���á�ִ�����ü������Դ���\n"
            ".h[flags]\n\t��ʾ����\n\t˵����ʡ��flags����ʾ�������\n"
            "\tflags��Ϊ����ֵ��\n"
            "\t\tc\t��ʾ�����а���\n\t\tt\t��ʾ���õ����ù���\n\t\ts\t��δ���á���ʾ���ü��������﷨\n"
            ".tl <tool> [args]\n\t�������ù���\n\t������tool:��������args���߲���\n\t˵�����������ù����б��.ht����\n"
            "#[string]\n\tע��\n\t������string:ע������\n.q\n\t�˳�\n"
        );
    }
    // ģ�鹦�ܰ���
    void Application::help_module() {
        puts(
            "���ù����б�\n\nosuc <���1����>,<���1׼ȷ��>[,<���2����>,<���2׼ȷ��>...]\n"
            "\t\t����osu!�������Ȩ����������\n"
            "b64x <flag>,<string>,[password]\n\tBase64����/����\n\t������flag:˳��ִ�е�������ִ�Сд����֧�ֵ����\n"
            "\t\tB\tBase64����\n\t\tb\tBase64����\n"
            "\t\tX\tBase64תBaseplus64\n\t\tx\tBaseplus64תBase64\n"
            "\t\tV\t��Baseplus64����ʹ�ñ���ά�������������\n\t\tv\t��Baseplus64����ʹ�ñ���ά�������������\n"
            "\tstring\tҪ������ַ�������Ҫ������ַ�����Ҫ��˫��������\n"
            "\tpassword\tִ��ά����������ӽ����������Կ��Ҫ��˫��������\n"
        );
    }
    // �����в�������
    void Application::help_cli() {
        puts(
            "[-d|-?|-h] \"express\"\n"
            "����:\n\t-d\t��������ģʽ\n"
            "\t-?,-h\t��ʾ����\n"
            "\texpress\tִ�б��ʽ����Ҫ��˫������������˫�����ڵ�˫����ǰ��Ҫ����ת���\\\n"
        );
    }
    // �ű��﷨����
    void Application::help_script() {}
    // �ű�����
    void Application::eval() {}
    // �������������
    // ������d�������� num0��һ�����ֵ��ַ���
    void Application::quickRndGen(int d, const char *num0) {
        using namespace Module::General;
        using namespace Module::Custom;
        dbgt(qrnd)
        Split& args = state.Args;
        // ԭʼ����
        const char *s[3] = {num0, args.SplitByChar(' ').c_str(), args.SplitByChar(' ').c_str()};
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

        //if(state.DebugMode)printf("[D]: %s| %s| %s| %s %f %f %d %d %s|\n",s[0],s[1],s[2],isfloat?"Float":"Int",isfloat?a.d:(double)a.i,isfloat?b.d:(double)b.i,n,d,pfarg);
        dbgd("%s| %s| %s| %s a=%f b=%f n=%d d=%d pfarg=%s|\n",s[0],s[1],s[2],isfloat?"Float":"Int",isfloat?a.d:(double)a.i,isfloat?b.d:(double)b.i,n,d,pfarg)

        if(isfloat)
            if(n<0) printf(pfarg, Dice.frs(a.d, b.d, -n), '\n');
            else for(int i=0, z=n>0? n: -n; i<z; ++i) printf(pfarg, Dice.fr(a.d, b.d), i==z-1? '\n': ' ');
        else
            if(n<0) printf("%d\n", Dice.irs(a.i, b.i, -n));
            else for(int i=0, z=n>0? n: -n; i<z; ++i) printf("%d%c", Dice.ir(a.i, b.i), i==z-1? '\n': ' ');
    }
} // namespace UI

int main(int argc, char **argv) {
    using namespace Module::Extension;
    UI::Application ui;
    // Load Module
    OsuRndScoreCalcModule osuRndScoreCalc(ui);
    Base64xModule base64x(ui); 
    ui.Start(argc, argv);
    return 0;
}