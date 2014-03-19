
struct network {
	int socket;
};

extern struct network *nNewCommandInterface(int);
extern int nPollCommand(struct network *, struct command *);
extern void nPollEvents(struct network *);
