#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cache.h"

CacheEntry *cache = NULL;
int cache_capacity = 0;

void init_cache(int size) {
    cache_capacity = size;
    cache = (CacheEntry *) malloc(sizeof(CacheEntry) * cache_capacity);
    if (!cache) {
        perror("Erro ao alocar memória para cache");
        exit(1);
    }

    for (int i = 0; i < cache_capacity; i++) {
        cache[i].valid = 0;
        cache[i].key[0] = '\0';
        cache[i].content[0] = '\0';
        cache[i].last_access = 0;
    }
}

void free_cache() {
    if (cache) {
        free(cache);
        cache = NULL;
        cache_capacity = 0;
    }
}


const char* cache_get(const char *key) {
    for (int i = 0; i < cache_capacity; i++) {
        if (cache[i].valid && strcmp(cache[i].key, key) == 0) {
            cache[i].last_access = time(NULL);  // Atualiza o timestamp!
            return cache[i].content;
        }
    }
    return NULL;
}



void cache_put(const char *key, const char *content) {
    time_t now = time(NULL);
    int updated = 0;

    // Atualiza se já existe
    for (int i = 0; i < cache_capacity; i++) {
        if (cache[i].valid && strcmp(cache[i].key, key) == 0) {
            strncpy(cache[i].content, content, sizeof(cache[i].content));
            cache[i].last_access = now;
            updated = 1;
            break;
        }
    }

    // Tenta inserir se houver espaço livre
    if (!updated) {
        for (int i = 0; i < cache_capacity; i++) {
            if (!cache[i].valid) {
                cache[i].valid = 1;
                strncpy(cache[i].key, key, sizeof(cache[i].key));
                strncpy(cache[i].content, content, sizeof(cache[i].content));
                cache[i].last_access = now;
                updated = 1;
                break;
            }
        }
    }

    // Substitui o mais antigo se necessário
    if (!updated) {
        int oldest_index = 0;
        time_t oldest_time = cache[0].last_access;
        for (int i = 1; i < cache_capacity; i++) {
            if (cache[i].last_access < oldest_time) {
                oldest_index = i;
                oldest_time = cache[i].last_access;
            }
        }

        strncpy(cache[oldest_index].key, key, sizeof(cache[oldest_index].key));
        strncpy(cache[oldest_index].content, content, sizeof(cache[oldest_index].content));
        cache[oldest_index].last_access = now;
        cache[oldest_index].valid = 1;
    }

}


void cache_invalidate(const char* key) {
    for (int i = 0; i < cache_capacity; i++) {
        if (cache[i].valid && strcmp(cache[i].key, key) == 0) {
            cache[i].valid = 0;
            cache[i].key[0] = '\0';
            cache[i].content[0] = '\0';
            return;
        }
    }
}

void cache_invalidate_related(const char *doc_key) {
    for (int i = 0; i < cache_capacity; i++) {
        if (cache[i].valid && strstr(cache[i].content, doc_key) != NULL) {
            cache[i].valid = 0;
            cache[i].key[0] = '\0';
            cache[i].content[0] = '\0';
        }
    }
}

void invalidate_cache_by_doc_id(int doc_id) {
    char pattern[16];
    snprintf(pattern, sizeof(pattern), "%d", doc_id);

    for (int i = 0; i < cache_capacity; i++) {
        if (!cache[i].valid) continue;

        // Evitar falso positivo: verifica se é número isolado
        char *found = cache[i].content;
        while ((found = strstr(found, pattern)) != NULL) {
            char before = (found == cache[i].content) ? '[' : *(found - 1);
            char after = *(found + strlen(pattern));

            if ((before == '[' || before == ',' || before == ' ') &&
                (after == ',' || after == ']' || after == ' ' || after == '\n' || after == '\0')) {
                cache[i].valid = 0;
                break; // Não precisas procurar mais nessa entrada
            }

            found += strlen(pattern);
        }
    }
}

