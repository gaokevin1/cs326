/**
 * inode_reader.c
 *
 * C program to print file information using stat system call.
 *
 * Author: Kevin and Angel 
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("\nInvalid Command Line Argument\n"); 
    } else {
    	perm_checker(argv[1]);
    }

    return 0; 
}

void perm_checker(char* arg) {
	struct stat statbuf;
	
	if (stat(arg, &statbuf) == -1) {
		perror("stat");
		return;
	}

	int uid = getuid();
	int uid_process = statbuf.st_uid;
	
	// If user owns the file
	if (uid == statbuf.st_uid) {
		printf("\nThis file is owned by the current user\n");
	} else {
		// If user has read privilages
		if ((statbuf.st_mode & S_IROTH) || statbuf.st_mode & S_IRWXO) {
			printf("\nThis file is owned by someone else but *IS* readable by this process\n");
		} else {
			printf("\nThis file is owned by someone else and is *NOT* readable by this process\n");
		}
	}
}