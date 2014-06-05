#if !defined(__TWEET_SORTER_H__)
#define __TWEET_SORTER_H__  1

#if !defined(_TIME_H)
    #include <time.h>
#endif

#if __GNU_C__
    #define PACKED  __attribute__((packed))
#else
    #define PACKED
#endif

/* assume already padded to 653 Chars */
#define MAX_TWEET_LENGTH     16

struct __tweet {
    unsigned int    line;
    unsigned short  hits;
    unsigned short  month;
    unsigned short  day;
    char text[MAX_TWEET_LENGTH];
} PACKED; 
typedef struct __tweet tweet_t;

void tweet_print(FILE *, const tweet_t *);
int tweet_compare(const tweet_t *, const tweet_t *);
#endif /* __TWEET_SORTER_H__ */
