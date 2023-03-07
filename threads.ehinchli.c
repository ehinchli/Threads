#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define BUFLEN 10100
#define NUM_THREADS 4
#define LINELEN 256

// struct to hold the information needed by each thread
typedef struct {
    char *A;
    int start;
    int end;
    int bestpos;
    int max;
} Threadinfo;

// read the file into the buffer
int readFile(char *filename, int *numChars, char *buffer) {
    char line[LINELEN];

    // open the file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: cannot open file %s\n", filename);
        return 1;
    }

    unsigned int pos = 0; // current position in buffer
    char *chp; // pointer to the current line
    unsigned int len; // length of the current line

    chp = fgets(line, LINELEN, fp); // read the first line of file
    // check if the file is empty
    if (chp == NULL) {
        printf("file is empty\n");
        fclose(fp);
        return 8;
    }

    // read the file line by line
    while (chp != NULL) {
        len = strlen(line);

        // remove the newline character from the end of the line
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len -= 1;
        }
        strcpy(buffer + pos, line); // copy the line to the buffer
        pos += len; // increment the position in the buffer


        chp = fgets(line, LINELEN, fp); // read the next line
    }

    //remove "." from second position in buffer by shifting everything to the left
    if (buffer[1] == '.') {
        memmove(buffer + 1, buffer + 1 + 1, strlen(buffer) - 1);
    }
    // set numChars to the total number of characters read
    *numChars = strlen(buffer);
    fclose(fp);
    printf("Digits read: %d\n", *numChars);
    return 0;
}

// find the longest sequence of digits that sum to the last digit whithin the sequence
void *findMaxSumSeq(void *param) {
    Threadinfo *tinfo = (Threadinfo *) param;
    int start = tinfo->start; // start index of the buffer
    int end = tinfo->end; // end index of the buffer
    int bestpos = tinfo->bestpos; // position of the longest sequence found so far
    int maxlen = tinfo->max; // longest sequence found so far
    char *A = tinfo->A; // buffer
    int sum = 0; // sum of the current sequence
    int length = 0; // length of the current sequence
    char queue[10]; // queue to hold the digits of the current sequence

    for (int i = start; i < end; i++) { // loop through the buffer
        // if the sum is equal to the current digit and the sequence is longer than the previous longest sequence
        if (sum == A[i] - '0' && length >= maxlen) {
            maxlen = length + 1; // set the new longest sequence
            bestpos = i;
        }
        sum += A[i] - '0'; // add the current digit to the sum
        queue[length] = A[i] - '0'; // add the current digit to the queue
        length += 1; // increment the length of the queue

        // if the sum is greater than 9, remove the first digit from the queue and subtract it from the sum
        while (sum > 9) {
            sum -= queue[0]; // subtract the first digit from the sum
            for (int j = 0; j < length - 1; j++) { // shift the queue to the left
                queue[j] = queue[j + 1]; // shift the queue to the left
            }
            length -= 1;
        }
    }

    // find the longest sequence of digits that sum to the last digit whithin the sequence
    tinfo->bestpos = bestpos;
    tinfo->max = maxlen;

    pthread_exit(NULL); // exit the thread
}


int main(int argc, char *argv[]) {
    int rc, numChars;
    char *buffer = (char *) malloc(BUFLEN); // buffer to hold the file contents

    if (argc < 2) { // check if a filename was provided
        printf("ERROR: need a filename\n");
        return 8;
    }
    // call readFile and check if it was successful
    rc = readFile(argv[1], &numChars, buffer);
    if (rc != 0) {
        return 8;
    }

    Threadinfo tinfo[NUM_THREADS]; // array of Threadinfo structs
    pthread_t threads[NUM_THREADS]; // array of threads

    // divide buffer into regions for each thread to search
    int len = numChars / NUM_THREADS; // length of each region
    int overlap = 20; // overlap size to prevent missing the longest sequence if between regions
    for (int i = 0; i < NUM_THREADS; i++) {
        tinfo[i].A = buffer; // set the buffer for each thread
        tinfo[i].start = i * len; // set the start index for each thread
        tinfo[i].start -= (i > 0) ? overlap : 0; // adjust start index to overlap
        tinfo[i].end = (i == NUM_THREADS - 1) ? numChars : (i + 1) * len; // set the end index for each thread
        tinfo[i].end += (i < NUM_THREADS - 1) ? overlap : 0; // adjust end index to overlap
        tinfo[i].bestpos = -1;
        tinfo[i].max = 0;
    }
    free(buffer); // file contents are no longer needed

    // create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, findMaxSumSeq, &tinfo[i]);
    }

    // wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // find thread that found the longest sequence
    int maxpos = 0, maxlen = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (tinfo[i].max > maxlen) {
            maxlen = tinfo[i].max;
            maxpos = tinfo[i].bestpos;
        }
    }

    // print results
    if (maxpos >= 0) {
        printf("Longest sequence: ");
        // print the longest sequence
        for (int i = maxpos - maxlen + 1; i <= maxpos; i++) {
            printf("%c", tinfo[0].A[i]);
        }
        // print the position of the longest sequence
        printf(" In position %d\n", maxpos - maxlen + 1);
    } else {
        printf("No sequence found\n");
    }
    return 0;
}

