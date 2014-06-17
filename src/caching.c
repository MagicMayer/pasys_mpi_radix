#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <tweet.h>
#include <errors.h>

#define INIT_CHUNK_SIZE     1000000
#define REALLOC_CHUNK_SIZE  1000000

uint64_t _elems = 0;
uint64_t _alloc_elems = 0;
tweet_t *_tw_cache = NULL;

tweet_t *_tw_maxelem = NULL;
tweet_t *_tw_current = NULL;

int __tweetcache_init(uint32_t size) {
    if(size == 0) {
        return(-1);
    }
    if(_tw_cache)
        return(0);
    _tw_cache = calloc(size, sizeof(tweet_t));
    ASSERT_NULL_ARG(_tw_cache, -1, ENOMEM);
    _elems = 0;
    _alloc_elems = size;
    _tw_maxelem = _tw_current = _tw_cache;
    return(0);
}

tweet_t *twcache_get_next_slot(void) {
    if(!_tw_cache || _alloc_elems == 0) {
        if(__tweetcache_init(INIT_CHUNK_SIZE))
            return(NULL);
    } else if((_tw_current - _tw_cache) >= (_alloc_elems - 1)) {
        fprintf(stderr, "%s: _alloc_elems(%lu) _elems(%lu)\n", __func__, _alloc_elems, _elems);
        ptrdiff_t twmax_offset = _tw_maxelem - _tw_cache;
        _alloc_elems += REALLOC_CHUNK_SIZE;
        tweet_t *tw_tmp = realloc(_tw_cache, sizeof(tweet_t) * _alloc_elems); 
        ASSERT_NULL_ARG(tw_tmp, NULL, ENOMEM);
        _tw_cache = tw_tmp;
        _tw_current = (_tw_cache + _elems); 
        _tw_maxelem = (_tw_cache + twmax_offset);
    }
    _tw_current++;
    return(_tw_current);
}

int twcache_finalize_record(void) {
    if(!_tw_cache || !_tw_current)
        return(-1);
    if(tweet_compare(_tw_maxelem, _tw_current) > 0) {
        _tw_maxelem = _tw_current;
    }
    _elems++;
    return(0);
}

void twcache_print_all(void) {
#if DEBUG
    for(int i = 0 ; i <= _elems ; i++) {
        tweet_print(stdout, (_tw_cache+i));
    }
#endif
    fprintf(stdout, "Max Tweet: \n");
    tweet_print(stdout, _tw_maxelem);
    fprintf(stdout, "Elements in Cache: %lu / Allocated Elements: %lu\n", _elems, _alloc_elems);
}
