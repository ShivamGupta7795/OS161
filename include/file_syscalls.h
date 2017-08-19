#include <cdefs.h> /* for __DEAD */
int sys_write(int fd,const void *buf, size_t buflen,int *err1); 
int sys_open(const char *filename, int flags,int *err1);
int sys_read(int fd, void *buf, size_t buflen,int *err1);
int sys_close(int fd,int *err1);
off_t sys_lseek(int fd, off_t pos, int whence, int *err1);
pid_t sys_fork(struct trapframe *tf,int *err1);
pid_t sys_getpid(void);
pid_t sys_waitpid(pid_t pid, int *status, int options, int *err1);
void sys__exit(int exitcode);
int sys_execv(const char *program, char **args,int *err1); 
int sys_dup2(int oldfd, int newfd,int *err1);
int sys_sbrk(intptr_t amount,int *err1);
