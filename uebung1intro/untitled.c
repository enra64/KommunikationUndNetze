#include <stdio.h>

void swap(int* a, int* b){
	printf("u:%i, v:%i\n", *a, *b);
	int c = *a;
	*a = *b;
	*b = c;
}

int main(int argc, char **argv)
{
	int *a;
	int x = 5;
	a = &x;
	
	printf("adresse von x:%p\n", &x);
	printf("wert von x:%i\n", x);
	
	printf("adresse von a:%p\n", &a);
	printf("adresse, auf die a zeigt (also quasi der wert von a):%p\n", a);
	printf("wert der adresse auf die a zeigt:%i\n", *a);
	
	int u;
	int v;
	
	printf("Zahl u:\n");
	scanf("%i", &u);
	
	printf("Zahl v:\n");
	scanf("%i", &v);
	
	swap(&u, &v);
	
	printf("wert von u:%i\n", u);
	printf("wert von v:%i\n", v);
	
	return 0;
}

