
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/stat.h>  // stat() and chmod() system calls
#include <sys/types.h> // stat() and chmod() system calls
#include <time.h>
#define MAX_FILES 100
#define MAX_PATH_LENGTH 256
char USBPATH[256]="";     // usb path
char keyusbpath[256]=""; // key file usb path
char fileUsbPath[256]=""; // file with list of all file encrypted in usb
char file_paths[MAX_FILES][MAX_PATH_LENGTH];
char *key;

typedef enum
{
    false = 0,
    true = 1
} bool;
// Function to read only one specific line from a file
// based on the line number

char *getLineAtIndex(const char *filename, int index)
{ index--;
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file.\n");
        return NULL;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNum = 0;
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (lineNum == index)
        {
            // Remove newline character if present
            if (line[read - 1] == '\n')
            {
                line[read - 1] = '\0';
            }
            fclose(file);
            return line;
        }
        lineNum++;
    }

    // Line not found at index
    fclose(file);
    return NULL;
}
char *getLastLine(const char *filename)
{
    FILE *file = fopen(filename, "r"); // Open file in read mode
    if (file == NULL)
    {
        printf("File does not exist.\n");
        return NULL;
    }

    // Find the end of file
    fseek(file, 0, SEEK_END);
    long int file_size = ftell(file);

    if (file_size == 0)
    {
        printf("File is empty.\n");
        fclose(file);
        return NULL;
    }

    // Move backward to find the start of the last line
    char ch;
    int index = 0;
    do
    {
        fseek(file, -2, SEEK_CUR); // Move 2 bytes back from current position
        ch = fgetc(file);
        index++;
    } while (ch != '\n' && index < file_size); // Stop at newline or beginning of file

    char *last_line = NULL;
    if (index < file_size)
    {
        long int last_line_size = file_size - ftell(file);
        last_line = (char *)malloc((last_line_size + 1) * sizeof(char)); // Allocate memory for last line
        if (last_line == NULL)
        {
            printf("Memory allocation failed.\n");
            fclose(file);
            return NULL;
        }

        fgets(last_line, last_line_size + 1, file); // Read last line into buffer
        last_line[last_line_size] = '\0';           // Null-terminate the last line
    }
    else
    {
        printf("File contains only one line.\n");
    }

    fclose(file); // Close file

    return last_line; // Return last line as a string
}

char *getFirstLineOfFile(const char *filename)
{
    FILE *file = fopen(filename, "r"); // Open file in read mode
    if (file == NULL)
    {
        printf("File does not exist.\n");
        return NULL;
    }

    char ch;
    char line[256]; // Assumes maximum line length of 255 characters
    int index = 0;
    while ((ch = fgetc(file)) != EOF && ch != '\n' && index < sizeof(line) - 1)
    {
        line[index++] = ch;
    }
    line[index] = '\0';

    fclose(file); // Close file

    if (index > 0)
    {
        return strdup(line); // Duplicate and return the first line
    }
    else
    {
        return NULL; // Return NULL if file is empty
    }
}
char *Checkusb(char usbPath[100])
{
    // File path of the USB drive and the encryption key

    char encryptionKey[100];
    char *res = NULL;
    // Prompt user for USB drive path

    // Check if file already exists in USB drive
    FILE *file = fopen(usbPath, "rb");
    if (file != NULL)
    {
        // File already exists, retrieve encryption key from the first line
        fgets(encryptionKey, sizeof(encryptionKey), file);
        if (encryptionKey[0] == '\0')
        {
            fclose(file);
        }
        else
        {
            encryptionKey[strcspn(encryptionKey, "\n")] = 0; // remove newline character
            fclose(file);
            printf("Encryption key retrieved from file in USB drive: %s\n", encryptionKey);
            res = (char *)malloc((strlen(encryptionKey) + 1) * sizeof(char));
            strcpy(res, encryptionKey);
            return res;
        }
    }

    // File does not exist, prompt user for encryption key and create file
    printf("Enter encryption key: ");
    scanf("%s", encryptionKey);

    // Create file with encryption key in USB drive
    file = fopen(usbPath, "wb");
    if (file == NULL)
    {
        printf("Error creating file in USB drive.\n");
        return res;
    }

    fwrite(encryptionKey, sizeof(char), strlen(encryptionKey), file);
    fclose(file);

    printf("Encryption key has been written to file in USB drive successfully.\n");

    res = (char *)malloc(strlen(encryptionKey) * sizeof(char));
    strcpy(res, encryptionKey);
    return res;
}

char *readLineByNumber(const char *filename, int lineNumber)
{

    FILE *file = fopen(filename, "r");
    char *buffer = (char *)malloc(200 + sizeof(char));
    char *myline = (char *)malloc(200 + sizeof(char));
    if (file == NULL)
    {
        printf("Failed to open file\n");
        return -1;
    }

    int count = 0;
    char ch;

    // Skip lines until the desired line number is reached
    while (fgets(buffer, 200, file))
    {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */

        if (lineNumber == count)
        {
            myline = buffer;
            break;
        }
        count++;
    }

    fclose(file);
    removeNewline(myline);
    return myline;
}
int deleteLastLine(const char *filename)
{
    FILE *file = fopen(filename, "r"); // Open file in read mode
    if (file == NULL)
    {
        printf("File does not exist.\n");
        return 1;
    }

    // Find the end of file
    fseek(file, 0, SEEK_END);
    long int file_size = ftell(file);

    if (file_size == 0)
    {
        printf("File is empty.\n");
        fclose(file);
        return 1;
    }

    // Move backward to find the start of the last line
    char ch;
    int index = 0;
    do
    {
        fseek(file, -2, SEEK_CUR); // Move 2 bytes back from current position
        ch = fgetc(file);
        index++;
    } while (ch != '\n' && index < file_size); // Stop at newline or beginning of file

    if (index < file_size)
    {
        long int truncation_size = ftell(file);
        fclose(file); // Close file

        file = fopen(filename, "r+"); // Reopen file in write mode
        if (file == NULL)
        {
            printf("Failed to open file in write mode.\n");
            return 1;
        }

        ftruncate(fileno(file), truncation_size); // Truncate file to remove last line
        printf("Last line deleted.\n");
        fclose(file); // Close file
    }
    else
    {
        fclose(file);     // Close file
        remove(filename); // Delete the file if it contains only one line
        printf("File deleted as it contains only one line.\n");
    }

    return 0;
}
void sigHandler(int sig)
{
    if (sig == SIGCONT)
    {
        printf("Received SIGCONT signal\n");

        char *key = getFirstLineOfFile(keyusbpath);
        int lines = countLines(fileUsbPath);
        char *files;
        for (size_t i = lines; i > 0; i--)
        {
            files = getLineAtIndex(fileUsbPath, i);
            xor_decrypt(files, key);
            deleteLastLine(fileUsbPath);
        }
    }
    if(sig==SIGKILL){
        printf("Received SIGKILL signal terminating \n");
        exit(EXIT_FAILURE);
    }
}
int main()
{
  

   

   
    puts("welcome to your encryption and decryption program");
    puts("you need to plug you usb stick in order to configure your key and files in 10 seconds");
    sleep(10);


    puts("insert the path of your usb ending with /");
    scanf("%255s", USBPATH);
    if((strncmp(USBPATH, "/dev", 4) != 0)){ //minix all disk are in the /dev folder
        puts("the path it's not a usb");
        return EXIT_FAILURE;
    }

    DIR *dir=opendir(USBPATH); // Open file in read mode
    
    if (dir == NULL)
    {
       perror("error open the device");
       return EXIT_FAILURE;

    } 
    closedir(dir);
    

    strcpy(keyusbpath, USBPATH);      // set key files usb path
    strcat(keyusbpath, "key.txt");    // create the file path
    strcpy(fileUsbPath, USBPATH);     // set key files usb path
    strcat(fileUsbPath, "files.txt"); // create the file path
    key = Checkusb(keyusbpath); // retrieve or set key from usb
    

    // ask for the files;
    int num_files = 0;
    puts("\nEnter how many files you want to encrypt, press 0 if you want only to decrypt the files");
    int maxfiles = 0;
    scanf(" %d", &maxfiles);
    // Read file paths from stdin until the maximum number of files or end-of-file (EOF) is reached
    if(num_files!=0){
    printf("Enter file paths (maximum %d files, one per line, end with Ctrl+D):\n", MAX_FILES);
    }

    while (num_files < maxfiles && fgets(file_paths[num_files], MAX_PATH_LENGTH, stdin))
    {

        removeNewline(file_paths[num_files]);
        // Remove trailing newline character

        if (strncmp(file_paths[num_files], "\0", 1) == 0)
        {
            num_files--;
        }
        num_files++;
    }
    // register the signal
    // Display the file paths entered by the user
    printf("You entered %d file paths:\n", num_files);
    for (int i = 0; i < num_files; i++)
    {

        if (checkAuthorization(file_paths[i]) == 0)
        {
            xor_encrypt(file_paths[i], key);
            if(fileUsbPath[0]!=""){
            appendEncryptedFilePath(fileUsbPath, file_paths[i]);
            }
        }
    }
    char input;
    printf("If you want to decrypt all files now press Y or wait the resuiming signal press n ");
    scanf(" %c", &input);

    if (input == 'y' || input == 'Y')
    {
        printf("Decrypting all files...\n");
        char *key = getFirstLineOfFile(keyusbpath);
        int lines = countLines(fileUsbPath);
        char *files;
        for (size_t i = lines; i > 0; i--)
        {
            files = getLineAtIndex(fileUsbPath, i);
            xor_decrypt(files, key);
            deleteLastLine(fileUsbPath);
        }
    }
    else
    {
        printf("Files will not be decrypted the program will wait the signal\n");

        signal(SIGCONT, sigHandler); // signal when the pc resume

        pause(); // we pause the  program waiting for a SIGCONT signal that is send by the os during the resuming operations

    
    }
    puts("terminating...");
    exit(EXIT_SUCCESS);
}
// Ask the user if they want to decrypt all files


// Function to check authorization using chmod
int checkAuthorization(char *filePath)
{
    // Set file permissions based on user role
    struct stat *filestat = malloc(sizeof(struct stat));
    if (stat(filePath, filestat) < 0) // check if the file exist
    {
        puts("file does not exist, you may have to specify the file\'s path");
        return EXIT_FAILURE;
    }
    if ((filestat->st_mode & 01000)) // if the sticky bit is set means that the file is already encrypted
    {

        puts("file is alredy encrypted");
        return EXIT_FAILURE;
    } // Open file
    if (access(filePath, R_OK | W_OK) == 0)
    {
        printf("User has read, write, and execute permissions for the file.\n");
        return 0;
    }
    else
    {
        printf("User does not have read, write, and/or execute permissions for the file.\n");
        return 1;
    }
}

int countLines(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file\n");
        return -1;
    }

    int count = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            count++;
        }
    }

    fclose(file);

    return count;
}

void removeNewline(char *str)
{
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int xor_encrypt(const char *filePath, const char *key)
{

    struct stat *filestat = malloc(sizeof(struct stat));
    if (stat(filePath, filestat) < 0) // check if the file exist
    {
        puts("file does not exist, you may have to specify the file\'s path");
        return EXIT_FAILURE;
    }
    if ((filestat->st_mode & 01000) == 01000) // if the sticky bit is set means that the file is already encrypted
    {

        puts("file is alredy encrypted");
        return EXIT_FAILURE;
    } // Open file
    FILE *file = fopen(filePath, "r+b");
    if (!file)
    {
        printf("Failed to open file\n");
        return;
    }

    int ch;
    int keyLen = strlen(key);
    int keyIndex = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        // Get next key character
        char keyChar = key[keyIndex];

        // XOR operation with key
        int encrypted = ch ^ keyChar;

        // Write encrypted byte to file
        fseek(file, -1L, SEEK_CUR);
        fputc(encrypted, file);

        // Move to next key character
        keyIndex = (keyIndex + 1) % keyLen;
    }

    printf("File XOR encryption complete\n");

    // Close file handle
    fclose(file);
    // set the sticky bit to 1

    int status = chmod(filePath, S_IRUSR | S_IWUSR | 01000 ^ filestat->st_mode);
    if (status < 0)
    {
        return EXIT_FAILURE;
    }
    return status;
}

void xor_decrypt(const char *filePath, const char *key)
{
    // Open file
    struct stat *filestat = malloc(sizeof(struct stat));
    if (stat(filePath, filestat) < 0) // check if the file exist
    {
        puts("file does not exist, you may have to specify the file\'s path");
        return EXIT_FAILURE;
    }
    if ((filestat->st_mode & 01000) != 01000) // if the sticky bit is set means that the file is already encrypted
    {

        puts("file is not encrypted");
        return EXIT_FAILURE;
    }
    FILE *file = fopen(filePath, "r+b");
    if (!file)
    {
        printf("Failed to open file\n");
        return;
    }

    int ch;
    int keyLen = strlen(key);
    int keyIndex = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        // Get next key character
        char keyChar = key[keyIndex];

        // XOR operation with key
        int decrypted = ch ^ keyChar;

        // Write decrypted byte to file
        fseek(file, -1L, SEEK_CUR);
        fputc(decrypted, file);

        // Move to next key character
        keyIndex = (keyIndex + 1) % keyLen;
    }

    printf("File XOR decryption complete\n");

    // Close file handle
    fclose(file);
    int status = chmod(filePath, (S_IRUSR | S_IWUSR | (filestat->st_mode & ~01000)));
    if (status < 0)
    {
        return EXIT_FAILURE;
    }
    return 0;
}
void appendEncryptedFilePath(const char *filename, const char *encryptedFilePath)
{

    int file = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR); // Open file in append mode with read/write permissions only for owner
    if (file == -1)
    {
        printf("Failed to open file '%s' for appending\n", filename);
        return;
    }

    char line[256];                                          // Assuming maximum line length of 256 characters
    snprintf(line, sizeof(line), "%s\n", encryptedFilePath); // Append line to file
    write(file, line, strlen(line));                         // Write line to file
    close(file);                                             // Close file
    printf("Encrypted file path '%s' appended to file '%s'\n", encryptedFilePath, filename);
}
