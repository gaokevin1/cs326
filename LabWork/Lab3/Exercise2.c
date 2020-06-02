/*
*   Exercise #2
*   CS326 / Malensek
*   
*   This program takes all of the command line arguments,
*   adds each of the values up, and displays the sum in the console.
*/

#include <stdio.h> 

int main(int argc, char *argv[])  
{
    int sum = 0;

    // Iterate over all of the command line arguments
    for (int i = 1; i < argc; i++)
    {
        // Create temporary integer and set to each argument value
        int temp_num = atoi(argv[i]);

        // For every argument, add to the total sum
        sum += temp_num;
    }

    // Print total
    printf("Total: %d\n", sum);
} 