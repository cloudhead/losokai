
#define CMD_MAX_ARGS 8
#define MAX_ARG_LEN  32

enum argtype {
	ARG_NUM,
	ARG_STR
};

struct arg {
	enum argtype t;
	union {
		float num;
		char  str[MAX_ARG_LEN];
	};
};

struct command {
	int  argc;
	char argv[CMD_MAX_ARGS][MAX_ARG_LEN];
};

extern int cmdFromString(struct command *, char *, size_t);
