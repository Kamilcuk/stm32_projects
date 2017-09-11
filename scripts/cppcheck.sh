#!/bin/bash
set -euo pipefail
# generated using:
# main -n > /tmp/1
# cat /tmp/1 | grep stm32_appinit.c | grep arm-none-eabi-gcc | sed 's/-isystem /-I/' | tr ' ' '\n' | grep "^-I[^\.]"
cppcheck \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/arch/arm/include/ \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/include \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/sched \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/arch/arm/src/chip \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/arch/arm/src/common \
	-I/home/projects/allrepos/stm32_projects/nuttx/nuttx/arch/arm/src/armv7-m \
       	-UCONFIG_ARCH_STDINT_H \
	-UCONFIG_ARCH_STDBOOL_H \
        -UCONFIG_ARCH_DEBUG_H \
	"$@"
