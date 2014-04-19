/* Copyright (C) 2014 Paul Fisher ("Fishaman P")
 * 
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY,
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * The source code and binaries for this software may be freely modified and redistributed, with
 * only the following restrictions, lifted only by the express written consent of the author:
 *     - Both the copyright notice and this license must be reproduced, unmodified and in full,
 *         in all redistributed copies and derivative works.
 *     - You may not profit in any way from the redistribution of this software or any
 *         derivative works, including, but not limited to, direct sale of this software,
 *         charging for redistribution of this software, or advertising that profits
 *         specifically from this software.
 */

// This line controls how the ID combos are laid out internally.
// Changing it to a 64-bit type may increase speed on 64-bit systems. Or maybe not.
// If the size of the type is less than 32 bits on x86(-64),
// CPU instruction decode penalties will slow the program to a crawl!
#define CLUSTER unsigned int

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline unsigned int gcrng(unsigned int input)
{
    return (input * 0x343fd) + 0x269ec3;
}

int main()
{
    // This is emulating log2(sizeof(CLUSTER) * 8) since C is strict about constant expressions
    // It only works on types 8-128 bits long inclusive, but that's good enough
    // If you don't care about getting a warning, just #import <math.h> and use log2
    const int CLUSTERSHIFT = (sizeof(CLUSTER) == 1) ? 3 :
                             (sizeof(CLUSTER) == 2) ? 4 :
                             (sizeof(CLUSTER) == 4) ? 5 :
                             (sizeof(CLUSTER) == 8) ? 6 : 7;
    
    // The ID combos are laid out internally as an array of "clusters".
    // To use memory efficiently, I keep track of legal combos with a single bit each.
    // Groups of CLUSTERBITS SIDs for a given TID are stored in each cell.
    // This means the array has (0x10000 / CLUSTERBITS) cells.
    CLUSTER sidArray[0x10000 / (sizeof(CLUSTER) * 8)]; // Will always be 0x2000 bytes long
    
    register unsigned int tid; // The TID currently being processed
    register unsigned int sid; // The SID obtained from the current seed
    register unsigned int sidOffset; // The bit in a cluster representing the current SID
    register unsigned int i; // A disposable loop counter
    char filename[23] = "IllegalPairs/"; // A buffer to hold the output filename
    
    // The actual program code starts here
    
    // Make sure the user is aware of the output's huge size
    printf("WARNING: This program will create 65536 files totaling 2.5GB on this drive.\n"
         "Are you sure you want to proceed?\n"
         "Enter 1 for yes, anything else for no: ");
    if (getchar() != '1') // The user didn't enter 1
    {
        puts("\nOkay then. Try moving this program to a drive with more space.");
        return 1; // Error code for "Partial success"
    }
    
    // Try to create a folder for the program's output
    if (system("mkdir IllegalPairs")) // Check if the error code is non-zero
    {
        puts("\nERROR: Could not create folder for output!\nExiting...");
        return 100; // Error code for "Nothing succeeded"
    }
    
    // Do a loop iteration for each Trainer ID
    for (tid = 0; tid < 0x10000; tid++)
    {
        // Create the file for this TID's output
        sprintf(filename + 13, "%d", tid); // Use the current TID as the start of the filename
        FILE * outputFile = fopen(strcat(filename, ".txt"), "w"); // Create [tid].txt
        if (outputFile == NULL) // The file couldn't be created
        {
            printf("ERROR: %s could not be created.\nExiting...\n", filename);
            return 1; // Error code for "Partial success"
        }
        
        memset(sidArray, 0, 0x2000); // Reset the array to a fresh state
        
        // Do a loop iteration for each RNG seed that produces this TID
        for (i = 0; i < 0x10000; i++)
        {
            // Compute the SID generated from this seed, and mark the TID/SID combo as legal
            sid = gcrng((tid << 16) | i) >> 16;
            sidArray[sid >> CLUSTERSHIFT] |= 1 << (sid & ((sizeof(CLUSTER) * 8) - 1));
        }
        
        // Go back through the array and print each TID/SID combo that's wasn't marked valid
        // Do a loop iteration for each cell
        for (i = 0; i < (0x10000 / (sizeof(CLUSTER) * 8)); i++)
        {
            // Do a loop iteration for each Secret ID
            for (sidOffset = 0; sidOffset < (sizeof(CLUSTER) * 8); sidOffset++)
            {
                // If the bit is 0, the combination was never found, and is therefore invalid
                if (!((sidArray[i] >> sidOffset) & 1))
                {
                    fprintf(outputFile, "%d\n", ((sizeof(CLUSTER) * 8) * i) | sidOffset);
                }
            }
        }
        
        printf("\nFinished TID %d!", tid); // Update the user on the progress
        fclose(outputFile); // We're done with this file, so close it
    }
    
    puts("\nProgram successfully finished!\nExiting...");
    return 0;
}
