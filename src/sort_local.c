/*
   ============================================================================
Name        : sort_local.c
Author      : 
Version     :
Copyright   : (c) A Fortenbacher
Description : local sort - tweets
============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mpi.h>

#define BASEPATH    "/mpidata"
#define FIN         BASEPATH "/parsys14/gross/twitter.data"
#define FOUT        BASEPATH "/ergebnisse/g15_twitter.data.out"

#define MAXPATHLEN 1024
#define TSIZE 32
#define TNUM 2400000

char* MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int readNumber(char** lptr) {
    char* ptr = *lptr;
    char* line = *lptr;
    while (*ptr != ' ') ptr++;
    *ptr = 0;
    *lptr = ptr+1;
    return atoi(line);
}

int readMonth(char** lptr) {
    char* ptr = *lptr;
    char* line = *lptr;
    while (*ptr != ' ') ptr++;
    *ptr = 0;
    *lptr = ptr+1;
    int i, m;
    for (i=0, m=1; i<12; i++, m++)
        if (strncmp(line, MONTHS[i], 3) == 0)
            return m;
    fprintf(stderr, "invalid month: %s\n", line);
    exit(3);
}

int countHits(const char* line, const char* key) {
    int n = strlen(key);
    int k = strlen(line) - n;
    int i;
    int hits = 0;
    for (i=0; i<k; i++, line++)
        if (*line == *key)
            if (strncmp(line, key, n) == 0)
                hits++;
    return hits;
}

void printTweet(const char* t, int rank) {
    printf("[Rank%d ",rank);
    int i;
    for (i=0; i<TSIZE; i++) {
        int k = t[i];
        printf("%2x ", k<0?k+256:k);
    }
    printf("]");
}

void writeTweet(unsigned char* tweet, const int fn, const int ln, const int hits,
        const int month, const int day, char* line) {
    short* ptr1 = (short*) tweet;
    *ptr1 = (short) fn;
    int* ptr2 = (int*) (tweet+2);
    *ptr2 = ln;
    *(tweet+6) = (char) hits;
    *(tweet+7) = (char) month;
    *(tweet+8) = (char) day;
    int i;
    int n = TSIZE-9;
    for (i=strlen(line); i<n; i++) line[i] = ' '; // padding
    memcpy(tweet+9, line, n);
    //  printTweet(tweet); printf("\n");
}

FILE *open_tweetfile(char *pattern, char *perms) {
    int _f_rank = -1, _f_size;
    char _f_buf[MAXPATHLEN + 1];
    FILE *_f = NULL;
    MPI_Comm_rank(MPI_COMM_WORLD, &_f_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_f_size);
    if(!pattern || !*pattern || !perms || !*perms || _f_rank < 0 || _f_rank > 16) {
        return NULL;
    }
    snprintf(_f_buf, MAXPATHLEN, "%s.%d", pattern, _f_rank);
    fprintf(stderr, "Node(%d) %s File: %s\n", _f_rank, (*perms == 'w') ? "writing" : "reading",  _f_buf);
    if(*perms == 'w' && unlink(_f_buf)) { /* delete old .out Files */
        fprintf(stderr, "unlink(2) %s failed: (%d) %s\n", _f_buf, errno, strerror(errno));
    }
    _f = fopen(_f_buf, perms);
    if(_f == NULL) {
        fprintf(stderr, "fopen(3) %s failed: (%d) %s\n", _f_buf, errno, strerror(errno));
    }
    return(_f);
}

void readTweets(const char* key, unsigned char* localTweets, int startTweet, int numberOfLocalTweets) {
    FILE *f = open_tweetfile(FIN, "r");
    if (f == NULL) {
        exit(1);
    }
    int i;
    char buffer[1024];
    unsigned char* tweet;

    for (i=0; i<startTweet; i++) {
        char* line = fgets(buffer, 1024, f);
        if (line == NULL) {
            fprintf(stderr, "error reading line %d\n", i);
            exit(2);
        }
    }
    for (i=startTweet, tweet=localTweets; i<startTweet+numberOfLocalTweets; i++, tweet+=TSIZE) {
        char* line = fgets(buffer, 1024, f);
        if (line == NULL) {
            fprintf(stderr, "error reading line %d\n", i);
            exit(2);
        }
        int fn = readNumber(&line);
        int ln = readNumber(&line);
        int month = readMonth(&line);
        int day = readNumber(&line);
        int hits = countHits(line, key);
        month = (hits << 4) + month;
        writeTweet(tweet, fn, ln, hits, month, day, line);
    }
    fclose(f);
}

int compare(const void* ptr1, const void* ptr2) {
    int i;
    unsigned char* t1 = (unsigned char*) ptr1;
    unsigned char* t2 = (unsigned char*) ptr2;
    for (i=6; i<TSIZE; i++) {
        if (t1[i] > t2[i]) return -1;
        if (t2[i] > t1[i]) return 1;
    }
    return 0;
}

void writeOrderedTweets(unsigned char* localTweets, int numberOfLocalTweets, int rank) {
    int i;
    unsigned char* tweet;
    FILE *f = open_tweetfile(FOUT, "w");
    if(!f) return;
    for (i = numberOfLocalTweets , tweet = localTweets + (numberOfLocalTweets * TSIZE - 1) ; i>=0 ; i-- , tweet-=TSIZE) {
        short* fnp = (short*) tweet;
        int* lnp = (int*) (tweet+2);
        fprintf(f, "%d %d\n", *fnp, *lnp);
    }
    for (i=0, tweet=localTweets+TSIZE*(numberOfLocalTweets-1); i<10; i++, tweet-=TSIZE) {
        printTweet(tweet, rank);
        printf("\n");
    }
    fclose(f);
}

/* Dieser Redixsort sortiert n Pointer auf Strings der Größe n
 * in ein Array A.
 */
void radixSortMPI (unsigned char *A, int *n, int d, int rank, int processes) {
    int		i, j, h;
    int		localBuckets[256],C[256];
    int    *globalBuckets = malloc(sizeof(int)*processes*256);
    int    *processBuckets = malloc(sizeof(int)*processes);
    unsigned char *B = malloc(TSIZE**n*1.2);

    MPI_Request req;
    MPI_Status stat;

    /* Das erste Byte hat die höchste Wertigkeit.
    */
    for (h=7; h>=7; h--) {

        /* COUNTING SORT A wird in B sortiert */
        /* Array C wird mit 0 initialisiert. */
        for (i=0; i<256; i++) {
            C[i] = 0;
        }
        /* Alle gleichen Bytewerte werden gezählt. */
        for (j=0; j<*n; j++) C[*(A+j*TSIZE+h)]++;
        memcpy(localBuckets,C,sizeof(int)*256);
        /* Alle Bytewerte werden mit der Anzahl der Vorgänger Bytewerte aufsummiert. */
        for (i=1; i<256; i++) C[i] += C[i-1];
        //for (j=0; j<n; j++) {
        for (j=*n-1; j>=0; j--) {
            /*Elemente werden vom Kleinsten zum Größten eingefügt.???*/

            int currentPosition = C[*(A+j*TSIZE+h)];
            //printf("Current Character: %d,Position: %d\n",*(A+j*TSIZE+h),currentPosition);
            memcpy(B+(currentPosition-1)*TSIZE,A+j*TSIZE,TSIZE);
            C[*(A+j*TSIZE+h)]--;
        }
        for (int p = 0; p < processes; p++) {
            if (p != rank) {
                MPI_Send(localBuckets, 256, MPI_INT, p, 0, MPI_COMM_WORLD);
            }
        }
        for (int p = 0; p < processes; p++) {
            if (p != rank) {
                MPI_Recv(C, 256, MPI_INT, p, 0, MPI_COMM_WORLD, &stat);
                for (int i = 0; i < 256; i++) {
                    *(globalBuckets + p*256 + i) = C[i];
                }
            }
            else{
                for (int i = 0; i < 256; i++) {
                    *(globalBuckets + p*256 + i) = localBuckets[i];
                }
            }
        }

        int nextBucket = 0;
        int globalCount = 0;
        int localCount = 0;
        int targetProcess = 0;
        unsigned char* currentBucket = B;
        unsigned char* localBucket = B;

        for (i= 0; i < processes;i++) {
            *(processBuckets + i) = 0;
        }

        for(i = 0; i < 256; i++) {
            //Todo evtl prozesse rausnehmen
            for(int j = 0; j < processes; j++) {
                globalCount = *(globalBuckets +j*256 + i) + globalCount;
            }
            localCount = localCount + localBuckets[i];
            if (globalCount>*n*1.1|| i == 255) {
                *(processBuckets + targetProcess) = i-nextBucket+1;//Anzahl der Buckets im Prozess
                if(rank!=targetProcess) {
                    printf("Rank%d sends Bucket: %d bis %d (%d Buckets), with %d Tweets to %d\n ",rank,nextBucket, 
                            i, *(processBuckets + targetProcess), localCount,targetProcess);
                    MPI_Isend(currentBucket, localCount*TSIZE, MPI_CHAR, targetProcess, 0, MPI_COMM_WORLD, &req);
                } else {
                    localBucket = currentBucket;
                }
                currentBucket = currentBucket+localCount*TSIZE;
                targetProcess += 1;
                nextBucket = i+1;
                globalCount = 0;
                localCount = 0;
            }
        }

        currentBucket = A;
        for (int p = 0; p < processes; p++) {
            int receiveSize = 0;
            nextBucket = 0;
            for(i = 0; i<rank;i++) {
                nextBucket += *(processBuckets+i);
            }
            for(i = 0; i<*(processBuckets+rank); i++) {
                receiveSize += *(globalBuckets + p*256 + nextBucket + i);
            }
            printf("Rank%d receives %d Buckets from %d  pos %d Receive Size %d\n",rank, 
                    *(processBuckets+rank),p,nextBucket,receiveSize);
            nextBucket = 0;
            if (p != rank && receiveSize > 0) {
                /* Segfault here when running with multiple nodes ! */
                MPI_Recv(currentBucket, receiveSize*TSIZE, MPI_CHAR, p, 0, MPI_COMM_WORLD, &stat);
            } else {
                memcpy(currentBucket,localBucket,receiveSize*TSIZE);
            }
            printf("Rank %d CurrentBucket %p receive size %d \n",rank,currentBucket,receiveSize);
            currentBucket += receiveSize*TSIZE;
        }
        // memcpy(A,B,TSIZE**n);
    }
}
    /* Dieser Radixsort sortiert n Pointer auf Strings der Größe n
     * in ein Array A.
     */
void radixSort (unsigned char *A,	int n, int d) {
    int		i, j, h;
    int		C[256];
    unsigned char *B = malloc(TSIZE*n*1.2);

    /* Das erste Byte hat die höchste Wertigkeit.
    */
    for (h=d-1; h>=7; h--) {

        /* COUNTING SORT A wird in B sortiert */
        /* Array C wird mit 0 initialisiert. */
        for (i=0; i<256; i++) C[i] = 0;
        /* Alle gleichen Bytewerte werden gezählt. */
        for (j=0; j<n; j++) C[*(A+j*TSIZE+h)]++;
        /* Alle Bytewerte werden mit der Anzahl der Vorgänger Bytewerte aufsummiert. */
        for (i=1; i<256; i++) C[i] += C[i-1];

        //for (j=0; j<n; j++) {
        for (j=n-1; j>=0; j--) {
            /*Elemente werden vom Kleinsten zum Größten eingefügt.???*/

            int currentPosition = C[*(A+j*TSIZE+h)];
            //printf("Current Character: %d,Position: %d\n",*(A+j*TSIZE+h),currentPosition);
            memcpy(B+(currentPosition-1)*TSIZE,A+j*TSIZE,TSIZE);
            C[*(A+j*TSIZE+h)]--;
        }

        /* B wird zurück nach A kopiert. */
        //for (j=0; j<n; j++) A[j] = B[j];
        memcpy(A,B,TSIZE*n);
    }
}

int main(int argc, char** argv) {
    int i , rank, processes, numberOfLocalTweets;
    unsigned char *localTweets;
    double r_time = 0, s_time = 0, o_time = 0, start = 0, end = 0;

    // initialize MPI environment and obtain basic info
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    if (argc != 2) {
        fprintf(stderr, "please specify search key\n");
        exit(1);
    }

    numberOfLocalTweets = TNUM / processes;
    localTweets = (unsigned char*) malloc(TSIZE*TNUM*1.2);

    start = MPI_Wtime();
    readTweets(argv[1],localTweets,numberOfLocalTweets*rank,numberOfLocalTweets);
    MPI_Barrier(MPI_COMM_WORLD);
    r_time = MPI_Wtime();

    radixSortMPI(localTweets, &numberOfLocalTweets , TSIZE, rank, processes);
    radixSort(localTweets, numberOfLocalTweets , TSIZE);
    MPI_Barrier(MPI_COMM_WORLD);
    s_time = MPI_Wtime();

    writeOrderedTweets(localTweets,numberOfLocalTweets,rank);
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    if(rank == 0) {
        fprintf(stderr, "TIME: total(%02.4f) read_time(%02.4f)\n", (r_time - start), (r_time - start));
        fprintf(stderr, "TIME: total(%02.4f) sort_time(%02.4f)\n", (s_time - start), (s_time - r_time)); 
        fprintf(stderr, "TIME: total(%02.4f) output_time(%02.4f)\n", (end - start), (end - s_time)); 
    }
    MPI_Finalize();
}

