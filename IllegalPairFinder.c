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

static inline unsigned int gcrng(unsigned int input)
{
    return (input * 0x343fd) + 0x269ec3;
}

int main()
{
    // This is emulating log2(sizeof(CLUSTER) * 8) since C is strict about constant expressions
    // It only works on primitive types 8-128 bits long inclusive, but that's good enough
    // If you don't care about getting a warning, just #import <math.h> and use log2
    const int CLUSTERSHIFT = (sizeof(CLUSTER) == 1) ? 3 :
                             (sizeof(CLUSTER) == 2) ? 4 :
                             (sizeof(CLUSTER) == 4) ? 5 :
                             (sizeof(CLUSTER) == 8) ? 6 : 7;
    
    // The ID combos are laid out internally as an array of "clusters".
    // To use memory efficiently, I keep track of legal combos with a single bit each.
    // Groups of CLUSTERBITS SIDs for a given TID are stored in each cell.
    // This means the array has (0x10000 / CLUSTERBITS) cells.
    CLUSTER sidArray[0x10000 / (sizeof(CLUSTER) * 8)] = {0}; // Implicitly initializes the array
    
    register unsigned int tid; // The TID currently being processed
    register unsigned int sid; // The SID obtained from the current seed
    register unsigned int sidOffset; // The bit in a cluster representing the current SID
    register unsigned int i; // A disposable loop counter
    
    puts("TID/SID");
    
    // Do a loop iteration for each Trainer ID
    for (tid = 0; tid < 0x10000; tid++)
    {
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
                    printf("%d/%d\n", tid, ((sizeof(CLUSTER) * 8) * i) | sidOffset);
                }
            }
        }
    }
    
    return 0;
}
