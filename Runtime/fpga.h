/* FPGA configuration loading. */

#pragma once

#include <ff.h>
#include <stdbool.h>
#include <stdint.h>

bool fpga_configure(FIL *file);
