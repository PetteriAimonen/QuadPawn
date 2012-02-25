
#pragma once
#include <stdbool.h>
#include <stdint.h>

bool get_program_metadata(const char *filename, uint32_t icon[32], int *icon_size, char *name, int name_size);
