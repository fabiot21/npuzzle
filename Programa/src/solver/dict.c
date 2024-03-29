#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "dict.h"

struct elt {
    struct elt *next;
    char *key;
};

struct hashTable {
    int size;           /* size of the pointer table */
    int n;              /* number of elements stored */
    struct elt **table;
};

#define INITIAL_SIZE (1024*5000)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (0.7)

/* hash table initialization code used in both HashTableCreate and grow */
HashTable
internalHashTableCreate(int size)
{
    HashTable d;
    int i;

    d = malloc(sizeof(*d));

    assert(d != 0);

    d->size = size;
    d->n = 0;
    d->table = malloc(sizeof(struct elt *) * d->size);

    assert(d->table != 0);

    for(i = 0; i < d->size; i++) d->table[i] = 0;

    return d;
}

HashTable
HashTableCreate(void)
{
    return internalHashTableCreate(INITIAL_SIZE);
}

void
HashTableDestroy(HashTable d)
{
    int i;
    struct elt *e;
    struct elt *next;

    for(i = 0; i < d->size; i++) {
        for(e = d->table[i]; e != 0; e = next) {
            next = e->next;

            free(e->key);
            free(e);
        }
    }

    free(d->table);
    free(d);
}

#define MULTIPLIER (67)

static unsigned long
hash_function(const char *s)
{
    unsigned const char *us;
    unsigned long h;

    h = 0;

    for(us = (unsigned const char *) s; *us; us++) {
        h = h * MULTIPLIER + *us;
    }

    return h;
}

static void
grow(HashTable d)
{
    HashTable d2;            /* new hash table we'll create */
    struct hashTable swap;   /* temporary structure for brain transplant */
    int i;
    struct elt *e;

    d2 = internalHashTableCreate(d->size * GROWTH_FACTOR);

    for(i = 0; i < d->size; i++) {
        for(e = d->table[i]; e != 0; e = e->next) {
            /* note: this recopies everything */
            /* a more efficient implementation would
             * patch out the strdups inside HashTableInsert
             * to avoid this problem */
            HashTableInsert(d2, e->key);
        }
    }

    /* the hideous part */
    /* We'll swap the guts of d and d2 */
    /* then call HashTableDestroy on d2 */
    swap = *d;
    *d = *d2;
    *d2 = swap;

    HashTableDestroy(d2);
}

/* insert a new key-value pair into an existing hash table */
void
HashTableInsert(HashTable d, const char *key)
{
    struct elt *e;
    unsigned long h;

    assert(key);

    e = malloc(sizeof(*e));

    assert(e);

    e->key = strdup(key);

    h = hash_function(key) % d->size;

    e->next = d->table[h];
    d->table[h] = e;

    d->n++;

    /* grow table if there is not enough room */
    if(d->n >= d->size * MAX_LOAD_FACTOR) {
        grow(d);
    }
}

/* return true if matching key is present else false */
bool
HashTableSearch(HashTable d, const char *key)
{
    struct elt *e;
    for(e = d->table[hash_function(key) % d->size]; e != 0; e = e->next) {
        if(!strcmp(e->key, key)) {
            /* got it */

            return true;
            // return e->value;
        }
    }

    return false;
}
