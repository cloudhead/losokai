//
// dict.c
// hash table implementation
//
// (c) 2014, Alexis Sellier
//
// TODO(cloudhead): Use dynamic array for bucket list
// TODO(cloudhead): Implement `dictRemove`
// TODO(cloudhead): Implement incremental resizing
// TODO(cloudhead): Figure out `dictInsert` semantics when key already exists
//
#include  <stdlib.h>
#include  <string.h>
#include  <stdio.h>
#include  <stdint.h>

#include  "hash.h"
#include  "dict.h"

static int bucketIndex(struct dict *d, const char *k, uint32_t *h);
static struct entry *entry(uint32_t h, const char *k, void *v);
static struct bucket *bucketInsert(struct bucket *list, struct entry *e);
static struct bucket *bucket(struct entry *head);

#define DICT_STARTING_SIZE 128
#define DICT_DEFAULT_HASH  hash

//
// Dict allocator/initializer
//
struct dict *dict(uint32_t (*hash)(const char *, size_t))
{
    size_t dsize = DICT_STARTING_SIZE * sizeof(struct bucket *);
    struct dict *d = malloc(sizeof(*d) + dsize);

    memset(d->data, 0, dsize);

    if (! hash)
        hash = DICT_DEFAULT_HASH;

    d->hash = hash;
    d->size = DICT_STARTING_SIZE;

    return d;
}

void dictFree(struct dict *d)
{
    for (int i = 0; i < d->size; i++) {
        if (d->data[i]) {
            // TODO: Free entry list
        }
    }
    free(d);
}

//
// Get value at key `k`
//
void *dictLookup(struct dict *d, const char *k)
{
    uint32_t h = 0;
    int index = bucketIndex(d, k, &h);
    struct bucket *list = d->data[index];

    while (list && list->head) {
        if (list->head->hash == h && strcmp(list->head->name, k) == 0) {
            return list->head->value;
        }
        list = list->tail;
    }
    return NULL;
}

//
// Set a value `v` for key `k` in dict `d`.
//
// If there is already a value at that index,
// append `v` to the entry list, else create a
// new one.
//
void dictInsert(struct dict *d, const char *k, void *v)
{
    uint32_t h = 0;
    int index = bucketIndex(d, k, &h);
    struct entry *e = entry(h, k, v);

    if (d->data[index]) {
        d->data[index] = bucketInsert(d->data[index], e);
    } else {
        d->data[index] = bucket(e);
    }
}

//
// Get the index of the bucket which contains key `k` and set `h` to `k`'s hash.
//
static int bucketIndex(struct dict *d, const char *k, uint32_t *h)
{
    *h = hash(k, strlen(k));
    return *h % d->size;
}

//
// Entry allocator/initializer
//
static struct entry *entry(uint32_t h, const char *k, void *v)
{
    struct entry *e = malloc(sizeof(*e) + strlen(k) + 1);
    strcpy(e->name, k);
    e->value = v;
    e->hash = h;

    return e;
}

//
// Entry list allocator/initializer
//
static struct bucket *bucket(struct entry *head)
{
    struct bucket *list = malloc(sizeof(*list));
    list->head = head;
    list->tail = NULL;
    return list;
}

//
// Insert entry `e` to the front of `list`.
//
static struct bucket *bucketInsert(struct bucket *list, struct entry *e)
{
    struct bucket *head;

    if (! list->head) { // Empty list
        list->head = e;
        return list;
    }
    head = bucket(e);
    head->tail = list;
    return head;
}

