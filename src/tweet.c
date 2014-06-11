#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <errors.h>
#include <tweet.h>

void tweet_print(FILE *stream, const tweet_t *t) {
    fprintf(stream, "<tweet> fnum(%d) lnum(%d) hits(%d) month(%d) day(%d) text(%s)\n",
            t->fnum, t->lnum, t->hits, t->month, t->day, t->text);
}

int tweet_compare(const tweet_t *t1, const tweet_t *t2) {
    ASSERT_NULL_ARG(t1, 0, EINVAL);
    ASSERT_NULL_ARG(t2, 0, EINVAL);
    if(t1->hits != t2->hits)    return(t1->hits - t2->hits);
    if(t1->month != t2->month)  return(t1->month - t2->month);
    if(t1->day != t2->day)      return(t1->day - t2->day);
    return(strncmp(t1->text, t2->text, MAX_TWEET_LENGTH));    
}

int tweet_count_hits(char *text, char *key) {
    register int hits = 0, len = strlen(key);
    ASSERT_NULL_STRING(text, 0, EINVAL);
    ASSERT_NULL_STRING(key, 0, EINVAL);
    for(char *tmp = text ; *tmp != 0 && (tmp = strcasestr(tmp, key)) != NULL ; tmp += len) {
        hits++;
    }
    return(hits);
}
