#include <stdio.h>
#include <string.h>

#include "ptp.h"

#define MAX_DATA 64

struct RetData {
	uint16_t code;
	uint16_t type;
	uint32_t transId;

	uint16_t param1;
	uint16_t param2;
	uint16_t param3;
	uint16_t param4;

	uint32_t nparam;

	char data[MAX_DATA];
};

struct RetData decodeBulkContainer(PTPUSBBulkContainer a, int print) {
	if (print) {
		printf("Length: %u\n", a.length);
		printf("Type: %hu\n", a.type);
		printf("Code: %hu (0x%x)\n", a.code, a.code);
		printf("Transfer ID: %u\n", a.trans_id);
		printf("Param: 0x%x\n", a.payload.params.param1);
		printf("Param: 0x%x\n", a.payload.params.param2);
		printf("Param: 0x%x\n", a.payload.params.param3);
		printf("Param: 0x%x\n", a.payload.params.param4);
		printf("Param: 0x%x\n", a.payload.params.param5);
		printf("Payload Data in hex: '");
		for (int i = 0; i < (int)(a.length); i++) {
			printf("%x ", a.payload.data[i]);
		}

		printf("'\n");

		printf("Payload Data: '%s'\n", a.payload.data);
		putchar('\n');
	}

	struct RetData ret = {
		.code = a.code,
		.type = a.type,
		.transId = a.trans_id,
		.param1 = a.payload.params.param1,
		.param2 = a.payload.params.param2,
		.param3 = a.payload.params.param3,
		.param4 = a.payload.params.param4,		
	};

	memcpy(ret.data, a.payload.data, MAX_DATA);

	return ret;
}

struct RetData decodeEventContainer(PTPUSBEventContainer a, int print) {
	if (print) {
		printf("Length: %u\n", a.length);
		printf("Type: %hu\n", a.type);
		printf("Code: %hu (0x%x)\n", a.code, a.code);
		printf("Transfer ID: %u\n", a.trans_id);
		printf("Param: 0x%x\n", a.param1);
		printf("Param: 0x%x\n", a.param2);
		printf("Param: 0x%x\n", a.param3);
		putchar('\n');
	}

	struct RetData ret = {
		.code = a.code,
		.type = a.type,
		.transId = a.trans_id,
		.param1 = a.param1,
		.param2 = a.param2,
		.param3 = a.param3,
	};

	return ret;
}

struct RetData decodeContainer(PTPContainer a, int print) {
	if (print) {
		printf("Code: %hu (0x%x)\n", a.Code, a.Code);
		printf("Session ID: %u\n", a.SessionID);
		printf("Transaction ID: %u\n", a.Transaction_ID);
		printf("Param: 0x%x\n", a.Param1);
		printf("Param: 0x%x\n", a.Param2);
		printf("Param: 0x%x\n", a.Param3);
		printf("Param: 0x%x\n", a.Param4);
		printf("Param: 0x%x\n", a.Param5);
		printf("NParam: %d\n", (char)a.Nparam);
		putchar('\n');
	}

	struct RetData ret = {
		.code = a.Code,
		.type = 0,
		.transId = a.Transaction_ID,
		.nparam = a.Nparam,
		.param1 = a.Param1,
		.param2 = a.Param2,
		.param3 = a.Param3,
	};

	return ret;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		puts("Usage:\n"
			"ptpd <dump_file> <type>\n"
			"types:\n"
			"PTPContainer:\t\tcontainer\n"
			"PTPUSBEventContainer:\tevent\n"
			"PTPUSBBulkContainer:\tbulk");
		return 0;
	}

	FILE *f = fopen(argv[1], "r");
	if (f == NULL) {
		return 1;
	}

	long addr = 0;
	while (1) {
		fseek(f, addr, SEEK_SET);
		int print = 0;

		struct RetData ret;
		if (!strcmp(argv[2], "container")) {
			PTPContainer a;
			if (!fread(&a, 1, sizeof(a), f)) {
				break;
			}

			ret = decodeContainer(a, print);
		} else if (!strcmp(argv[2], "event")) {
			PTPUSBEventContainer a;
			if (!fread(&a, 1, sizeof(a), f)) {
				break;
			}

			ret = decodeEventContainer(a, print);
		} else if (!strcmp(argv[2], "bulk")) {
			PTPUSBBulkContainer a;
			if (!fread(&a, 1, sizeof(a), f)) {
				break;
			}

			ret = decodeBulkContainer(a, print);
		}

		int filter = 1;

		// Look for vendor calls (90* 91*)
		//filter &= ret.code >> 8 == 0x90 || ret.code >> 8 == 0x91;

		filter &= ret.code == 0x900c || ret.code == 0x900d || ret.code == 0x901d;

		// Avoid too high transaction IDs (Typically 0-300)
		filter &= ret.transId < 300;

		if (filter) {
			printf("Packet match: 0x%lx\n", addr);
			printf("Code: 0x%x\n", ret.code);
			printf("Type: 0x%x\n", ret.type);
			printf("Params: %x %x %x %x\n", ret.param1,
				ret.param2, ret.param3, ret.param4);

			if (!strcmp(argv[2], "bulk")) {
				for (int i = 0; i < MAX_DATA; i++) {
					printf("%c:%x  ", (char)ret.data[i], (unsigned char)ret.data[i]);
				}
			}
			puts("\n---------");
		}

		addr++;
	}

	fclose(f);
}
