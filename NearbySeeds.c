/* Copyright (C) 2014-2015 Paul Fisher ("Fishaman P")
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

// The maximum amount of frame advances considered to be nearby.
#define MAX_DISTANCE 999999
 
#include <stdio.h>

static inline unsigned int gcrng(unsigned int input)
{
    return (input * 0x343fd) + 0x269ec3;
}

static unsigned int gcrngr10000(unsigned int input)
{
    register unsigned int i; // Loop counter
    
    for (i = 0; i < 10000; i++) // 10000 is the number of frames considered "very nearby"
    {
        input = (input * 0xb9b33155) + 0xa170f641; // This reverses the GCRNG equation
    }
    return input;
}

static unsigned int distanceToSeed(unsigned int startSeed, unsigned int targetSeed)
{
    register unsigned int distance;
    
    for (distance = 0; startSeed != targetSeed; distance++)
    {
        startSeed = gcrng(startSeed);
    }
    return distance;
}

static void flush()
{
    while (getchar() != '\n'); // Consume the rest of the line on the input
}

int main()
{
    unsigned int currSeed; // The RNG seed to use as a base
    unsigned int matches; // // The number of seeds that fit user requirements
    unsigned int choiceValue; // All execution paths require a single 32bit integer
    
    printf("Select from the following functionality:\n");
    printf("1. Distance to seed\n");
    printf("2. Custom TID\n");
    printf("3. Make PID shiny\n");
    printf("4. TID offset from target\n\n");
    
    matches = 0;
    do // Get the choice the user wants
    {
        printf("Enter the number for your selection: ");
        matches = scanf("%u", &choiceValue); // Matches isn't meant to be used like this, but whatever
        flush(); // Flush the input so we can check for a newline properly
    }
    while (!(matches && (choiceValue < 5) && (choiceValue != 0))); // Verify that the user entered a valid choice
    
    matches = 0;
    do // Get the current seed
    {
        if (choiceValue == 4)
        {
            printf("Target seed: 0x");
        }
        else
        {
            printf("Current seed: 0x");
        }
        matches = scanf("%x", &currSeed); // Matches isn't meant to be used like this, but whatever
        flush(); // Flush the input so we can check for a newline properly
    }
    while (!matches); // Verify that the user entered a valid seed
    
    switch (choiceValue)
    {
        case 1: // Distance to seed
        {
            unsigned int targetSeed; // The seed the user wants to hit
            
            matches = 0;
            do // Get the target seed
            {
                printf("Target seed: 0x");
                matches = scanf("%x", &targetSeed); // Matches isn't meant to be used like this, but whatever
                flush(); // Flush the input so we can check for a newline properly
            }
            while (!matches); // Verify that the user entered a valid seed
            
            matches = distanceToSeed(currSeed, targetSeed);
            if (matches == 0)
            {
                printf("Error: You are at the target seed already!\n");
            }
            else
            {
                printf("Advances needed: %u\n", matches);
            }
            break;
        }
        
        case 2: // Custom TID
        {
            unsigned int tid; // The Trainer ID the user wants to abuse for
            unsigned int distance; // The number of frame advances required
            
            matches = 0;
            do // Get the Trainer ID
            {
                printf("Trainer ID: ");
                matches = scanf("%u", &tid); // Matches isn't meant to be used like this, but whatever
                flush(); // Flush the input so we can check for a newline properly
            }
            while (!(matches && (tid < 0x10000))); // Verify that the user entered a valid TID
            
            matches = 0;
            for (distance = 1; distance < MAX_DISTANCE; distance++)
            {
                currSeed = gcrng(currSeed);
                if ((currSeed >> 16) == tid) // This seed will produce the desired TID
                {
                    printf("%u/%u: %u advances\n", tid, gcrng(currSeed) >> 16, distance);
                    matches++;
                }
            }
            
            if (matches == 0)
            {
                printf("Sorry, no nearby seeds will produce that TID.\n");
            }
            break;
        }
        
        case 3: // Make PID shiny
        {
            unsigned int pid; // The PID the user wants to make shiny
            unsigned int distance; // The number of frame advances required
            
            matches = 0;
            do // Get the PID
            {
                printf("PID: 0x");
                matches = scanf("%x", &pid); // Matches isn't meant to be used like this, but whatever
                flush(); // Flush the input so we can check for a newline properly
            }
            while (!matches); // Verify that the user entered a valid PID
            
            pid = (pid & 0x0000FFFF) ^ (pid >> 16); // xor the two halves of the PID to prep for calculations
            matches = 0;
            for (distance = 1; distance < MAX_DISTANCE; distance++)
            {
                currSeed = gcrng(currSeed);
                if (((currSeed >> 16) ^ (gcrng(currSeed) >> 16) ^ pid) < 8) // This TID/SID combo will make the PID shiny
                {
                    printf("%u/%u: %u advances\n", currSeed >> 16, gcrng(currSeed) >> 16, distance);
                    matches++;
                }
            }
            
            if (matches == 0)
            {
                printf("Sorry, no nearby seeds will make that PID shiny.\n");
            }
            break;
        }
        
        case 4: // TID offset from target
        {
            unsigned int tid; // The Trainer ID the user actually got
            int distance; // The number of frames the user was off
            
            matches = 0;
            do // Get the Trainer ID
            {
                printf("Actual Trainer ID: ");
                matches = scanf("%u", &tid); // Matches isn't meant to be used like this, but whatever
                flush(); // Flush the input so we can check for a newline properly
            }
            while (!(matches && (tid < 0x10000))); // Verify that the user entered a valid TID
            
            if ((currSeed >> 16) == tid) // The target seed produces the TID the user got
            {
                printf("Error: You got your target Trainer ID!\n");
                break;
            }
            
            matches = 0;
            currSeed = gcrngr10000(currSeed); // Rewind the RNG to find earlier seeds
            for (distance = 9999; distance > 0; distance--)
            {
                currSeed = gcrng(currSeed);
                if ((currSeed >> 16) == tid) // A possible match was found
                {
                    printf("%u/%u: %u frames early\n", tid, gcrng(currSeed) >> 16, distance);
                    matches++;
                }
            }
            currSeed = gcrng(currSeed); // Consume the RNG frame for the target seed
            for (distance = 1; distance < 10000; distance++)
            {
                currSeed = gcrng(currSeed);
                if ((currSeed >> 16) == tid) // A possible match was found
                {
                    printf("%u/%u: %u frames late\n", tid, gcrng(currSeed) >> 16, distance);
                    matches++;
                }
            }
            
            if (matches == 0)
            {
                printf("Sorry, no nearby seeds give that Trainer ID.\n");
            }
            break;
        }
    }
}
