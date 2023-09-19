#pragma once
#include "preload.h"
extern "C" void quantum_interruption_handle(u64 rsp);
extern "C" void timer_isr();
