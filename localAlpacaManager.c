#include <stdio.h> // Standard input/output
#include <sys/stat.h> // For directory handling 
#include <sys/types.h> // For type definitions
#include <string.h> // For string operations
#include "contextStructs.h" // Custom header file 

// Directory creation (OS specific)
#ifdef _WIN32
    #include <windows.h> // Directory creation in Windows
    #define MKDIR(path) _mkdir(path)  
#else
    #include <unistd.h> // Directory creation in Linux/macOS
    #define MKDIR(path) mkdir(path, 0777)
#endif

// Maximum number of directories (10) and maximum content length for a file (5000)
#define MAX_DIR 10 
#define MAX_CONTXT_CHAR 5000

int main() {
    int whatDo; // --> Variable to store user choice
    struct contextDir dirArr[MAX_DIR]; // Array of directory structures 

    do { 
        // Prompt user for an action 
        printf("Enter 1 to create a category, enter 2 to create a file, enter 3 to exit program: ");
        scanf("%d", &whatDo);
        getchar(); // Consume newline 

        switch (whatDo) {
            case (1): // Creating a new directory
                // Check if max number of directories has been reached 
                if (dirArr[MAX_DIR - 1].dirTitle[0] != '\0') {
                    printf("You have reached the maximum amount of categories. (%d)\n", MAX_DIR);
                    break;
                } else {
                    // Loop through the array to find an empty slot for a new directory
                    for (int i = 0; i < MAX_DIR; i++) { // Check if the slot is empty 
                        if (dirArr[i].dirTitle[0] == '\0') {
                            printf("Type in name of directory: ");
                            fgets(dirArr[i].dirTitle, sizeof(dirArr[i].dirTitle), stdin);
                            dirArr[i].dirTitle[strcspn(dirArr[i].dirTitle, "\n")] = '\0';

                            // Create directory and check if successful 
                            if (MKDIR(dirArr[i].dirTitle) == 0) {
                                printf("Directory '%s' created successfully.\n", dirArr[i].dirTitle);
                                break;
                            } else {
                                printf("Error creating directory, directory may already exist.\n");
                                return 1;
                            }
                        }
                    }
                }
                break;

            case(2): // New file creation inside a directory                                 
                int selectDir;
                    // Display available directories
                for (int i = 0; i < MAX_DIR; i++) {
                    if (dirArr[i].dirTitle[0] != '\0') {
                        printf("%s[%d]\n", dirArr[i].dirTitle, i + 1);
                    }
                }

                // Ask user to select a directory
                printf("Which of these categories does the context belong to?: ");
                scanf("%d", &selectDir);
                getchar();
                selectDir--;
                char fileContent[MAX_CONTXT_CHAR];
                int fileIndex;
                // Find an empty slot in the file list
                for (int i = 0; i < MAX_FILES; i++) {
                    if (dirArr[selectDir].fileTitle[i][0] == '\0') {
                        fileIndex = i;
                        break;
                    }
                }
                
                // Get file name from user
                printf("Type in name of text file: ");
                fgets(dirArr[selectDir].fileTitle[fileIndex], sizeof(dirArr[selectDir].fileTitle[fileIndex]), stdin);
                dirArr[selectDir].fileTitle[fileIndex][strcspn(dirArr[selectDir].fileTitle[fileIndex], "\n")] = '\0';

                char filePath[MAX_TITLE_CHAR + MAX_TITLE_CHAR + 2];
                snprintf(filePath, sizeof(filePath), "%s/%s", dirArr[selectDir].dirTitle, dirArr[selectDir].fileTitle[fileIndex]);
                
                FILE *fptr;
                fptr = fopen(filePath, "w");
                if (fptr == NULL) {
                    printf("Error creating file.\n");
                    return 1;
                }
                // Prompt user to enter file content
                printf("Type in the contents of the file (Type in END to stop):\n");

                while (1) {
                    fgets(fileContent, sizeof(fileContent), stdin);
                    if (strncmp(fileContent, "END", 3) == 0) {
                        break;
                    }
                    fprintf(fptr, "%s", fileContent);
                }
                
                fclose(fptr);
                printf("File '%s' created.\n", dirArr[selectDir].fileTitle[fileIndex]);
                break;

            default:
                printf("Exiting Program.\n");
                break;
        }
    } while (whatDo == 1 || whatDo == 2);

    return 0;
}
