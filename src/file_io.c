#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int tweetfile_open (const char *fname) {
    struct stat st;
    int fd = -1;
    ASSERT_NULL_ARG(fname, -1, EINVAL);
    if(stat(fname, &st) == -1) {
        LOG_ERROR("<error : %s> stat(2) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(-1);
    }
    if(!S_ISREG(st.st_mode)) {
        LOG_ERROR("<error : %s> %s is not a regular file\n", __func__, fname);
        return(-1);
    }
    if((fd = open(fname, O_CLOEXEC | O_NONBLOCK)) == -1) {
        LOG_ERROR("<error : %s> open(2) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(-1);
    }
    return(fd);
}

size_t tweetfile_readline(int fd, char **buffer, size_t bsize) {
    char *p;
    unsigned char lb = 0; 
    ASSERT_NULL_ARG(buffer, -1, EINVAL);
    p = *buffer;
    while(read(fd, &lb, 1) > 0) {
        if((p - *buffer) >= bsize || lb == '\n') {
            *(p++) = 0;
            break;
        }
        *(p++) = (char) lb;
    }
    return((size_t) (p - *buffer));
}

int tweetfile_parse (const char *fname) {
    int fd = -1, line = 0;
    char *buf = NULL, *cur_line = NULL; 
    tweet_t tweet;
    
    ASSERT_NULL_ARG(fname, -1, EINVAL);
    if((fd = tweetfile_open(fname)) <= 0) {
        return(-1);
    }

    buf = calloc(1024, sizeof(char));
    ASSERT_NULL_ARG(buf, -1, ENOMEM);

    while(tweetfile_readline(fd, &buf, 1024) > 0) {
        cur_line = buf;
        line++;
        if(__tweetline_get_uint16(&cur_line, &tweet.fnum)) {
            LOG_ERROR("fnum\n");
            continue;
        }
        if(__tweetline_get_uint32(&cur_line, &tweet.lnum)) {
            LOG_ERROR("lnum\n");
            continue;
        }
        if(__tweetline_get_month(&cur_line, &tweet.month)) {
            LOG_ERROR("month\n");
            continue;
        }
        if(__tweetline_get_uint8(&cur_line, &tweet.day)) {
            LOG_ERROR("day\n");
            continue;
        }
        memcpy(&tweet.text, cur_line, MAX_TWEET_LENGTH - 1);
        tweet.text[MAX_TWEET_LENGTH] = 0;
        if((tweet.hits = tweet_count_hits(cur_line, "YOU")) > 0)
            tweet_print(stdout, &tweet);
    }
    return(0);
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
