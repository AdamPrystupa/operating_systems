#include <stdio.h>
#include <stdlib.h>
int main(){
	char *user = getenv("LOGNAME");
	printf("Hello world %s!\n",user);
	return 0;
}

