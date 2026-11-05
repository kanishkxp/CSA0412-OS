#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* Reads a small file and prints it using only system calls.
   Used with strace to see open, read, write, close and exit_group. */
int main(void)
{
    const char *msg = "patient 101: heart rate normal\n";
    int fd = open("patient.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, msg, strlen(msg));
    close(fd);

    char buf[64];
    fd = open("patient.txt", O_RDONLY);
    ssize_t n = read(fd, buf, sizeof buf);
    close(fd);
    write(STDOUT_FILENO, buf, (size_t)n);
    unlink("patient.txt");
    return 0;
}
