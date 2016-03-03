#include "sum_totient_ported.h"
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv) {
	int uppers[] = {
		15000,
		30000,
		100000
	};
	size_t local = 250, i = 4;
	if (argc < 2)
		printf("No local workgroup size given defaulting to 250\n");
	else
		local = atoi(argv[1]);
	if(argc < 3)
		printf("No upper limit was given, defaulting to all\n");
	else
		i = atoi(argv[2]);
	switch (i) {
	case 1:
	case 2:
	case 3:
		test_totient(1, uppers[i], local);
	default:
		for (int i = 0; i < 3; i++)
			test_totient(1, uppers[i], local);
	}
	system("PAUSE");
	return 0;
}

