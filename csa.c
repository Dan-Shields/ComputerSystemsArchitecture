/*
 * Author: Daniel Shields 
 * Student ID: 9972507
*/
#include <stdio.h>
#include <string.h>
#include <math.h>

// Number of total cache blocks
#define cacheSize 256

#define addressLength 20

int run (FILE traceFile[50], char traceFileName[50], int mode) {

    // Number of words per cache block
    int cacheBlockSize = (int) pow(2, mode-1);

    // Number of cache lines
    int cacheLength = cacheSize / cacheBlockSize;

    // Declare cache array and initialise to invalid values (-1) to prevent address 0x000 returning a hit when array is supposed to be empty
    int cache[cacheSize];
    for (int i = 0; i < cacheSize; i++) {
        cache[i] = -1;
    }

    // Declare array for storing dirty bits (intially all unset)
    int dirtyFlags[cacheSize] = {0};

    // Stores block ID that should to be written to next
    int nextBlock = 0;

    int NRA = 0, NWA = 0, NCRH = 0, NCRM = 0, NCWH = 0, NCWM = 0;

    int line = 0;
    while (1) {
        char mode;
        int address;

        if (fscanf(traceFile, "%c %x\n", &mode, &address) != 2) break; // stop when end of file reached

        int hit = 0;
        int hitLocation; 
        for (int i = 0; i < cacheLength; i++) {
            if (address == cache[i]) {
                // Cache hit
                hit = 1;
                hitLocation = i; // Used in write hit operations to mark the block as dirty
                break; // Stop for-loop
            }
        }

        if (mode == 'R') {
            if (hit) {
                // Cache read hit
                NCRH++;
            } else {
                // Cache read miss
                NCRM++;

                int cacheBlockID = nextBlock++ % cacheLength; // Locate cache block for overwriting
                if (dirtyFlags[cacheBlockID]) { // If block is dirty
                    NWA++; // Write its previous data back to the lower memory
                }

                NRA++; // Read data from lower memory into the cache block
                cache[cacheBlockID] = address;

                dirtyFlags[cacheBlockID] = 0; // Mark cache block not-dirty
            }
        } else {
            if (hit) {
                // Cache write hit
                NCWH++;

                dirtyFlags[hitLocation] = 1; // Mark cache block dirty
            } else {
                // Cache write miss
                NCWM++;

                int cacheBlockID = nextBlock++ % cacheLength; // Locate cache block for overwriting
                if (dirtyFlags[cacheBlockID]) { // If block is dirty
                    NWA++; // Write its previous data back to the lower memory
                }

                NRA++; // Read data from lower memory into the cache block

                cache[cacheBlockID] = address; // Write the new data into the cache block

                dirtyFlags[cacheBlockID] = 1; // Mark cache block dirty
            }
        }

        line++;
    }
    fclose(traceFile);

    printf("%s,%d,NRA: %d, NWA: %d, NCRH: %d, RCRM: %d, NCWH: %d, NCWM: %d\n", traceFileName, mode, NRA, NWA, NCRH, NCRM, NCWH, NCWM);
}

int main (void) {
    
    FILE * testTraceFile;
    testTraceFile = fopen("test.trc","r");

    FILE * bubbleTraceFile;
    bubbleTraceFile = fopen("bubble_sort_trace_019.trc","r");

    FILE * crossCorrelationTraceFile;
    crossCorrelationTraceFile = fopen("cross_correlation_trace_019.trc","r");

    run(testTraceFile, "test.trc", 1);
    run(bubbleTraceFile, "bubble_sort_trace_019.trc", 1);
    run(crossCorrelationTraceFile, "cross_correlation_trace_019.trc", 1);
}
