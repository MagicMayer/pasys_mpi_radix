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

int tweetfile_parse (char *fname, char *key) {
    int fd = -1, line = 0;
    char *buf = NULL, *cur_line = NULL; 
    tweet_t *tweet = NULL;
    
    ASSERT_NULL_ARG(fname, -1, EINVAL);
    ASSERT_NULL_ARG(key, -1, EINVAL);

    if((fd = tweetfile_open(fname)) <= 0) {
        return(-1);
    }

    buf = calloc(1024, sizeof(char));
    ASSERT_NULL_ARG(buf, -1, ENOMEM);

    tweet = calloc(100000, sizeof(tweet_t));
    ASSERT_NULL_ARG(tweet, -1, ENOMEM);

    while(tweetfile_readline(fd, &buf, 1024) > 0) {
        cur_line = buf;
        if(__tweetline_get_uint16(&cur_line, &tweet[line].fnum)) {
            LOG_ERROR("fnum\n");
            continue;
        }
        if(__tweetline_get_uint32(&cur_line, &tweet[line].lnum)) {
            LOG_ERROR("lnum\n");
            continue;
        }
        if(__tweetline_get_month(&cur_line, &tweet[line].month)) {
            LOG_ERROR("month\n");
            continue;
        }
        if(__tweetline_get_uint8(&cur_line, &tweet[line].day)) {
            LOG_ERROR("day\n");
            continue;
        }
        if((tweet[line].hits = tweet_count_hits(cur_line, key)) < 0) {
            continue;
        }
        memcpy(tweet[line].text, cur_line, MAX_TWEET_LENGTH - 1);
        tweet[line].text[MAX_TWEET_LENGTH] = 0;
        line++;
    }
    /* just temp */
    //qsort(tweet, 100000, sizeof(tweet_t), tweet_compare);
	int i;
	unsigned char *Ap[10000];
    for (i=0; i<10000; i++) Ap[i] = &tweet[i*sizeof(tweet_t)];
    radixSort(Ap,10000,sizeof(tweet_t));
    for(int i = 0 ; i < 10000 ; i++) {
        if(!tweet[i].hits) continue;
        tweet_print(stdout, &tweet[i]);
    }
    return(line);
}
void countingSort (unsigned char *A[], unsigned char *B[], int n, int h) {
	int		C[256];
	int		i, j;

	for (i=0; i<256; i++) C[i] = 0;
	for (j=0; j<n; j++) C[A[j][h]]++;
	for (i=1; i<256; i++) C[i] += C[i-1];
	for (j=0; j<n; j++) {

		/*Elemente werden vom kleinsten zum Größten eingefügt*/

		B[C[A[j][h]]-1] = A[j];
		C[A[j][h]]--;
	}
}

/* Dieser Redixsort sortiert n Pointer auf Strings der Größe n
 * in ein Array A.
 */
void radixSort (unsigned char *A[], int n, int d) {
	int		i, j;
	unsigned char	*B[n];

	/* Das erste Byte hat die höchste Wertigkeit.
	 */
	for (i=d-1; i>=5; i--) {

		/* A wird in B sortiert */
		countingSort (A, B, n, i);

		/* B wird zurück nach A kopiert. */

		for (j=0; j<n; j++) A[j] = B[j];
	}
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
