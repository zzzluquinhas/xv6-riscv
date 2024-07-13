#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  struct pstat info;
  if (getpinfo(&info) < 0)
  {
	printf("getpinfo failed\n");
	exit(1);
  }
  for (int i = 0; i < NPROC; i++)
  {
	if (info.inuse[i])
	{
	  printf("PID: %d\tTickets: %d\tTicks: %d\n", info.pid[i], info.tickets[i], info.ticks[i]);
	}
  }
  exit(0);
}