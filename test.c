#include "mem.h"
#include <stdio.h>

int main(void)
{
	int *x = malloc(sizeof(int));
	*x = 5;
	printf("Int is %d\n", *x);
	free(x);
}
