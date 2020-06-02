/*
*   Exercise #3
*   CS326 / Malensek
*   
*   This program reads from a given text file,
*   and then reports information from the file.
*/

#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>

int main(int argc, char *argv[])  
{   
    // Create a pointer to a new FILE
    FILE *nf;
    char line;
    char prevline;

    // Initialize all counter variables
    int line_count = 0, word_count = 0, char_count = 0;
    
    // Open read-only file
    nf = fopen(argv[1], "r");
    
    // If file cannot be opened
    if (nf == NULL)
    {
        printf("File could not be opened.");
        return 1;
    }
    else
    {
        // Loop until end of text file is reached
        while ((line = fgetc(nf)) != EOF)
        {   
            char_count++;

            // If there is a new line or space
            if (line == ' ' || line == '\n' || line == '\t')
            {
                if (isspace(prevline) == 0)
                {
                    word_count++;
                }
            }

            // If there is a new line
            if (line == '\n')
            {
                line_count++;
            }

            prevline = line;
        }

        char_count++;
        line_count++;

        // For last word
        if (isspace(prevline) == 0)
        {
            word_count++;
        }
    }

    // Print information from text file
    printf("%d\n", char_count);
    printf("%d\n", word_count);
    printf("%d\n", line_count);

    // Close reading of file
    fclose(nf);
    return 0;
}