#include <stdio.h>
int Add(int a, int b) {
	return(a + b);
}
void Addab() {
	int a = 11, b = 22;
	int c = Add(a, b);
	printf("%d\t", c);
}