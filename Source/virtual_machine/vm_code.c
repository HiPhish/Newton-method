#include "vm_code.h"

VMCode vm_code_copy(const VMCode original, int *const error) {
	VMCode copy = {
		.length   = original.length,
		.capacity = original.capacity,
		.code     = malloc(original.capacity * sizeof(uint8_t))
	};

	if (!copy.code || !original.code) {
		if (error) {*error = 1;}
		goto end;
	}
	for (int i = 0; i < copy.length; ++i) {copy.code[i] = original.code[i];}

end:
	return copy;
}

void vm_code_clear(VMCode *const code) {
	free(code->code);
	code->code     = NULL;
	code->length   =    0;
	code->capacity =    0;
}

