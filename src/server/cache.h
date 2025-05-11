#ifndef CACHE_H
#define CACHE_H

#include <time.h>

typedef struct {
    char key[64];
    char content[1024];
    int valid;         
    time_t last_access; 
} CacheEntry;

extern CacheEntry *cache;
extern int cache_capacity;

void init_cache(int size);
void free_cache();
const char* cache_get(const char *key, const char *part_key);
void cache_put(const char *key, const char *part_key, const char *content);
void cache_invalidate(const char *key, const char *part_key);
void cache_invalidate_related(const char *doc_key, const char *part_key);
void invalidate_cache_by_doc_id(int doc_id);


#endif
