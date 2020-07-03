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
		return seed;
	}

	inline uint64_t RI() {
		// 随机整数，[0,0xFFFFFFFF]
		return x256ss.next();
	}
	inline double RF() {
		// 随机浮点数，[0,1)
		const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | RI() >> 12 };
		return u.d - 1.0;
	}

	int ir(int a, int b) {
		// 返回范围为[a,b]的随机整数
		return RI() % (b-a+1) + a;
	}
	int irs(int a, int b, int n) {
		// 返回n个范围为[a,b]的随机整数之和
		int s = 0;
		for(int i=0; i<n; ++i) s += ir(a, b);
		return s;
	}
	double fr(double a, double b) {
		// 返回范围为[a,b)的随机浮点数
		return RF() * (b-a) + a;
	}
	double frs(double a, double b, int n) {
		// 返回n个范围为[a,b)的随机浮点数之和
		double s = 0;
		for(int i=0; i<n; ++i) s += fr(a, b);
		return s;
	}

	static double pl(double a,int d) {
		//精度限制。a数字，d小数位数,d<0不限制
		if(d>=0) {
			double s=pow(10,d);
			a=(int)(a*s)/s;
		}
		return a;
	}
}; // class RNDRG

namespace RDPM { // Random Public Module
	RNDRG dice; // 随机数生成器
	// 错误码 列表
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
	// 错误码列表长度
	static const int ERROR_CODE_LIST_LEN  = sizeof(ERROR_CODE_STR)/sizeof(ERROR_CODE_STR[0]);
	// 错误码类
	struct ErrorCode {
		ERROR_CODE_LIST baseErrCode; // 基础错误码
		const char *baseErrStr; // 基础错误码信息
		std::string extErrStr; // 扩展错误码信息

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
	// 状态集合
	static const int IN_BUF_LEN = 512; // 输入缓冲区长度
	struct {
		char inputBuffer[UI_v1b::IN_BUF_LEN]; // 输入缓冲区
		bool mode_strict; // 严格模式
		bool mode_debug; // 调试模式
		bool mode_colonOrder; // 用:代替.进入命令模式
		bool mode_console; // 命令行模式（特殊）
	} state;
	// ParseIns() 退出码
	enum {
		PROGRAM_CONTINUE,
		PROGRAM_EXIT
	};
	// 入口
	void Start() {
		using namespace RDPM;
		if(!state.mode_console)
			puts("RandRange v1.5.5 - CLI Demo\n输入.h查看帮助\n程序支持命令行参数，第一个参数为执行语句，建议用双引号括起来\n");
		while(true) {
			if(!state.mode_console) {
				printf("> ");
				state.inputBuffer[0] = '\0';
				// 输入
				fgets(state.inputBuffer, IN_BUF_LEN, stdin);
			}
			trim(state.inputBuffer);
			try {
				switch(ParseIns()) {
					case PROGRAM_EXIT: return; // 退出
				}
			} catch(ErrorCode &e) {
				int err = e.baseErrCode;
				if(err==ERR_default)
					printf("%s %d\n", e.baseErrStr, err);
				else
					printf("%s\n%s", e.baseErrStr, e.extErrStr.c_str());
			}
			if(state.mode_console) return; // 退出
		}
	}
	// 指令解析
	int ParseIns() {
		using namespace RDPM;
		char *arg = strtok(state.inputBuffer, " ");
		if(!arg) return PROGRAM_CONTINUE;
		if((!state.mode_colonOrder && arg[0]=='.' && !isdigit(arg[1])) || (state.mode_colonOrder && arg[0]==':')) {
			/* 指令模式 */
			if(arg[1]=='q') return PROGRAM_EXIT; // 退出
			else if(arg[1]=='h') {
				if(arg[2]=='\0') help_order(); // 帮助 指令
				else if(arg[2]=='s' && arg[3]=='\0') help_script(); // 帮助 脚本
			}
			else if(arg[1]=='r' && arg[2]=='\0') eval(); // 计算脚本
			else if(arg[1]=='p' && arg[2]=='l' && arg[3]=='\0') {
				// 带精度限制的快速随机数生成
				char *t = strtok(NULL, " ");
				if(!t) throw ErrorCode(ERR_ORDER_SYNTAX);
				qrnd(atoi(t), strtok(NULL, " "));
			}
			else if(arg[1]=='m') // 环境模式修改
				for(int i=2; arg[i]!='\0'; ++i) {
					char c = arg[i];
					switch(c) {
						case 's':
							state.mode_strict = !state.mode_strict; 
							printf("%s严格模式\n", state.mode_strict? "开启": "关闭");
							break;
						case 'c':
							state.mode_colonOrder = !state.mode_colonOrder;
							printf("%s冒号指令模式\n", state.mode_colonOrder? "开启": "关闭");
							break;
						case 'd':
							state.mode_debug = !state.mode_debug;
							printf("%s调试模式\n", state.mode_debug? "开启": "关闭");
							break;
						default:
							if(!isspace(c)) throw ErrorCode(ERR_ORDER_INVAILD);
					}
				}
			else if(arg[1]=='s' && arg[2]=='d' && arg[3]=='\0') { // 获取/设置随机数种子
					char *s = strtok(NULL, " ");
					unsigned int sd;
					if(s) {
						sscanf(s, "%u", &sd);
						dice.Seed(sd);
					}
					printf("当前种子值：%u\n", dice.Seed());
			}
			else if(arg[1]=='t' && arg[2]=='l' && arg[3]=='\0') {
				// 内置模块运行
				char *mod = strtok(NULL, " ");
				if(!strcmp(mod, "osuc")) // 内置模块osuc
					mod_osuc();
				else
					throw ErrorCode(ERR_ORDER_INVAILD);
			}
			else // 无效指令
				throw ErrorCode(ERR_ORDER_INVAILD);
		} else if(!state.mode_strict && (isdigit(arg[0]) || (state.mode_colonOrder && arg[0]=='.' && isdigit(arg[1])) ))
			// 快速随机数生成
			qrnd(-1, arg);
		else // 无效指令
			throw ErrorCode(ERR_ORDER_INVAILD);

		return PROGRAM_CONTINUE;
	}
	/* 指令集合 */
	// 帮助 指令 
	void help_order() {
		puts(
			"<a> <b> <n> 或 .pl <d> <a> <b> <n>\n"
			"\t生成随机数。最小值为a，最大值为b，生成个数为n\n"
			"\tn为负数时则生成|n|个随机数并累加，d表示限制小数位数到n位\n"
			"\ta或b为小数则生成随机浮点数\n"
			"\t注意：严格模式下不能直接输入 <a> <b> <n>\n"
			".r <code>\n\t【未启用】执行计算脚本。完整脚本语法请输入.hs查看\n"
			".m<mode>\n"
			"\t设置环境开关。mode可为以下值：\n"
			"\t\ts\t开启/关闭严格模式\n\t\tc\t使用/取消:代替.进入命令模式\n\t\td\t开启/关闭调试模式\n"
			".sd [seed]\n\t省略seed则显示当前随机数种子\n\tseed（unsigned int型）若存在则设置随机数种子\n"
			".tl <module> [<arg0>,[<arg1>...]]\n\t运行内置模块（tool）。module为模块名，argX为参数\n\t当前支持的模块：\n"
			"\tosuc <玩家1分数>,<玩家1准确率>[,<玩家2分数>,<玩家2准确率>...]\n\t\t自制osu分数随机数加权计算。注：0<=准确率<=1\n"
			".h\t命令帮助\n.hs\t计算脚本语法帮助\n"
			".q\t退出\n"
		);
	}
	// 帮助 脚本
	void help_script() {}
	// 快速随机数生成
	// 参数：d精度限制 num0第一个数字的字符串
	void qrnd(int d, char *num0) {
		using namespace RDPM;
		// 原始参数
		char *s[3] = {num0, strtok(NULL, " "), strtok(NULL, " ")};
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

		if(state.mode_debug)printf("[D]: %s| %s| %s| %s %f %f %d %d %s|\n",s[0],s[1],s[2],isfloat?"Float":"Int",isfloat?a.d:(double)a.i,isfloat?b.d:(double)b.i,n,d,pfarg);

		if(isfloat)
			if(n<0) printf(pfarg, dice.frs(a.d, b.d, -n), '\n');
			else for(int i=0, z=n>0? n: -n; i<z; ++i) printf(pfarg, dice.fr(a.d, b.d), i==z-1? '\n': ' ');
		else
			if(n<0) printf("%d\n", dice.irs(a.i, b.i, -n));
			else for(int i=0, z=n>0? n: -n; i<z; ++i) printf("%d%c", dice.ir(a.i, b.i), i==z-1? '\n': ' ');
	}
	// 内置模块 osuc
	void mod_osuc() {
		// 公式： Score = osu_score * rnd(avg(ACCs), MAX_RND_RATIO)
		// .tl osuc 1122223333,0.9743,1122333344,0.9801,1122334444,0.9856
		// .tl osuc 2434779,0.9409,2538793,0.9672,2657600,1
		using namespace RDPM;
		const static double MAX_RND_RATIO = 1.05; // 随机数最大值

		std::vector<double> arg;
		char *s;
		while(s = strtok(NULL, ",")) // 载入参数 s!=NULL则push
			arg.push_back(atof(s));
		int n = arg.size();
		if(n==0 || n%2==1) throw ErrorCode(ERR_ORDER_SYNTAX); // 不为偶数个参数则报错
		printf("--当前公式：Score = osu_score * rnd(avg(ACCs), %.2f)\n", MAX_RND_RATIO);
		// 求平均ACC
		double avg_acc = 0;
		for(int i=1; i<n; i+=2) avg_acc += arg[i];
		avg_acc /= n/2.0;
		printf("--ACC平均值：%f\n", avg_acc);
		// 计算结果
		for(int i=0; i<n; i+=2) {
			double origin = arg[i], rnd = dice.fr(avg_acc, MAX_RND_RATIO), score = origin*rnd;
			printf("第%d位玩家的加权分数：%f\t(原始分：%d, ACC：%f, 权数：%f)\n", i/2+1, score, (int)origin, arg[i+1], rnd);
		}
	}
	// 计算脚本
	void eval() {}
public:
	UI_v1b(bool isConsole, char *arg) {
		// 初始化
		state.mode_console = isConsole;
		state.mode_colonOrder = false;
		state.mode_debug = false;
		state.mode_strict = false;
		if(state.mode_console) strcpy(state.inputBuffer, arg);
		//if(state.mode_debug) printf("[D]: arg: %s|\n", state.inputBuffer);
		Start();
	}
	/* 工具函数集合 */
	static inline void clrbuf(){ // 清空输入缓冲区
		scanf("%*[^'\n']%*c");
	}
	static void trim(char *s){ // 去首尾空白字符，参数不能为字符常量
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