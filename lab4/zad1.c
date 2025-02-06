#include <stdio.h>
#include <time.h>

int main()
{

	int i = 0;
	struct timespec req = {0, 10000000}; // 10ms

	while (1)
	{
		++i;
		nanosleep(&req, NULL);
	}
	return 0;
}
