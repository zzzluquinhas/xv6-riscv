#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int tickets = atoi(argv[1]);
  if (argc != 2)
  {
	fprintf(1, "Usage: settickets <tickets>\n");
	exit(1);
  }
  if (tickets < 1)
  {
	fprintf(1, "Tickets must be greater than 1\n");
	exit(1);
  }
  if(settickets(tickets) < 0)
  {
	fprintf(1, "Error: settickets\n");
	exit(1);
  }
  fprintf(1, "Process tickets set to %d\n", tickets);
  exit(0);
}