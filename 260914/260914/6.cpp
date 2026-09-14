#include<stdio.h>

void Enter2values() {
	int a, b;
	printf("Enter 2 values :");
	scanf_s("%d\n%d", &a, &b);
	int c = a + b;
	printf("%d\n", c);
}