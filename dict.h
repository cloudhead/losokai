//
// dict.h
//
struct entry {
    void     *value;
	uint32_t  hash;
    char      name[];
};

struct bucket {
    struct entry   *head;
    struct bucket  *tail;
};

struct dict {
    uint32_t      (*hash)(const char *, size_t);
    size_t          size;
    struct bucket  *data[];
};

typedef struct dict *dict_t;

struct dict *dict(uint32_t (*hash)(const char *k, size_t len));
void         dictFree(struct dict *d);
void        *dictLookup(struct dict *d, const char *k);
void         dictInsert(struct dict *d, const char *k, void *v);

