#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <errors.h>
#include <tweet.h>

void tweet_print(FILE *stream, const tweet_t *t) {
    fprintf(stream, "<tweet> fnum(%d) lnum(%d) hits(%d) month(%d) day(%d) text(%s)\n",
            t->fnum, t->lnum, t->hits, t->month, t->day, t->text);
}

int tweet_compare(const void *tw1, const void *tw2) {
    ASSERT_NULL_ARG(tw1, 0, EINVAL);
    ASSERT_NULL_ARG(tw2, 0, EINVAL);
    tweet_t *t1 = (tweet_t *) tw1;
    tweet_t *t2 = (tweet_t *) tw2;
    if(t1->hits != t2->hits)    return(t2->hits - t1->hits);
    if(t1->month != t2->month)  return(t2->month - t1->month);
    if(t1->day != t2->day)      return(t2->day - t1->day);
    return(strncmp(t2->text, t1->text, MAX_TWEET_LENGTH));    
}

int tweet_compare_binary(const void *v1, const void *v2) {
    ASSERT_NULL_ARG(v1, 0, EINVAL);
    ASSERT_NULL_ARG(v2, 0, EINVAL);
    char *t1_base = (char *) v1;
    char *t2_base = (char *) v2;
    for(unsigned i = 0 ; i < sizeof(tweet_t) ; i++) {
        if(*(t2_base + i) != *(t1_base + i)) 
            return(*(t2_base + i) - *(t1_base + i));
    }
    return(0);
}

uint8_t tweet_count_hits(char *text, char *key) {
    register int hits = 0, len = strlen(key);
    ASSERT_NULL_STRING(text, UCHAR_MAX, EINVAL);
    ASSERT_NULL_STRING(key, UCHAR_MAX, EINVAL);
    for(char *tmp = text ; *tmp != 0 && (tmp = strstr(tmp, key)) != NULL ; tmp += len) {
        hits++;
    }
    return(hits);
}
