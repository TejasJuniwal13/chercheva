#ifndef MODEL_H
#define MODEL_H

#include "tdlib.h"

typedef struct {
    String term;
    s64 freq; /* occupied when freq > 0 */
} TF_Entry;

typedef struct {
    TF_Entry *data;
    size_t size, alloc;
} TF_Map;

typedef struct {
    String path;
    String content;
    TF_Map tf;
} Document;

typedef struct {
    Document *data;
    size_t size, alloc;
} Document_Vector;

TF_Entry *tf_lookup(const TF_Map *map, String term);
void tf_insert(TF_Map *map, TF_Entry tf);
void docs_free(Document_Vector *docs);
f64 tf_weight(String term, const Document *d);
f64 idf_weight(String term, Document_Vector docs);

#endif // MODEL_H
