#include <stdio.h>
#include "unp.h"
#include <arpa/inet.h>

union {
	struct sockaddr_in s;
	uint8_t c[16];
} un;

int main(int argc, char **argv) {

	in_port_t p;
	uint16_t p_;
	char addr[16];

	printf("port: ");
	scanf("%u", &p_);
	printf("address: ");
	scanf("%s", &addr);

	un.s.sin_family = AF_INET;
	un.s.sin_port = htons(p_);
	inet_aton(addr, &un.s.sin_addr);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%02x ", un.c[4 * i + j]);

		}
		puts("");
	}
	return 0;
};
