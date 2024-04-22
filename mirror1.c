#define _XOPEN_SOURCE 500 // Required for nftw
#define BUFFER_SIZE 100000  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <pwd.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> // for getenv

#define MAX_FILE_TYPES 3

#define PORT 7001
#define MAX_COMMAND_SIZE 256
#define MAX_RESPONSE_LENGTH 256
#define MAX_PATH_LENGTH 4096
#define TAR_COMMAND "tar -czf temp.tar.gz"

char result_buffer[MAX_COMMAND_SIZE];
char file_info[1024]; // Global variable to store file information
char UserPath[256]; // Assuming maximum UserPath length is 255 characters


// Structure to hold directory name and creation time
struct DirInfo {
    char* name;
    time_t creation_time;
};

const char* target_filename;
int found;
int size1, size2;
int searched_files = 0; // Variable to count files searched
int files_added = 0;    // Variable to count files added to the tar archive

char** files_to_archive; // Array to store files to be added to the archive
int files_to_archive_count = 0; // Counter for files to be added
char* file_types[MAX_FILE_TYPES];
int file_types_count = 0;
char tar_filename[] = "temp.tar.gz";
// int searched_files = 0;
char** split_string(const char* str, int* num_tokens) {
    // Count the number of tokens
    *num_tokens = 0;
    const char* temp = str;
    while (*temp) {
        if (*temp == ' ') {
            (*num_tokens)++;
        }
        temp++;
    }
    (*num_tokens)++; // Add one for the last token

    // Allocate memory for an array of strings
    char** tokens = (char**)malloc(*num_tokens * sizeof(char*));
    if (tokens == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Tokenize the string
    char* token = strtok((char*)str, " ");
    int i = 0;
    while (token != NULL) {
        tokens[i] = strdup(token); // Duplicate the token
        if (tokens[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        i++;
        token = strtok(NULL, " ");
    }
    printf("Val is %d\n", *num_tokens);

    return tokens;
}

int is_selected_file(const char* filename) {
    char* ext = strrchr(filename, '.');
    if (ext != NULL) {
        for (int i = 0; i < file_types_count; ++i) {
            if (strcmp(ext + 1, file_types[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int add_to_tar(const char* filename, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
    printf("Checing this file %s\n", filename);
    ++searched_files; // Increment the counter for each file searched

    if (tflag == FTW_F && is_selected_file(filename)) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { // Child process
            execlp("tar", "tar", "rf", tar_filename, filename, NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
        else { // Parent process
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    fprintf(stderr, "tar command failed with exit status %d\n", WEXITSTATUS(status));
                }
            }
            else {
                fprintf(stderr, "tar command failed\n");
            }
        }
    }
    return 0;
}

int add_to_tar2(const char* filename) {
    printf("Adding this file to tar %s\n",filename);
    if (1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { // Child process
            execlp("tar", "tar", "rf", "/home/ap2310/w24project/temp.tar.gz", filename, NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
        else { // Parent process
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    fprintf(stderr, "tar command failed with exit status %d\n", WEXITSTATUS(status));
                }
            }
            else {
                fprintf(stderr, "tar command failed\n");
            }
        }
    }
    return 0;
}


void create_tar_file() {
    int tar_fd;
    if ((tar_fd = open(tar_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    close(tar_fd);
}

void create_tar_file2() {
    int tar_fd;
    if ((tar_fd = open("/home/ap2310/w24project/temp.tar.gz", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        perror("open");
        // printf("ADSF");
        // exit(EXIT_FAILURE);
    }
    close(tar_fd);
}

// Function to create directory if it doesn't exist
void ensure_directory(const char* dir_path) {
    struct stat st;
    if (stat(dir_path, &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(dir_path, 0700) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }
    else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s is not a directory\n", dir_path);
        exit(EXIT_FAILURE);
    }
}

// Function to add a file path to the list of files to be archived
void add_file_to_archive(const char* file_path) {
    // Allocate memory for the file path
    char* new_path = strdup(file_path);
    if (new_path == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Reallocate memory for the files_to_archive array
    char** new_files_to_archive = realloc(files_to_archive, (files_to_archive_count + 1) * sizeof(char*));
    if (new_files_to_archive == NULL) {
        perror("Memory allocation failed");
        free(new_path); // Free memory allocated for new_path
        exit(EXIT_FAILURE);
    }

    // Update files_to_archive and increment count
    files_to_archive = new_files_to_archive;
    files_to_archive[files_to_archive_count] = new_path;
    files_to_archive_count++;
}

// Function to filter files based on size
int filter(const char* file_path, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    if (typeflag == FTW_F) { // Check if it's a file
        off_t file_size = sb->st_size;
        if (file_size >= size1 && file_size <= size2) {
            // Increment count of searched files
            searched_files++;
            // Add this file to the list of files to be archived
            add_file_to_archive(file_path);
        }
    }
    return 0;
}

// Function to free memory allocated for files_to_archive
void free_files_to_archive() {
    for (int i = 0; i < files_to_archive_count; i++) {
        free(files_to_archive[i]);
    }
    free(files_to_archive);
}

// Function to execute the tar command
void execute_tar_command() {
    // Create a temporary file to store the list of files to be archived
    FILE* file_list = fopen("file_list.txt", "w");
    if (file_list == NULL) {
        perror("Error creating file list");
        free_files_to_archive(); // Free memory allocated for files_to_archive
        exit(EXIT_FAILURE);
    }

    char tar_filename[MAX_PATH_LENGTH];
    snprintf(tar_filename, sizeof(tar_filename), "w24project/temp_w24fz_%ld_%d.tar.gz", (long int)time(NULL), rand());

    // Write the list of files to the file list
    for (int i = 0; i < files_to_archive_count; i++) {
        fprintf(file_list, "%s\n", files_to_archive[i]);
    }
    fclose(file_list);

    // Execute the tar command using the file list
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        free_files_to_archive(); // Free memory allocated for files_to_archive
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) { // Child process
        // Execute the tar command using the file list
        execlp("tar", "tar", "-czf", tar_filename, "-T", "file_list.txt", (char*)NULL);
        perror("execvp failed");
        free_files_to_archive(); // Free memory allocated for files_to_archive
        exit(EXIT_FAILURE);
    }
    else { // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error executing command: tar\n");
            free_files_to_archive(); // Free memory allocated for files_to_archive
            exit(EXIT_FAILURE);
        }
    }

    // Remove the temporary file
    if (remove("file_list.txt") != 0) {
        perror("Error removing file_list.txt");
        free_files_to_archive(); // Free memory allocated for files_to_archive
        exit(EXIT_FAILURE);
    }

    // Reset files count for next iteration
    files_to_archive_count = 0;
}



int search_file(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {

    const char* filename = strrchr(fpath, '/');
    if (filename != NULL && strcmp(filename + 1, target_filename) == 0) {
        sprintf(file_info, "Filename: %s\n", fpath);
        sprintf(file_info + strlen(file_info), "Size: %ld bytes\n", sb->st_size);
        sprintf(file_info + strlen(file_info), "Date created: %s", ctime(&sb->st_ctime));
        sprintf(file_info + strlen(file_info), "Permissions: ");
        sprintf(file_info + strlen(file_info), "%s", (S_ISDIR(sb->st_mode)) ? "d" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IRUSR) ? "r" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IWUSR) ? "w" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IXUSR) ? "x" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IRGRP) ? "r" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IWGRP) ? "w" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IXGRP) ? "x" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IROTH) ? "r" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IWOTH) ? "w" : "-");
        sprintf(file_info + strlen(file_info), "%s", (sb->st_mode & S_IXOTH) ? "x" : "-");
        sprintf(file_info + strlen(file_info), "\n");
        found = 1;
        // Returning nonzero value to stop the search
        return 1;
    }
    return 0;
}




// Function to compare two directory names alphabetically
int compare_alpha(const void* a, const void* b) {
    const char* str1 = *(const char**)a;
    const char* str2 = *(const char**)b;

    // Skip dot at the beginning, if present
    if (str1[0] == '.')
        str1++;
    if (str2[0] == '.')
        str2++;

    // Perform a case-insensitive comparison
    return strcasecmp(str1, str2);
}

// Function to compare two directory creation times
int compare_time(const void* a, const void* b) {
    const struct DirInfo* dir1 = (const struct DirInfo*)a;
    const struct DirInfo* dir2 = (const struct DirInfo*)b;

    if (dir1->creation_time < dir2->creation_time) return -1;
    if (dir1->creation_time > dir2->creation_time) return 1;
    return 0;
}

// Function to list directories based on specified criteria
void listDirectories(int client_socket, const char* path, char option) {
    DIR* dir;
    struct dirent* entry;
    struct DirInfo directories[100];
    int count = 0;

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            struct stat statbuf;
            char dir_path[MAX_PATH_LENGTH];
            snprintf(dir_path, sizeof(dir_path), "%s/%s", path, entry->d_name);

            if (stat(dir_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                directories[count].name = strdup(entry->d_name);
                directories[count].creation_time = statbuf.st_ctime; // Use st_ctime for creation time
                count++;
            }
        }
        closedir(dir);

        // Sort directories based on specified criteria
        switch (option) {
        case 'a':
            qsort(directories, count, sizeof(struct DirInfo), compare_alpha);
            break;
        case 't':
            qsort(directories, count, sizeof(struct DirInfo), compare_time);
            break;
        default:
            fprintf(stderr, "Invalid option\n");
            exit(EXIT_FAILURE);
        }

        // Send sorted directories to the client
        for (int i = 0; i < count; i++) {
            char response[MAX_PATH_LENGTH + 2]; // Add 2 for newline and null terminator
            snprintf(response, sizeof(response), "%s\n", directories[i].name);
            write(client_socket, response, strlen(response));
            free(directories[i].name);
        }
    }
    else {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
}


char files1[BUFFER_SIZE] = ""; // List of files to include in the tar command
// int search1(const char *dir_path, const char *date, time_t target_time) {
char* search1(const char* dir_path, const char* date, time_t target_time) {

    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    int files_found = 0;
    if ((dir = opendir(dir_path)) == NULL) {
        // perror("Error opening directory");
        return files_found;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (strstr(path, "cache") != NULL || strstr(path, "local") != NULL || strstr(path, ".cache") != NULL) {
            continue;
        }

        files_found += search1(path, date, target_time);
        if (stat(path, &statbuf) == 0) {

            time_t file_time = statbuf.st_ctime; // Use st_ctime as a fallback
            if (file_time >= target_time) {
                printf("File time is %ld and target is %ld\n", file_time, target_time);
                // printf("Path names are %s\n",path);
                add_to_tar2(path);
            }
        }
    }
    // return 1;

    // closedir(dir);
    return files1;
}

int isFileValid(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // File couldn't be opened
        return 0;
    }
    fclose(file);
    return 1;
}

// int search1(const char *dir_path, const char *date, time_t target_time) {
char* search3(const char* dir_path, const char* extension) {
    // printf("Inside search2\n");
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    int files_found = 0;
    // printf("dir path %s\n", dir_path);
    if ((dir = opendir(dir_path)) == NULL) {
        // perror("Error opening directory");
        return files_found;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (strstr(path, "cache") != NULL || strstr(path, "local") != NULL || strstr(path, ".cache") != NULL) {
            continue;
        }
        search3(path, extension);
        if (stat(path, &statbuf) == 0) {
            printf("Testing path %s\n",path);
            if (strstr(path, extension) != NULL && isFileValid(path)) {
                // printf("Path is %s\n", path);
                // printf("Paths are %s\n", path);
                add_to_tar2(path);
            }
            // time_t file_time = statbuf.st_ctime; // Use st_ctime as a fallback
    // if (file_time >= target_time) {
    //     printf("File time is %ld and target is %ld\n", file_time, target_time);
    //     // printf("Path names are %s\n",path);
    //     add_to_tar2(path);
        }
    }
    // }
    // return 1;

    // closedir(dir);
    // return files1;
}


void w24fda_collect_info(char* date, char* file_info, int new_socket) {
    // Convert date string to time_t
    char* tar_filename1 = "w24project/tempa.tar.gz";
    struct tm tm_date;

    if (strptime(date, "%Y-%m-%d", &tm_date) == NULL) {
        perror("strptime");
        strcat(file_info, "Invalid date format\n");
        return;
    }

    tm_date.tm_hour = 23;
    tm_date.tm_min = 59;
    tm_date.tm_sec = 59;

    // Adjust month index to match the range (0-11)
    tm_date.tm_mon -= 1; // April is the 4th month, so subtract 1 to get index 3.

    time_t target_time = mktime(&tm_date);
    printf("Target time: %ld\n", target_time);

    // Clear files1 buffer before using
    memset(files1, 0, sizeof(files1));

    // Traverse the file system and collect file information based on the date
    DIR* dir;
    struct dirent* entry;
    create_tar_file2();

    int files_found1 = search1(UserPath, date, target_time);
    char command3[BUFFER_SIZE];
    send(new_socket, tar_filename1, strlen(tar_filename1), 0);

}


char files2[BUFFER_SIZE] = ""; // List of files to include in the tar command
// int search2(const char *dir_path, const char *date, time_t target_time) {
char* search2(const char* dir_path, const char* date, time_t target_time) {

    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    int files_found = 0;
    if ((dir = opendir(dir_path)) == NULL) {
        // perror("Error opening directory");
        return files_found;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (strstr(path, "cache") != NULL || strstr(path, "local") != NULL || strstr(path, ".cache") != NULL || strstr(path, "snap") != NULL) {
            continue;
        }
        printf("path %s \n", path);
        files_found += search2(path, date, target_time);
        if (stat(path, &statbuf) == 0) {

            time_t file_time = statbuf.st_ctime; // Use st_ctime as a fallback
            printf("File time is %ld and target is %ld\n", file_time, target_time);
            if (file_time <= target_time) {
                printf("File time is %ld and target is %ld\n", file_time, target_time);
                // printf("Path names are %s\n",path);
                add_to_tar2(path);
            }
        }
    }
    // return 1;

    // closedir(dir);
    return files2;
}

void w24fdb_collect_info(char* date, char* file_info, int new_socket) {
    // Convert date string to time_t
    char* tar_filename1 = "w24project/tempw24fdb.tar.gz";
    struct tm tm_date;

    if (strptime(date, "%Y-%m-%d", &tm_date) == NULL) {
        perror("strptime");
        strcat(file_info, "Invalid date format\n");
        return;
    }

    tm_date.tm_hour = 23;
    tm_date.tm_min = 59;
    tm_date.tm_sec = 59;

    // Adjust month index to match the range (0-11)
    tm_date.tm_mon -= 1; // April is the 4th month, so subtract 1 to get index 3.

    time_t target_time = mktime(&tm_date);
    printf("Target time: %ld\n", target_time);

    // Clear files2 buffer before using
    memset(files2, 0, sizeof(files2));

    // Traverse the file system and collect file information based on the date
    DIR* dir;
    struct dirent* entry;
    create_tar_file2();

    int files_found1 = search2(UserPath, date, target_time);
    char command3[BUFFER_SIZE];
    send(new_socket, tar_filename1, strlen(tar_filename1), 0);

}


void crequest(int client_socket) {
    char buffer[MAX_COMMAND_SIZE];
    int n;
    // Function to check if a string contains only numeric characters
    int isNumeric(const char* str) {
        while (*str) {
            if (*str < '0' || *str > '9') // Check if the character is not a digit
                return 0;
            str++;
        }
        return 1;
    }

    while (1) {

        memset(buffer, 0, MAX_COMMAND_SIZE);
        // Read command from client
        read(client_socket, buffer, sizeof(buffer));
        printf("Executing %s\n", buffer);
        // Process command
        if (strncmp(buffer, "quitc", 5) == 0) {
            printf("Client requested to quit.\n");
            break;
        }

        else if (strncmp(buffer, "w24ft", 5) == 0) {

            printf("This is it %s\n", buffer);
            int c;
            char** tokens = split_string(buffer, &c);
            printf("Val received is %d\n", c);

            create_tar_file2();
            printf("Search path is %s\n", UserPath);

            for (int i = 1; i < c + 1; i++) {
                char command[10];
                sprintf(command, ".%s", tokens[i]);
                file_types[file_types_count++] = tokens[i];
                int files_found1 = search3("/home/ap2310/", command);
            }

            char command3[BUFFER_SIZE];

            write(client_socket, "w24project/temp.tar.gz", strlen("w24project/temp.tar.gz"));
            // free(files_info);
            //}
        }
        else if (strncmp(buffer, "w24fda", 6) == 0) {
            printf("else");
            // Handle w24fda command
            char* date = strtok(buffer, " ") + 7;
            if (date == NULL || strlen(date) != 10) {
                char response[] = "Usage: w24fda YYYY-MM-DD\n";
                write(client_socket, response, strlen(response));
            }
            else {
                //files_info = malloc(BUFFER_SIZE);
                char* files_info = malloc(BUFFER_SIZE);
                if (files_info == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                memset(files_info, 0, BUFFER_SIZE);
                w24fda_collect_info(date, files_info, client_socket);
                write(client_socket, files_info, strlen(files_info));
                free(files_info);
            }
        }
        else if (strncmp(buffer, "w24fdb", 6) == 0) {
            printf("else");
            // Handle w24fda command
            char* date = strtok(buffer, " ") + 7;
            if (date == NULL || strlen(date) != 10) {
                char response[] = "Usage: w24fdb YYYY-MM-DD\n";
                write(client_socket, response, strlen(response));
            }
            else {
                //files_info = malloc(BUFFER_SIZE);
                char* files_info = malloc(BUFFER_SIZE);
                if (files_info == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                memset(files_info, 0, BUFFER_SIZE);
                w24fdb_collect_info(date, files_info, client_socket);
                write(client_socket, files_info, strlen(files_info));
                free(files_info);
            }
        }
        else if (strncmp(buffer, "w24fn", 5) == 0) {
            // Handle w24fn command
            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Usage: w24fn filename\n";
                write(client_socket, response, strlen(response));
            }
            else {
                // Check if there are additional tokens (filenames)
                token = strtok(NULL, " ");
                if (token != NULL) {
                    char response[] = "Usage: w24fn filename\n";
                    write(client_socket, response, strlen(response));
                }
                else {
                    // Syntax is correct, proceed with processing the command
                    target_filename = strtok(buffer, " ") + 6; // Extract filename from the command

                    const char* root_dir = getenv("HOME");
                    if (root_dir == NULL) {
                        printf("Unable to get home directory.\n");
                        exit(EXIT_FAILURE);
                    }

                    found = 0;
                    int flags = FTW_PHYS; // Using FTW_PHYS for physical walk of the filesystem
                    if (nftw(root_dir, search_file, 20, flags) == -1) {
                        perror("nftw");
                        exit(EXIT_FAILURE);
                    }

                    if (!found) {
                        // Send "File not found" response
                        char response[] = "File not found.\n";
                        write(client_socket, response, strlen(response));
                    }
                    else {
                        // // Send the file information
                        // write(client_socket, file_info, strlen(file_info));

                        // // Send "Command processed successfully" response
                        // char success_response[] = "Command processed successfully.\n";
                        // write(client_socket, success_response, strlen(success_response));
                        // Calculate the length of the combined response
                        size_t combined_response_length = strlen(file_info) + strlen("Command processed successfully.\n") + 1;

                        // Dynamically allocate memory for the combined response
                        char* combined_response = malloc(combined_response_length);
                        if (combined_response == NULL) {
                            perror("Memory allocation failed");
                            exit(EXIT_FAILURE);
                        }

                        // Copy the file_info and additional response to the combined response buffer
                        strcpy(combined_response, file_info);
                        strcat(combined_response, "Command processed successfully.\n");

                        // Send the combined response to the client
                        write(client_socket, combined_response, combined_response_length);

                        // Free the dynamically allocated memory
                        free(combined_response);
                    }


                }
            }
        }
        else if (strncmp(buffer, "w24fz", 5) == 0) {

            // Handle w24fz command
            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            if (token == NULL || !isNumeric(token) || size1 < 0) { // Check if size1 is not provided or not numeric
                char response[] = "Usage: w24fz size1 size2\ninvalid size1 range\n";
                write(client_socket, response, strlen(response));
            }
            else {
                size1 = atoi(token);
                token = strtok(NULL, " ");
                if (token == NULL || !isNumeric(token) || size2 < 0) { // Check if size2 is not provided or not numeric
                    char response[] = "Usage: w24fz size1 size2\ninvalid size2 range\n";
                    write(client_socket, response, strlen(response));
                }
                else {
                    size2 = atoi(token);

                    if (size1 > size2) {
                        char response[] = "Invalid size range\n";
                        write(client_socket, response, strlen(response));
                    }

                    // Allocate memory for files_to_archive
                    files_to_archive = malloc(MAX_PATH_LENGTH * sizeof(char*));
                    if (files_to_archive == NULL) {
                        perror("Memory allocation failed");
                        exit(EXIT_FAILURE);
                    }

                    // Ensure that the directory exists or create it
                    const char* archive_directory = "w24project";
                    ensure_directory(archive_directory);

                    // Perform the search using nftw, starting from the user's home directory
                    char* home_directory = getenv("HOME");
                    if (home_directory == NULL) {
                        perror("Unable to get home directory");
                        exit(EXIT_FAILURE);
                    }

                    // Reset searched files count before starting the search
                    searched_files = 0;

                    if (nftw(home_directory, filter, 20, FTW_PHYS) == -1) {
                        perror("nftw");
                        exit(EXIT_FAILURE);
                    }

                    // Execute tar command to create the tar archive
                    execute_tar_command();

                    // Check if any files were found
                    if (searched_files == 0) {
                        char response[] = "No file found\n";
                        write(client_socket, response, strlen(response));
                    }
                    else {
                        char response_combined[MAX_RESPONSE_LENGTH]; // Assuming MAX_RESPONSE_LENGTH is defined somewhere
                        // Combine the response messages
                        sprintf(response_combined, "Searched files: %d\nArchive created successfully: temp.tar.gz\nCommand processed successfully.\n", searched_files);
                        // Send the combined response to the client
                        write(client_socket, response_combined, strlen(response_combined));


                    }

                    // Free memory allocated for files_to_archive
                    free_files_to_archive();

                }

            }

        }

        else if (strncmp(buffer, "dirlist", 7) == 0) {
            printf("dirlist");
            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            if (token == NULL || (token[0] != '-' || (token[1] != 'a' && token[1] != 't'))) {
                char response[] = "Usage: dirlist -a|-t\n";
                write(client_socket, response, strlen(response));
            }
            else {
                // Syntax is correct, proceed with processing the command
                struct passwd* pw = getpwuid(getuid());
                if (!pw) {
                    perror("getpwuid");
                    exit(EXIT_FAILURE);
                }
                const char* home_dir = pw->pw_dir;

                // Call listDirectories with the correct arguments
                listDirectories(client_socket, home_dir, token[1]);

                // Send response to client
                char response[] = "Command processed successfully.\n";
                write(client_socket, response, strlen(response));
            }
        }
        else {
            // Handle other commands

            // Send response to client
            char response[] = "Unknown command.\n";
            write(client_socket, response, strlen(response));

        }
    }

    // Close client socket
    close(client_socket);
}


int main() {
    char* home_dir = getenv("HOME");
    sprintf(UserPath, "%s", home_dir);
    // printf("Path: %s\n", UserPath);

    int server_socket, client_socket, pid;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int addrlen = sizeof(server_addr);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Server configurations
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }


    while (1) {


        // Accept incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Fork a child process to handle client request
        printf("client connected to mirror1\n");
        if ((pid = fork()) == 0) {
            // Child process
            close(server_socket); // Close server socket in child process
            crequest(client_socket); // Handle client request
            exit(0);
        }
        else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        // Parent process
        close(client_socket); // Close client socket in parent process
    }

    return 0;
}
