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

int tweetfile_open (const char *fname) {
    struct stat st;
    int fd = -1;
    
    if(!fname) {
        errno = EINVAL;
        return(-1);
    }
    
    if(stat(fname, &st) == -1) {
        fprintf(stderr, "<error : %s> stat(2) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(-1);
    }

    if(!S_ISREG(st.st_mode)) {
        fprintf(stderr, "<error : %s> %s is not a regular file\n", __func__, fname);
        return(-1);
    }

    if((fd = open(fname, O_CLOEXEC | O_NONBLOCK)) == -1) {
        fprintf(stderr, "<error : %s> open(2) failed: (%d) %s\n", __func__, errno, strerror(errno));
        return(-1);
    }

    return(fd);
}

size_t tweetfile_readline(int *fd, char *buffer, size_t buf_bytes) {
    size_t bytes = 0;
    unsigned char lb; /* local buffer */
    do {
        if(read(*fd, &lb, 1) <= 0)
            break;
        if((bytes+1) >= buf_bytes || lb == '\n') {
            buffer[(bytes + 1)] = '\0';
            break;
        }
        buffer[bytes++] = lb;
    } while(1);
    return(bytes);
}

int tweetfile_parse (const char *fname) {
    int fd = -1, line = 0, bytes = 0;
    char buf[1024] = { 0 };
    
    if(!fname) {
        errno = EINVAL;
        return(-1);
    }

    fd = tweetfile_open(fname);

    for(int i = 0 ; i < 4 ; i++) {
        bytes = tweetfile_readline(&fd, buf, 1024);
        printf("%d (%dB): %s\n", line++, bytes, buf);
    }
//     while(tweet_readline(&fd, buf, 1024) > 0) { 
//         printf("%d: %s", line, buf);
//         line++;
//     }
    
    return(0);
}
