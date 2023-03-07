#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>

#define BUFLEN 10100
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
    printf("Digits read: %d", *numChars);
    return 0;
}

// find the longest sequence of digits that sum to the last digit whithin the sequence
void *findMaxSumSeq(void *param) {
    Threadinfo *tinfo = (Threadinfo *) param;
    const int start = tinfo->start; // start index of the buffer
    const int end = tinfo->end; // end index of the buffer
    int bestpos = tinfo->bestpos; // position of the longest sequence found so far
    int maxlen = tinfo->max; // longest sequence found so far
    char *A = tinfo->A; // buffer


    char endingNum[3]; // last two digits of the sequence
    int endingNumInt; // last two digits of the sequence as an int
    for (int i = start; i < end; i++) {

        // get the last two digits of the sequence
        strncpy(endingNum, &A[i], 2);
        endingNum[2] = '\0'; // add null terminator
        endingNumInt = strtol(endingNum, NULL, 10); // convert to int

        int sum = 0, temp = 0;
        int length = 1;

        while (sum < endingNumInt && i - length >= 0) { // check if the sum is less than the last two digits
            temp = A[i - length] - '0'; // convert char to int
            sum += temp; // add to the sum

            if (sum == endingNumInt && length > maxlen) { // check if the sum is equal to the last two digits
                bestpos = i;
                maxlen = length;
            }
            length += 1;
        }
    }

    // find the longest sequence of digits that sum to the last digit whithin the sequence
    tinfo->bestpos = bestpos;
    tinfo->max = maxlen;

    pthread_exit(NULL); // exit the thread
}

// print the results of the search
int printResults(const char *buffer, const int maxpos, const int maxlen) {
    if (maxpos >= 0) {
        printf("\nmaxlen: %d position: %d | ", maxlen + 2, maxpos);
        // +2 to account for the two digits at the end of the sequence

        printf("Longest sequence: ");
        for (int i = maxpos - maxlen; i <= maxpos + 1; i++) {
            printf("%c", buffer[i]);
        }
        int sum = 0, count = 0;
        for (int i = maxpos - maxlen; i <= maxpos - 1; i++) {
            sum += buffer[i] - '0';
            count++;
        }
        printf(" | Sanity check: (sum = %d) and (length = %d)", sum, count + 2);
        // +2 to account for ending digits
    } else {
        printf("No sequence found\n");
    }
    printf("\n");
    return 0;
}

// create threads and process results
void findSolution(int NUM_THREADS, int numChars, char *buffer, int *maxpos, int *maxlen) {
    Threadinfo tinfo[NUM_THREADS]; // array of Threadinfo structs
    pthread_t threads[NUM_THREADS]; // array of threads

    // divide buffer into regions for each thread to search
    int len = numChars / NUM_THREADS; // length of each region
    int overlap = 100; // overlap size to prevent missing the longest sequence if between regions
    for (int i = 0; i < NUM_THREADS; i++) {

        tinfo[i].A = buffer; // set the buffer for each thread
        tinfo[i].start = i * len; // set the start index for each thread
        tinfo[i].start -= (i > 0) ? overlap : 0; // adjust start index to overlap
        tinfo[i].end = (i == NUM_THREADS - 1) ? numChars : (i + 1) * len; // set the end index for each thread
        tinfo[i].end += (i < NUM_THREADS - 1) ? overlap : 0; // adjust end index to overlap
        tinfo[i].bestpos = -1;
        tinfo[i].max = 0;
    }

    // create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, findMaxSumSeq, &tinfo[i]);
    }

    // wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    // find thread that found the longest sequence
    for (int i = 0; i < NUM_THREADS; i++) {
        if (tinfo[i].max > (*maxlen)) {
            (*maxlen) = tinfo[i].max;
            (*maxpos) = tinfo[i].bestpos;
        }
    }
}

// read file and find the longest sequence of digits that sum to the last digit whithin the sequence
void runReadfileSearch(int argc, char *const *argv) {
    int rc, numChars;
    char *buffer = (char *) malloc(BUFLEN); // buffer to hold the file contents
    if (argc < 2) { // check if a filename was provided
        printf("ERROR: need a filename\n");
//        return 8;
    }
    // call readFile and check if it was successful
    rc = readFile(argv[1], &numChars, buffer);
    if (rc != 0) {
        printf("ERROR: readFile failed\n");
//        return 8;
    }


    int maxpos = 0, maxlen = 0, NUM_THREADS = 4;
    findSolution(NUM_THREADS, numChars, buffer, &maxpos, &maxlen);
    printResults(buffer, maxpos, maxlen);
    free(buffer);
}

// generate random numbers and find the longest sequence of digits that sum to the last digit whithin the sequence
void runRandomSearch(int RAND_ELEMENTS, int NUM_THREADS) {
    char *bufferRand = (char *) malloc(RAND_ELEMENTS);

    srand(time(NULL));
    for (int i = 0; i < RAND_ELEMENTS; ++i) {
        bufferRand[i] = rand() % 10 + '0';
    }

    int maxpos = 0, maxlen = 0;
    struct timeval tv1, tv2; // timers
    gettimeofday(&tv1, NULL); // start timer

    findSolution(NUM_THREADS, RAND_ELEMENTS, bufferRand, &maxpos, &maxlen);

    gettimeofday(&tv2, NULL); // end timer


//    printResults(bufferRand, maxpos, maxlen);

//    printf("NUM_THREADS = %d \t ", NUM_THREADS);

    printf("%f\n",
           (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
           (double) (tv2.tv_sec - tv1.tv_sec));

    free(bufferRand);
}

int main(int argc, char *argv[]) {
//    runReadfileSearch(argc, argv);

//    printf("--------------------\n");
//    int RAND_ELEMENTS = 1000000; // 1 million
//    printf("RAND ELEMENTS %d\n", RAND_ELEMENTS);
//    for (int i = 1; i <= 64; i += 1) {
//        int NUM_THREADS = i;
//        runRandomSearch(RAND_ELEMENTS, NUM_THREADS);
//    }

    for (int k = 1; k <= 3; k += 1) {
        printf("--------------------\n");
        int RAND_ELEMENTS = 100000000; // 100 million
        printf("RAND ELEMENTS %d\n", RAND_ELEMENTS);
        for (int i = 1; i <= 32; i += 1) {
            int NUM_THREADS = i;
            runRandomSearch(RAND_ELEMENTS, NUM_THREADS);
        }


    }


//    printf("--------------------\n");
//    RAND_ELEMENTS = 1000000000; // 1 billion
//    printf("RAND ELEMENTS %d\n", RAND_ELEMENTS);
//    for (int i = 1; i <= 16; i += 2) {
//        int NUM_THREADS = i;
//        runRandomSearch(RAND_ELEMENTS, NUM_THREADS);
//    }
    return 0;
}

