#include "mem.h"
#include <errno.h>
#include <stdio.h>

int main(void)
{
	int *x = malloc(sizeof(int));
	*x = 5;
	printf("Int is %d\n", *x);
	printf("Errno is %d\n", errno);
	free(x);
	return errno;
}
