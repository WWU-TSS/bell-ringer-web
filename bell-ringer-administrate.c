#include <unistd.h>

int main(int argc, char **argv, char *const *envp)
{
    setuid(0);
    setgid(0);
    execve("/usr/local/bin/bell-ringer-administrate.sh", argv, envp);
    return 1;
}
