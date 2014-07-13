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

#define MAX_TWEET_LENGTH     23

/* explicit Structs instead of weird Pointer Magic 
 * keeps the Code readable! */ 
struct __tweet {
    uint8_t  hits;
    uint8_t  month;
    uint8_t  day;
    char     text[MAX_TWEET_LENGTH + 1];
    uint16_t fnum;      /* file number */
    uint32_t lnum;      /* line number */
} PACKED; 
typedef struct __tweet tweet_t;

#define LVLHITS_BUCKETSIZE  256
#define LVLMONTH_BUCKETSIZE  12

/* Input Functions */
int tweetfile_parse(char *, char *, off_t, off_t);
uint8_t tweet_count_hits(char *, char *);

/* caching Functions */
tweet_t *twcache_get_next_slot(void);
int twcache_finalize_record(void);
void twcache_print_all(void);
void twcache_destroy(void);

/* basic Compare functions */
int tweet_compare_binary(const void *, const void *);
int tweet_compare(const void *, const void *);

/* misc helper and service Functions */
void tweet_print(FILE *, const tweet_t *);
#endif /* __TWEET_SORTER_H__ */
