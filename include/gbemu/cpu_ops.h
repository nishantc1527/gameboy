#pragma once
#include <stdint.h>

struct Cpu;
struct Bus;

typedef uint8_t (*CpuOp)(struct Cpu*, struct Bus*);
extern const CpuOp cpu_ops[256];
extern const CpuOp cpu_cb_ops[64];
