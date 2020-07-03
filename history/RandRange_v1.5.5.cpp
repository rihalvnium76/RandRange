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
		return seed;
	}

	inline uint64_t RI() {
		// ���������[0,0xFFFFFFFF]
		return x256ss.next();
	}
	inline double RF() {
		// �����������[0,1)
		const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | RI() >> 12 };
		return u.d - 1.0;
	}

	int ir(int a, int b) {
		// ���ط�ΧΪ[a,b]���������
		return RI() % (b-a+1) + a;
	}
	int irs(int a, int b, int n) {
		// ����n����ΧΪ[a,b]���������֮��
		int s = 0;
		for(int i=0; i<n; ++i) s += ir(a, b);
		return s;
	}
	double fr(double a, double b) {
		// ���ط�ΧΪ[a,b)�����������
		return RF() * (b-a) + a;
	}
	double frs(double a, double b, int n) {
		// ����n����ΧΪ[a,b)�����������֮��
		double s = 0;
		for(int i=0; i<n; ++i) s += fr(a, b);
		return s;
	}

	static double pl(double a,int d) {
		//�������ơ�a���֣�dС��λ��,d<0������
		if(d>=0) {
			double s=pow(10,d);
			a=(int)(a*s)/s;
		}
		return a;
	}
}; // class RNDRG

namespace RDPM { // Random Public Module
	RNDRG dice; // �����������
	// ������ �б�
	enum ERROR_CODE_LIST {
		ERR_default,
		ERR_ORDER_SYNTAX,
		ERR_ORDER_INVAILD,
		ERR_SCRIPT_PARSE_ERROR
	};
	static const char *ERROR_CODE_STR[] = {
		"FATAL: Unknown error code ",
		"ERROR: Instruction syntax error",
		"ERROR: Invalid instruction",
		"ERROR: Script parsing error"
	};
	// �������б���
	static const int ERROR_CODE_LIST_LEN  = sizeof(ERROR_CODE_STR)/sizeof(ERROR_CODE_STR[0]);
	// ��������
	struct ErrorCode {
		ERROR_CODE_LIST baseErrCode; // ����������
		const char *baseErrStr; // ������������Ϣ
		std::string extErrStr; // ��չ��������Ϣ

		ErrorCode(const ERROR_CODE_LIST baseCode, const std::string &&extInfo = ""):
		baseErrCode(baseCode), extErrStr(extInfo) {
			if(baseCode>ERROR_CODE_LIST_LEN || baseCode<=0)
				baseErrStr = ERROR_CODE_STR[0];
			else
				baseErrStr = ERROR_CODE_STR[baseCode];
		}
	};
} // namespace RDPM

/*
class RDINPT { // Random Interpreter
public:
}; // class RDINPT
*/

class UI_v1b {
	// ״̬����
	static const int IN_BUF_LEN = 512; // ���뻺��������
	struct {
		char inputBuffer[UI_v1b::IN_BUF_LEN]; // ���뻺����
		bool mode_strict; // �ϸ�ģʽ
		bool mode_debug; // ����ģʽ
		bool mode_colonOrder; // ��:����.��������ģʽ
		bool mode_console; // ������ģʽ�����⣩
	} state;
	// ParseIns() �˳���
	enum {
		PROGRAM_CONTINUE,
		PROGRAM_EXIT
	};
	// ���
	void Start() {
		using namespace RDPM;
		if(!state.mode_console)
			puts("RandRange v1.5.5 - CLI Demo\n����.h�鿴����\n����֧�������в�������һ������Ϊִ����䣬������˫����������\n");
		while(true) {
			if(!state.mode_console) {
				printf("> ");
				state.inputBuffer[0] = '\0';
				// ����
				fgets(state.inputBuffer, IN_BUF_LEN, stdin);
			}
			trim(state.inputBuffer);
			try {
				switch(ParseIns()) {
					case PROGRAM_EXIT: return; // �˳�
				}
			} catch(ErrorCode &e) {
				int err = e.baseErrCode;
				if(err==ERR_default)
					printf("%s %d\n", e.baseErrStr, err);
				else
					printf("%s\n%s", e.baseErrStr, e.extErrStr.c_str());
			}
			if(state.mode_console) return; // �˳�
		}
	}
	// ָ�����
	int ParseIns() {
		using namespace RDPM;
		char *arg = strtok(state.inputBuffer, " ");
		if(!arg) return PROGRAM_CONTINUE;
		if((!state.mode_colonOrder && arg[0]=='.' && !isdigit(arg[1])) || (state.mode_colonOrder && arg[0]==':')) {
			/* ָ��ģʽ */
			if(arg[1]=='q') return PROGRAM_EXIT; // �˳�
			else if(arg[1]=='h') {
				if(arg[2]=='\0') help_order(); // ���� ָ��
				else if(arg[2]=='s' && arg[3]=='\0') help_script(); // ���� �ű�
			}
			else if(arg[1]=='r' && arg[2]=='\0') eval(); // ����ű�
			else if(arg[1]=='p' && arg[2]=='l' && arg[3]=='\0') {
				// ���������ƵĿ������������
				char *t = strtok(NULL, " ");
				if(!t) throw ErrorCode(ERR_ORDER_SYNTAX);
				qrnd(atoi(t), strtok(NULL, " "));
			}
			else if(arg[1]=='m') // ����ģʽ�޸�
				for(int i=2; arg[i]!='\0'; ++i) {
					char c = arg[i];
					switch(c) {
						case 's':
							state.mode_strict = !state.mode_strict; 
							printf("%s�ϸ�ģʽ\n", state.mode_strict? "����": "�ر�");
							break;
						case 'c':
							state.mode_colonOrder = !state.mode_colonOrder;
							printf("%sð��ָ��ģʽ\n", state.mode_colonOrder? "����": "�ر�");
							break;
						case 'd':
							state.mode_debug = !state.mode_debug;
							printf("%s����ģʽ\n", state.mode_debug? "����": "�ر�");
							break;
						default:
							if(!isspace(c)) throw ErrorCode(ERR_ORDER_INVAILD);
					}
				}
			else if(arg[1]=='s' && arg[2]=='d' && arg[3]=='\0') { // ��ȡ/�������������
					char *s = strtok(NULL, " ");
					unsigned int sd;
					if(s) {
						sscanf(s, "%u", &sd);
						dice.Seed(sd);
					}
					printf("��ǰ����ֵ��%u\n", dice.Seed());
			}
			else if(arg[1]=='t' && arg[2]=='l' && arg[3]=='\0') {
				// ����ģ������
				char *mod = strtok(NULL, " ");
				if(!strcmp(mod, "osuc")) // ����ģ��osuc
					mod_osuc();
				else
					throw ErrorCode(ERR_ORDER_INVAILD);
			}
			else // ��Чָ��
				throw ErrorCode(ERR_ORDER_INVAILD);
		} else if(!state.mode_strict && (isdigit(arg[0]) || (state.mode_colonOrder && arg[0]=='.' && isdigit(arg[1])) ))
			// �������������
			qrnd(-1, arg);
		else // ��Чָ��
			throw ErrorCode(ERR_ORDER_INVAILD);

		return PROGRAM_CONTINUE;
	}
	/* ָ��� */
	// ���� ָ�� 
	void help_order() {
		puts(
			"<a> <b> <n> �� .pl <d> <a> <b> <n>\n"
			"\t�������������СֵΪa�����ֵΪb�����ɸ���Ϊn\n"
			"\tnΪ����ʱ������|n|����������ۼӣ�d��ʾ����С��λ����nλ\n"
			"\ta��bΪС�����������������\n"
			"\tע�⣺�ϸ�ģʽ�²���ֱ������ <a> <b> <n>\n"
			".r <code>\n\t��δ���á�ִ�м���ű��������ű��﷨������.hs�鿴\n"
			".m<mode>\n"
			"\t���û������ء�mode��Ϊ����ֵ��\n"
			"\t\ts\t����/�ر��ϸ�ģʽ\n\t\tc\tʹ��/ȡ��:����.��������ģʽ\n\t\td\t����/�رյ���ģʽ\n"
			".sd [seed]\n\tʡ��seed����ʾ��ǰ���������\n\tseed��unsigned int�ͣ����������������������\n"
			".tl <module> [<arg0>,[<arg1>...]]\n\t��������ģ�飨tool����moduleΪģ������argXΪ����\n\t��ǰ֧�ֵ�ģ�飺\n"
			"\tosuc <���1����>,<���1׼ȷ��>[,<���2����>,<���2׼ȷ��>...]\n\t\t����osu�����������Ȩ���㡣ע��0<=׼ȷ��<=1\n"
			".h\t�������\n.hs\t����ű��﷨����\n"
			".q\t�˳�\n"
		);
	}
	// ���� �ű�
	void help_script() {}
	// �������������
	// ������d�������� num0��һ�����ֵ��ַ���
	void qrnd(int d, char *num0) {
		using namespace RDPM;
		// ԭʼ����
		char *s[3] = {num0, strtok(NULL, " "), strtok(NULL, " ")};
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

		if(state.mode_debug)printf("[D]: %s| %s| %s| %s %f %f %d %d %s|\n",s[0],s[1],s[2],isfloat?"Float":"Int",isfloat?a.d:(double)a.i,isfloat?b.d:(double)b.i,n,d,pfarg);

		if(isfloat)
			if(n<0) printf(pfarg, dice.frs(a.d, b.d, -n), '\n');
			else for(int i=0, z=n>0? n: -n; i<z; ++i) printf(pfarg, dice.fr(a.d, b.d), i==z-1? '\n': ' ');
		else
			if(n<0) printf("%d\n", dice.irs(a.i, b.i, -n));
			else for(int i=0, z=n>0? n: -n; i<z; ++i) printf("%d%c", dice.ir(a.i, b.i), i==z-1? '\n': ' ');
	}
	// ����ģ�� osuc
	void mod_osuc() {
		// ��ʽ�� Score = osu_score * rnd(avg(ACCs), MAX_RND_RATIO)
		// .tl osuc 1122223333,0.9743,1122333344,0.9801,1122334444,0.9856
		// .tl osuc 2434779,0.9409,2538793,0.9672,2657600,1
		using namespace RDPM;
		const static double MAX_RND_RATIO = 1.05; // ��������ֵ

		std::vector<double> arg;
		char *s;
		while(s = strtok(NULL, ",")) // ������� s!=NULL��push
			arg.push_back(atof(s));
		int n = arg.size();
		if(n==0 || n%2==1) throw ErrorCode(ERR_ORDER_SYNTAX); // ��Ϊż���������򱨴�
		printf("--��ǰ��ʽ��Score = osu_score * rnd(avg(ACCs), %.2f)\n", MAX_RND_RATIO);
		// ��ƽ��ACC
		double avg_acc = 0;
		for(int i=1; i<n; i+=2) avg_acc += arg[i];
		avg_acc /= n/2.0;
		printf("--ACCƽ��ֵ��%f\n", avg_acc);
		// ������
		for(int i=0; i<n; i+=2) {
			double origin = arg[i], rnd = dice.fr(avg_acc, MAX_RND_RATIO), score = origin*rnd;
			printf("��%dλ��ҵļ�Ȩ������%f\t(ԭʼ�֣�%d, ACC��%f, Ȩ����%f)\n", i/2+1, score, (int)origin, arg[i+1], rnd);
		}
	}
	// ����ű�
	void eval() {}
public:
	UI_v1b(bool isConsole, char *arg) {
		// ��ʼ��
		state.mode_console = isConsole;
		state.mode_colonOrder = false;
		state.mode_debug = false;
		state.mode_strict = false;
		if(state.mode_console) strcpy(state.inputBuffer, arg);
		//if(state.mode_debug) printf("[D]: arg: %s|\n", state.inputBuffer);
		Start();
	}
	/* ���ߺ������� */
	static inline void clrbuf(){ // ������뻺����
		scanf("%*[^'\n']%*c");
	}
	static void trim(char *s){ // ȥ��β�հ��ַ�����������Ϊ�ַ�����
		char *p = (char *)1, *q = 0, *c;
		for (c = s; *c; ++c) if (!isspace(*c)) p = q ? p : c, q = c;
		for (c = s; p <= q; *c++ = *p++);
		*c = 0;
	} // by Edgar
};

int main(int argc, char **argv) {
	UI_v1b UI(argc>1? true: false, argv[1]);
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