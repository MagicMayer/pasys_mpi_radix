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

#define FIN "/home/vk/workspace/Beleg/twitter.data.0"
#define FOUT "/home/vk/workspace/Beleg/twitter.out2.0"

#define TSIZE 32
#define TNUM 100000

char* MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char TWEETS[TNUM*TSIZE];

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

void writeTweet(char* tweet, const int fn, const int ln, const int hits,
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

void readTweets(const char* key) {
  FILE* f = fopen(FIN, "r");
  if (f == NULL) fprintf(stderr, "open failed: %s\n", FIN);
  int i;
  char buffer[1024];
  char* tweet;
  for (i=0, tweet=TWEETS; i<TNUM; i++, tweet+=TSIZE) {
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
    writeTweet(tweet, fn, ln, hits, month, day, line);
  }
  fclose(f);
}

int compare(const void* ptr1, const void* ptr2) {
	int i;
	char* t1 = (char*) ptr1;
	char* t2 = (char*) ptr2;
	for (i=6; i<TSIZE; i++) {
		if (t1[i] > t2[i]) return -1;
		if (t2[i] > t1[i]) return 1;
	}
	return 0;
}

void writeOrderedTweets() {
  FILE* f = fopen(FOUT, "w");
  int i;
  char* tweet;
  for (i=0, tweet=TWEETS; i<TNUM; i++, tweet+=TSIZE) {
    short* fnp = (short*) tweet;
    int* lnp = (int*) (tweet+2);
    fprintf(f, "%d %d\n", *fnp, *lnp);
  }
  for (i=0, tweet=TWEETS; i<10; i++, tweet+=TSIZE) {
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
void radixSort (unsigned char *A[], int n, int d) {
	int		i, j;
	unsigned char	*B[n];

	/* Das erste Byte hat die höchste Wertigkeit.
	 */
	for (i=d-1; i>=6; i--) {

		/* A wird in B sortiert */
		countingSort (A, B, n, i);

		/* B wird zurück nach A kopiert. */
		for (j=0; j<n; j++) A[j] = B[j];
	}
	for (i = n-1; i>=n-10;i--)
	{
		printTweet(A[i]);
		printf("\n");
	}
}

int main(int argc, char** argv) {
  int i , rank, processes, localTweets, firstLocalTweet;
  unsigned char *Ap[TNUM];

  if (argc != 2) {
	  fprintf(stderr, "please specify search key\n");
	  exit(1);
  }
  readTweets(argv[1]);

  processes = 1;
  rank = 0;
  localTweets = TNUM/processes;
  firstLocalTweet = rank*localTweets;

  for (i=0; i<localTweets; i++) Ap[i] = &TWEETS[firstLocalTweet+i*TSIZE];
  radixSort(Ap, localTweets, TSIZE);
  //qsort(TWEETS, TNUM, TSIZE, compare);
  writeOrderedTweets();
}

