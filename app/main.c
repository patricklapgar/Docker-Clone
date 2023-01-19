#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <curl/curl.h>

# define PATH_SIZE 4096

int setup_environment(char *command, char *image);
int copy_executed_file(char *source, char *destination);
// int create_directory(char *dir);

// Usage: your_docker.sh run <image> <command> <arg1> <arg2> ...
int main(int argc, char *argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
	char *command = argv[3];

	// Ensure an isolated process tree exists upon execution so the program can't interact 
	// with processes running on the host
	unshare(CLONE_NEWPID);
	
	int child_pid = fork();
	if (child_pid == -1) {
		printf("Error forking!");
		return 1;
	}

	if (child_pid == 0) {
		// Replace current program with calling program
		// Create a buffer for the docker image
		char docker_image[PATH_SIZE];
		
		// Setup the environment
		setup_environment(command, docker_image);

		// Replace currently-running program with the docker image
		if (execv(docker_image, &argv[3]) == -1) {
			printf("Error loading child process image %s: %s\n", docker_image, strerror(errno));
			return 1;
		}
	} else {
		// Get child process status
		int status;
		int pid = waitpid(child_pid, &status, 0);
		if (pid == -1) {
			printf("Error in process wait stage");
			return 1;
		}

		if (WEXITSTATUS(status)) {
			printf("Exit code: %d\n", WEXITSTATUS(status));
			return WEXITSTATUS(status);
		}
	}

	return 0;
}

int setup_environment(char *command, char *image) {
	// Create the directory
	char template[] = "/tmp/some_dir.XXXXXX";
	char *temp_dir = mkdtemp(template);
	
	// Some test code
	// char *temp_dir = "/some_dir";
	// printf("Creating temporary directory\n");
	// if(create_directory(temp_dir) != 0) {
	// 	return -1;
	// }

	// Copy the docker image to the temp directory
	strcpy(image, basename(command)); // basename() takes the pathname & returns a pointer to the final component of the pathname

	// Construct the destination path
	size_t size = strlen(temp_dir) + strlen(image);
	char *full_path_size = malloc(size + 1); // Add 1 for \0 character
	strcpy(full_path_size, temp_dir);
	strcat(full_path_size, "/");
	strcat(full_path_size, image);

	// Copy the file to the destination
	if (copy_executed_file(command, full_path_size) != 0) {
		free(full_path_size);
		return -1;
	}
	free(full_path_size);

	// chroot to activate our new environment
	if (chdir(temp_dir) || chroot(temp_dir)) {
		perror("Error, could not chroot to temporary directory");
		return -1;
	}

	return 0;
}

int copy_executed_file(char *source, char *destination) {
	FILE *filesource, *filedestination;
	char buffer[BUFSIZ];
	int s;

	// Try to read from source file
	if ((filesource = fopen(source, "rb")) == NULL) {
		perror("Error, could not open source");
		return -1;
	}

	// Try to write to destination file
	int fd = open(destination, O_RDWR | O_CREAT, 0777);
	if ((filedestination = fdopen(fd, "wb")) == NULL) {
		perror("Error, could not open destination");
		fclose(filesource);
		return -1;
	}

	// Copy contents of source file to destination
	while (!feof(filesource) && !ferror(filesource)) {
		s = fread(buffer, 1, BUFSIZ, filesource);
		if (s > 0) {
			s = fwrite(buffer, 1, s, filedestination);
		}
	}

	fclose(filedestination);
	fclose(filesource);
	return 0;
}
