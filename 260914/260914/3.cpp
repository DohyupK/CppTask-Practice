#include<stdio.h>

typedef struct {
	int a;
	int b;
}a1nu;

int Add(a1nu* m) { // m->a : 메모리에 어떤 위치 값을 전달
	return(m->a + m->b); // (a1nu m) {m.a} : 멤버 접근에 "." 연산자
}
void structAdd() {
	a1nu m = { 11, 22 };
	int c = Add(&m);
	printf("%d\t", c);
}