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

// DO NOT MODIFY ANY OF THESE!
// They are all essentially magic numbers required for correct operation!

// The size of a cluster in bits
const int CLUSTERBITS = sizeof(CLUSTER) * 8;

// This is crude emulation of log2(CLUSTERBITS) since C is strict about constant expressions
// It only works on primitive types 8-128 bits long inclusive, but that's good enough
// If you don't care about getting a warning, just #import <math.h> and use log2
const int CLUSTERSHIFT = 3 + ((sizeof(CLUSTER) >> 1) & 3) + (sizeof(CLUSTER) >> 3) + ((sizeof(CLUSTER) >= 8) * 2);
//const int CLUSTERSHIFT = (sizeof(CLUSTER) == 1) ? 3 :
//                         (sizeof(CLUSTER) == 2) ? 4 :
//                         (sizeof(CLUSTER) == 4) ? 5 :
//                         (sizeof(CLUSTER) == 8) ? 6 : 7;

// The number of rows in the ID combo array
const int NUMROWS = 0x10000 / (sizeof(CLUSTER) * 8);

inline int gcrng(int input)
{
    return (input * 0x343fd) + 0x269ec3;
}

int main()
{
    printf("DEBUG: CLUSTERBITS = %d\n", CLUSTERBITS); // debug line
    printf("DEBUG: CLUSTERSHIFT = %d\n", CLUSTERSHIFT); // debug line
    printf("DEBUG: NUMROWS = %d\n", NUMROWS); // debug line
    puts("DEBUG: Program started."); // debug line
    
    // The ID combos are laid out internally as a 2D array.
    // To use memory efficiently, I keep track of legal combos with a single bit each.
    // I chose to store groups of CLUSTERBITS TIDs paired with a single SID for each cell.
    // This means the 2D array has (0x10000 / CUSTERBITS) rows and 0x10000 columns.
    // TODO: Is it a better idea to have 0x10000 rows and (0x10000 / CLUSTERBITS) columns?
    CLUSTER** idComboGrid = calloc(NUMROWS * 0x10000, sizeof(CLUSTER));
    
    printf("DEBUG: Memory for array allocated at %x.\n", idComboGrid); // debug line
    //puts("DEBUG: Writing first cell with 0xAAAAAAAA"); // debug line
    //idComboGrid[0][0] = 0xAAAAAAAA; // debug line
    printf("DEBUG: Cell read as: "); // debug line
    printf("%x", idComboGrid[0][0]); // debug line
    
    register unsigned int row; // The row of the array we are currently iterating through
    register unsigned int tidOffset; // The bit in a cluster representing the current TID
    register unsigned int tidHigh16; // The TID we're currently checking, stored as a High16
    register unsigned int i; // A disposable loop counter
    
    // Do a loop iteration for each row in the array (each cluster)
    for (row = 0; row < NUMROWS; row++)
    {
        // Do a loop iteration for each bit in a cluster (each TID)
        for (tidOffset = 0; tidOffset < CLUSTERBITS; tidOffset++)
        {
            // Compute the TID we're checking right now, and store it as high 16 bits
            tidHigh16 = ((row << CLUSTERSHIFT) | tidOffset) << 16;
            
            printf("DEBUG: tidHigh16 = %x\n", tidHigh16); // debug line
            
            // Do a loop iteration for each RNG seed that produces this TID
            for (i = 0; i < 0x10000; i++)
            {
                printf("DEBUG: Accessing idComboGrid[%d][%d], bit %d\n", row, gcrng(tidHigh16 | i) >> 16, (1 << tidOffset) - 1); // debug line
                // Compute the SID generated from this seed, and mark the TID/SID combo as legal
                idComboGrid[row][gcrng(tidHigh16 | i) >> 16] |= (1 << tidOffset);
            }
        }
    }
    
    puts("TID/SID");
    
    // Go back through the array and print each TID/SID combo that's wasn't marked valid
    // Do a loop iteration for each column (each SID)
    for (i = 0; i < 0x10000; i++)
    {
        for (row = 0; row < NUMROWS; row++)
        {
            for (tidOffset = 0; tidOffset < CLUSTERBITS; tidOffset++)
            {
                // If this TID/SID combo is represented by a 0 bit, it's invalid
                if (!((idComboGrid[row][i] >> tidOffset) & 1))
                    printf("%d/%d\n", (row << CLUSTERSHIFT) | tidOffset, i);
            }
        }
    }
    
    free(idComboGrid);
    return 0;
}
