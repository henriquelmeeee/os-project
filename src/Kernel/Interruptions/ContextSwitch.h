#pragma once
#include "preload.h"
void __attribute__((interrupt)) quantum_interruption_handle(InterruptFrame* args);
