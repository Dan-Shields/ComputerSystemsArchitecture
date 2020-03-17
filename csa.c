/*
 * Author: Daniel Shields 
 * Student ID: 9972507
*/
#include <stdio.h>
#include <string.h>
#include <math.h>

// Total number of cache blocks
#define cacheSize 256

// Function which takes file stream, name of the file and what cache mode to run and prints out the results for that config
int runCacheTest (FILE traceFile[50], char traceFileName[50], int cacheModeId) {

    // Number of words per cache block
    int cacheBlockSize = (int) pow(2, cacheModeId-1);

    // Number of cache blocks
    int cacheBlockCount = cacheSize / cacheBlockSize;

    // Declare cache array and initialise to invalid values (-1) to prevent address 0x000 returning a hit when array is supposed to be empty
    int cache[cacheSize];
    for (int i = 0; i < cacheSize; i++) {
        cache[i] = -1;
    }

    // Declare array for storing dirty bits (intially all unset)
    int dirtyFlags[cacheSize] = {0};

    // Stores block index that should to be written to next
    int nextBlockIndex = 0;

    // Declare counters and init to 0
    int NRA = 0, NWA = 0, NCRH = 0, NCRM = 0, NCWH = 0, NCWM = 0;

    while (1) {
        char mode;
        int address;

        // Read next line of file, stop when EoF reached
        if (fscanf(traceFile, "%c %x\n", &mode, &address) != 2) break; 
        
        // Get tag bits by shifting address right a certain number of places
        int tag = address >> (cacheModeId - 1);

        int hit = 0;
        int hitLocation;

        // Loop through cache array searching for a match
        for (int i = 0; i < cacheBlockCount; i++) {
            int foundAddress = cache[i];

            // Get tag bits by shifting address right a certain number of places
            int foundTag = foundAddress >> (cacheModeId - 1);

            if (tag == foundTag) {
                // Cache hit
                hit = 1;
                hitLocation = i; // Used in write hit operations to mark the block as dirty
                break; // Stop for-loop
            }
        }

        // Once search is complete, act based upon what happened (4 possibilities)
        if (mode == 'R') {
            if (hit) {
                // READ HIT //
                NCRH++;
            } else {
                // READ MISS //
                NCRM++;

                int cacheBlockID = nextBlockIndex++ % cacheBlockCount; // Locate cache block for overwriting
                if (dirtyFlags[cacheBlockID]) { // If block is dirty
                    NWA += cacheBlockSize; // Write its previous data back to the lower memory
                }

                NRA += cacheBlockSize; // Read data from lower memory into the cache block
                cache[cacheBlockID] = address;

                dirtyFlags[cacheBlockID] = 0; // Mark cache block not-dirty
            }
        } else if (mode == 'W') {
            if (hit) {
                // WRITE HIT //
                NCWH++;

                dirtyFlags[hitLocation] = 1; // Mark cache block dirty
            } else {
                // WRITE MISS //
                NCWM++;

                int cacheBlockID = nextBlockIndex++ % cacheBlockCount; // Locate cache block for overwriting
                if (dirtyFlags[cacheBlockID]) { // If block is dirty
                    NWA += cacheBlockSize; // Write its previous data back to the lower memory
                }

                NRA += cacheBlockSize; // Read data from lower memory into the cache block

                cache[cacheBlockID] = address; // Write the new data into the cache block

                dirtyFlags[cacheBlockID] = 1; // Mark cache block dirty
            }
        }
    }

    // CSV print
    printf("%s,%d,%d,%d,%d,%d,%d,%d\n", traceFileName, cacheModeId, NRA, NWA, NCRH, NCRM, NCWH, NCWM);

    // Pretty print
    //printf("%s       \t\t%d\t\tNRA: %d    \t\t NWA: %d\t\t NCRH: %d\t\t RCRM: %d\t\t NCWH: %d\t\t NCWM: %d\n", traceFileName, cacheModeId, NRA, NWA, NCRH, NCRM, NCWH, NCWM);
}

int main (void) {

    FILE * bubbleTraceFile;
    bubbleTraceFile = fopen("bubble_sort_trace_019.trc","r");

    FILE * crossCorrelationTraceFile;
    crossCorrelationTraceFile = fopen("cross_correlation_trace_019.trc","r");

    for (int i = 1; i <= 8; i++) {
        runCacheTest(bubbleTraceFile, "bubble_sort_trace_019.trc", i);
        fseek(bubbleTraceFile, 0, SEEK_SET); // Reset file pointer to start of file
    }

    for (int i = 1; i <= 8; i++) {
        runCacheTest(crossCorrelationTraceFile, "cross_correlation_trace_019.trc", i);
        fseek(crossCorrelationTraceFile, 0, SEEK_SET); // Reset file pointer to start of file
    }

    fclose(bubbleTraceFile);
    fclose(crossCorrelationTraceFile);
}
