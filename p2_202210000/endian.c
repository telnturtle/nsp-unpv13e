#include <stdio.h>
#include <netinet/in.h>

union {
	uint32_t i;
	uint8_t c[4];
} un;

int main(int argc, char **argv) {
	scanf("%x", &un.i);
	for (int i = 0; i < 4; i++) {
		printf("%x", un.c[i]);
	}
	puts("");
	return 0;
}
