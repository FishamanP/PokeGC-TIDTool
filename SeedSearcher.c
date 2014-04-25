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

#include <stdio.h>

static inline unsigned int gcrng(unsigned int input)
{
    return (input * 0x343fd) + 0x269ec3;
}

static unsigned int gcrngr1100(unsigned int input)
{
    register unsigned int i; // Loop counter
    for (i = 0; i < 1100; i++) // 1100 is the approximate delay for actual TID/SID abuse
    {
        input = (input * 0xb9b33155) + 0xa170f641; // This reverses the GCRNG equation
    }
    
    return input;
}

static void flush()
{
    while (getchar() != '\n'); // Consume the rest of the line on the input
}

int main()
{
    unsigned int tid; // The Trainer ID the user wants
    unsigned int sid; // The Secret ID the user wants
    register unsigned int seed; // The lower half of the RNG value currently being processed
    register unsigned int matches; // The number of seeds that produce the desired TID/SID combo
    
    matches = 0;
    do // Get the TID the user wants
    {
        printf("Trainer ID: ");
        matches = scanf("%u", &tid); // Matches isn't meant to be used like this, but whatever
        flush(); // Flush the input so we can check for a newline properly
    }
    while (!(matches && (tid < 0x10000))); // Verify that the user entered a valid TID
    
    matches = 0;
    do // Get the SID the user wants
    {
        printf("Secret ID: ");
        matches = scanf("%u", &sid); // Matches isn't meant to be used like this, but whatever
        flush(); // Flush the input so we can check for a newline properly
    }
    while (!(matches && (sid < 0x10000))); // Verify that the user entered a valid SID
    
    matches = 0;
    // Do a loop iteration for each seed that produces this Trainer ID
    for (seed = tid << 16; seed < ((tid << 16) + 0x10000); seed++)
    {
        if ((gcrng(seed) >> 16) == sid) // If this seed produces the desired SID, say so
        {
            printf("\nSeed: 0x%.8x (-1100: 0x%.8x)", seed, gcrngr1100(seed));
            matches++;
        }
    }
    
    if (matches) // If we found a working seed, yay!
    {
        printf("\n\nDone!");
    }
    else // Awww, this TID/SID combo is illegal
    {
        printf("\nSorry, no matches found. That means this TID/SID combo is impossible to get.\n"
               "Check the blacklist next time.");
    }
    printf("\nPress Enter to exit...");
    getchar(); // A rudimentary pause function
    return 0;
}
