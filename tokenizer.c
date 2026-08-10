#include <ctype.h>

#include "tdlib.h"

// @Cleanup make this use an arena instead.
String next_token(String *corpus)
{
    // trim left
    while ((unsigned char)corpus->size > 0
           && isspace((unsigned char)*corpus->data)) {
        corpus->data++;
        corpus->size--;
    }

    if (corpus->size == 0)
        return (String){ 0 };

    if (isalpha((unsigned char)*corpus->data)) {
        size_t n = 0;
        while (n < corpus->size && isalnum((unsigned char)corpus->data[n]))
            n++;

        String token = { 0 };
        token.data = malloc(n + 1);
        memcpy(token.data, corpus->data, n); // @Improvement add error checking
        token.data[n] = '\0';
        token.size = n;

        corpus->data += n;
        corpus->size -= n;

        return token;
    }

    if (isdigit((unsigned char)*corpus->data)) {
        size_t n = 0;

        while (n < corpus->size && isdigit((unsigned char)corpus->data[n]))
            n++;

        String token = { 0 };
        token.data = malloc(n + 1); // @Improvement add error checking
        memcpy(token.data, corpus->data, n);
        token.data[n] = '\0';
        token.size = n;

        corpus->data += n;
        corpus->size -= n;

        return token;
    }

    // for punctuation
    String token = { 0 };
    token.data = malloc(2);
    token.data[0] = *corpus->data;
    token.data[1] = '\0';
    token.size = 1;
    
    corpus->data++;
    corpus->size--;

    return token;
}

