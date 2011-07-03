#ifndef TABLE
#define TABLE

// This file contains functions that wrap the hash-tables of glib, to make them friendly
// to my memory managment, and replaceable --- they'll be the first with their backs
// against the wall when the revolution comes.

#include "oyster.h"
#include <glib.h>

table *make_table()
{
    table *ret = NEW(table);
    ret->it = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)oyster_unref);
    ret->ref = 0;
    ret->incref = &table_ref;
    ret->decref = &table_unref;
    return ret;
}

void *table_get(int key, table *tab, int *err)
{
    void *ret = g_hash_table_lookup(tab->it, GINT_TO_POINTER(key));
    *err = 1;
    if (ret == NULL){
        *err = 0;
    }
    return ret;
}

void table_put(int key, oyster *entry, table *tab)
{
    int *poo = GINT_TO_POINTER(key);
    incref(entry);
    g_hash_table_insert(tab->it, poo, entry);
}

int table_empty(table *tab)
{
    return g_hash_table_size(tab->it) == 0;
}

void table_ref(table *x){
    x->ref++;
}

void table_unref(table *x)
{
    x->ref--;
    if(x->ref <= 0){
        g_hash_table_unref(x->it);
        free(x);
    }
}

#endif
