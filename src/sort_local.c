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
// #include <mpi.h>

#define FIN "/home/vk/workspace/Twitter/twitter.data.gross"
#define FOUT "/home/vk/workspace/Twitter/twitter.out2.0"

#define TSIZE 32
#define TNUM 240000

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

void printTweet(const char* t) {
	printf("[");
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

void readTweets(const char* key, unsigned char* localTweets, int startTweet, int numberOfLocalTweets) {
  FILE* f = fopen(FIN, "r");
  if (f == NULL) fprintf(stderr, "open failed: %s\n", FIN);
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

void writeOrderedTweets(unsigned char* localTweets, int numberOfLocalTweets) {
  FILE* f = fopen(FOUT, "w");
  int i;
  unsigned char* tweet;
  for (i=0, tweet=localTweets; i<numberOfLocalTweets; i++, tweet+=TSIZE) {
    short* fnp = (short*) tweet;
    int* lnp = (int*) (tweet+2);
    fprintf(f, "%d %d\n", *fnp, *lnp);
  }
//  for (i=0, tweet=TWEETS; i<20; i++, tweet+=TSIZE) {
//	  printTweet(tweet);
//	  printf("\n");
//  }
  for (i=0, tweet=localTweets+TSIZE*(numberOfLocalTweets-1); i<10; i++, tweet-=TSIZE) {
	  printTweet(tweet);
	  printf("\n");
  }
  fclose(f);
}

void countingSort (unsigned char *A[], unsigned char *B[], int n, int h) {
	int		C[256];
	int		i, j;

	/* Array C wird mit 0 initialisiert. */
	for (i=0; i<256; i++) C[i] = 0;
	/* Alle gleichen Byetewerte werden gezählt. */
	for (j=0; j<n; j++) C[A[j][h]]++;
	/* Alle Bytewerte werden mit der Anzahl der Vorgänger Bytewerte aufsummiert. */
	for (i=1; i<256; i++) C[i] += C[i-1];

	//for (j=0; j<n; j++) {
	for (j=n-1; j>=0; j--) {
		/*Elemente werden vom Kleinsten zum Größten eingefügt.???*/

		B[C[A[j][h]]-1] = A[j];
		C[A[j][h]]--;
	}
}
/* Dieser Redixsort sortiert n Pointer auf Strings der Größe n
 * in ein Array A.
 */
void radixSortMPI (unsigned char *A,	int *n, int d, int rank) {
	int		i, j, h;
	int		Buckets[256],C[256];
    unsigned char *B = malloc(TSIZE**n);

	/* Das erste Byte hat die höchste Wertigkeit.
	 */
	for (h=7; h>=7; h--) {

		/* COUNTING SORT A wird in B sortiert */
		/* Array C wird mit 0 initialisiert. */
		for (i=0; i<256; i++) C[i] = 0;
		/* Alle gleichen Bytewerte werden gezählt. */
		for (j=0; j<*n; j++) C[*(A+j*TSIZE+h)]++;
		memcpy(Buckets,C,sizeof(int)*256);
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

		/* B wird zurück nach A kopiert. */
		//for (j=0; j<n; j++) A[j] = B[j];
		int nextBucket = 0;
		int count = 0;
		for(int i = 0; i < 256; i++)
		{
			printf(" %d",Buckets[i]);
			count = Buckets[i] + count;
			if (count>*n/2*1.1)
			{
				printf("Send Bucket: %d bis %d , with %d Tweets.\n ",nextBucket, i, count);
				nextBucket = i+1;
				count = 0;
			}

		}
		printf("Send Bucket: %d bis %d , with %d Tweets.\n ",nextBucket, 255, count);
		memcpy(A,B,TSIZE**n);
	}
}
/* Dieser Redixsort sortiert n Pointer auf Strings der Größe n
 * in ein Array A.
 */
void radixSort (unsigned char *A,	int n, int d) {
	int		i, j, h;
	int		C[256];
    unsigned char *B = malloc(TSIZE*n*2);

	/* Das erste Byte hat die höchste Wertigkeit.
	 */
	for (h=d-1; h>=6; h--) {

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


  if (argc != 2) {
	  fprintf(stderr, "please specify search key\n");
	  exit(1);
  }

  processes = 2;
  rank = 1;
  numberOfLocalTweets = TNUM/processes;
  localTweets = (unsigned char*) malloc(TSIZE*TNUM*2);

  readTweets(argv[1],localTweets,numberOfLocalTweets*rank,numberOfLocalTweets);

  radixSortMPI(localTweets, &numberOfLocalTweets , TSIZE, rank);

  radixSort(localTweets, numberOfLocalTweets , TSIZE);
  //qsort(TWEETS, TNUM, TSIZE, compare);
  writeOrderedTweets(localTweets,numberOfLocalTweets);
}

