#include <math.h>

#include "model.h"
#include "tdlib.h"

#define MAP_INITSZ 64
#define MAP_LOADF 0.5

static void tf_rehash(TF_Map *map, size_t needed)
{
    size_t alloc = map->alloc;

    if (alloc == 0)
        alloc = MAP_INITSZ;

    while ((f64)needed / alloc > MAP_LOADF)
        alloc *= 2;

    if (alloc == map->alloc)
        return;

    TF_Entry *old_tf = map->data;
    size_t old_alloc = map->alloc;

    TF_Entry *new_tf = calloc(alloc, sizeof(*new_tf));
    if (!new_tf)
        TD_FATAL("tf_rehash: out of memory\n");

    map->data = new_tf;
    map->alloc = alloc;

    for (size_t i = 0; i < old_alloc; ++i)
    {
        if (old_tf[i].freq <= 0)
            continue;

        /* @Todo hash cachable */
        String_View term = {
            .data = old_tf[i].term.data,
            .size = old_tf[i].term.size,
        };
        size_t index = td_sv_hash(term) % map->alloc;
        while (map->data[index].freq != 0)
            index = (index + 1) % map->alloc;
        map->data[index] = old_tf[i];
    }

    free(old_tf);
}

TF_Entry *tf_lookup(const TF_Map *map, String term)
{
    if (map->alloc == 0)
        return NULL;

    String_View term_sv = {
        .data = term.data,
        .size = term.size,
    };
    size_t start = td_sv_hash(term_sv) % map->alloc;
    size_t index = start;

    do
    {
        if (map->data[index].freq <= 0)
            return NULL;

        if (strcmp(map->data[index].term.data, term.data) == 0)
            return &map->data[index];

        index = (index + 1) % map->alloc;
    } while (index != start); /* keep probing until we wrap back to the start */

    return NULL;
}

void tf_insert(TF_Map *map, TF_Entry tf)
{
    tf_rehash(map, map->size + 1);

    String_View term = {
        .data = tf.term.data,
        .size = tf.term.size,
    };
    size_t index = td_sv_hash(term) % map->alloc;

    while (map->data[index].freq != 0)
    {
        if (strcmp(map->data[index].term.data, tf.term.data) == 0)
        {
            map->data[index].freq += tf.freq;
            return;
        }

        index = (index + 1) % map->alloc;
    }

    map->data[index] = tf;
    map->size++;
}

void docs_free(Document_Vector *docs)
{
    if (!docs || !docs->data)
        return;

    for (size_t i = 0; i < docs->size; ++i)
    {
        Document *doc = &docs->data[i];

        if (doc->path.data)
        {
            free(doc->path.data);
            doc->path.data = NULL;
        }

        if (doc->content.data)
        {
            free(doc->content.data);
            doc->content.data = NULL;
        }

        if (doc->tf.data)
        {
            for (size_t j = 0; j < doc->tf.alloc; ++j)
            {
                TF_Entry *entry = &doc->tf.data[j];
                if (entry->term.data)
                {
                    free(entry->term.data);
                    entry->term.data = NULL;
                }
            }
            free(doc->tf.data);
            doc->tf.data = NULL;
        }
    }

    free(docs->data);
    docs->data = NULL;
    docs->size = 0;
    docs->alloc = 0;
}

f64 tf_weight(String term, const Document *d)
{
    TF_Entry *entry = tf_lookup(&d->tf, term);
    if (!entry)
        return 0.0;

    return log(1 + entry->freq);
}

f64 idf_weight(String term, Document_Vector docs)
{
    size_t df = 0;
    for (size_t i = 0; i < docs.size; ++i)
    {
        if (tf_lookup(&docs.data[i].tf, term))
            df++;
    }

    return log(1 + (f64)docs.size / (1 + df));
}
