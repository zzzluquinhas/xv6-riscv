#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char **argv) {
  if(argc != 2){
	fprintf(2, "Usage: getcnt <syscall id>\n");
	exit(1);
  }

  int syscall_num = atoi(argv[1]);

  int cnt;

  if((cnt = getcnt(syscall_num)) < 0){
	fprintf(2, "getcnt: failed to get count\n");
	exit(1);
  }

  fprintf(1, "syscall %d has been called %d times\n", syscall_num, cnt);
  exit(0);
}