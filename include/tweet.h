#if !defined(__TWEET_SORTER_H__)
#define __TWEET_SORTER_H__  1

#if !defined(__uint32_t_defined)
    #include <stdint.h>
#endif

#if __GNU_C__
    #define PACKED  __attribute__((packed))
#else
    #define PACKED
#endif

#define MAX_TWEET_LENGTH     11

/* explicit Structs instead of weird Pointer Magic 
 * keeps the Code readable! */ 
struct __tweet {
    uint8_t  hits;
    uint8_t  month;
    uint8_t  day;
    char     text[MAX_TWEET_LENGTH];
    uint16_t fnum;      /* file number */
    uint32_t lnum;      /* line number */
} PACKED; 
typedef struct __tweet tweet_t;

void tweet_print(FILE *, const tweet_t *);
int tweet_compare(const tweet_t *, const tweet_t *);
#endif /* __TWEET_SORTER_H__ */
