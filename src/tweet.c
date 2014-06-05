#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tweet.h>

void tweet_print(FILE *stream, const tweet_t *t) {
    fprintf(stream, "<tweet> num(%d) hits(%d) month(%d) day(%d) text(%s)\n",
            t->line, t->hits, t->month, t->day, t->text);
}

int tweet_compare(const tweet_t *t1, const tweet_t *t2) {
    if(t1->hits != t2->hits)    return(t1->hits - t2->hits);
    if(t1->month != t2->month)  return(t1->month - t2->month);
    if(t1->day != t2->day)      return(t1->day - t2->day);
    return(strncmp(t1->text, t2->text, MAX_TWEET_LENGTH));    
}
