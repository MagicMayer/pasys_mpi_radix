#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>

#include <tweet.h>
#include <errors.h>

int8_t __tweetline_get_string(char **, char **);
int8_t __tweetline_get_uint32(char **, uint32_t *);
int8_t __tweetline_get_uint16(char **, uint16_t *);
int8_t __tweetline_get_uint8(char **, uint8_t *);
int8_t __tweetline_get_month(char **, uint8_t *);

FILE *tweetfile_open (const char *fname) {
    struct stat st;
    FILE *fd = NULL;
    ASSERT_NULL_STRING(fname, NULL, EINVAL);
    if(stat(fname, &st) == -1) {
        LOG_ERROR("<error : %s> stat(2) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(NULL);
    }
    if(!S_ISREG(st.st_mode)) {
        LOG_ERROR("<error : %s> %s is not a regular file\n", __func__, fname);
        return(NULL);
    }
    if((fd = fopen64(fname, "rex")) == NULL) {
        LOG_ERROR("<error : %s> fopen(3) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(NULL);
    }
    return(fd);
}

int tweetfile_parse (char *fname, char *key, off_t start, off_t end) {
    FILE *fd = NULL;
    char *cur_line = NULL; 
    char buffer[1024];
    int line = 0, parsed = 0;
    bool with_offset = ((start | end) > 0) ? true : false;
    tweet_t *tweet = NULL;
    
    ASSERT_NULL_ARG(fname, -1, EINVAL);
    ASSERT_NULL_ARG(key, -1, EINVAL);

    if((fd = tweetfile_open(fname)) == NULL) {
        return(-1);
    }

    if(with_offset)
        fprintf(stdout, "Reading File '%s' with Offset: start(%lu) end(%lu)\n", fname, start, end);
    
    while(fgets(buffer, sizeof(buffer), fd)) { 
        if(with_offset) {
            line++;
            if(line < start)
                continue;
            if(line > end)
                break;
        }
        cur_line = buffer;
        tweet = twcache_get_next_slot();
        ASSERT_NULL_ARG(tweet, line, ENOBUFS);
        if(__tweetline_get_uint16(&cur_line, &tweet->fnum)) 
            continue;
        if(__tweetline_get_uint32(&cur_line, &tweet->lnum))
            continue;
        if(__tweetline_get_month(&cur_line, &tweet->month))
            continue;
        if(__tweetline_get_uint8(&cur_line, &tweet->day))
            continue;
        if((tweet->hits = tweet_count_hits(cur_line, key)) < 0)
            continue;
        memcpy(tweet->text, cur_line, MAX_TWEET_LENGTH - 1);
        tweet->text[MAX_TWEET_LENGTH] = 0;
        parsed++;
        twcache_finalize_record();
    }
    fclose(fd);
    return(parsed);
}

/* Various Functions to deserialize the Strings in tweetline */
int8_t __tweetline_get_string(char **lbuf, char **string) {
    ASSERT_NULL_ARG(lbuf, -1, EINVAL);
    ASSERT_NULL_ARG(string, -1, EINVAL);
    for( ; **lbuf && ((**lbuf == ' ') || (**lbuf == '\t')) ; ++*lbuf);
    if(!**lbuf || (**lbuf == '\n'))
        return(-1);
    *string = *lbuf;
    for( ; **lbuf && (**lbuf != ' ') && (**lbuf != '\t') ; ++*lbuf);
    if(**lbuf) {
        **lbuf = '\0';
        ++*lbuf;
    }
    for( ; **lbuf && ((**lbuf == ' ') || (**lbuf == '\t')) ; ++*lbuf);
    return(0);
}

int8_t __tweetline_get_uint32(char **lbuf, uint32_t *num) {
    char __num_buf[32] = { 0 };
    char *num_ptr = __num_buf;
    if(__tweetline_get_string(lbuf, &num_ptr)) {
        *num = 0;
        return(-1);
    }
    *num = (uint32_t) strtoul(num_ptr, NULL, 10);
    return(0);
}

int8_t __tweetline_get_uint16(char **lbuf, uint16_t *num) {
    uint32_t tmp = 0;
    if(__tweetline_get_uint32(lbuf, &tmp)) {
        *num = 0;
        return(-1);
    }
    *num = (uint16_t) tmp;
    return(0);
}

int8_t __tweetline_get_uint8(char **lbuf, uint8_t *num) {
    uint32_t tmp = 0;
    if(__tweetline_get_uint32(lbuf, &tmp)) {
        *num = 0;
        return(-1);
    }
    *num = (uint8_t) tmp;
    return(0);
}

int8_t __tweetline_get_month(char **lbuf, uint8_t *month) {
    char __mbuf[4] = { 0 };
    char *m = __mbuf;

    ASSERT_NULL_ARG(lbuf, -1, EINVAL);
    ASSERT_NULL_ARG(month, -1, EINVAL);

    if(__tweetline_get_string(lbuf, &m)) {
        *month = 0;
        return(-1);
    }
    switch(m[0]) {
        case 'J':
            if(m[1] == 'a')
                *month = 1;
            else 
                *month = (m[2] == 'n') ? 6 : 7;
            break;
        case 'F':
            *month = 2;
            break;
        case 'M':
            *month = (m[2] == 'r') ? 3 : 5;
            break;
        case 'A':
            *month = (m[1] == 'p') ? 4 : 8;
            break;
        case 'S':
            *month = 9;
            break;
        case 'O':
            *month = 10;
            break;
        case 'N':
            *month = 11;
            break;
        case 'D':
            *month = 12;
            break;
        default:
            *month = 0;
            return(-1);
    }
    return(0);
}
