
-include $(TOPDIR)/Make.defs

CONFIG_[[app_DIR]]_[[app_NAME]]_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_[[app_DIR]]_[[app_NAME]]_STACKSIZE ?= [[app_default_stacksize]]

APPNAME = [[app_name]]
PRIORITY = $(CONFIG_[[app_DIR]]_[[app_NAME]]_PRIORITY)
STACKSIZE = $(CONFIG_[[app_DIR]]_[[app_NAME]]_STACKSIZE)

ASRCS = $(wildcard *.s)
CSRCS = $(filter-out [[app_name]]_main.c,$(wildcard *.c))
MAINSRC = [[app_name]]_main.c

CONFIG_[[app_DIR]]_[[app_NAME]]_PROGNAME ?= [[app_name]]$(EXEEXT)
PROGNAME = $(CONFIG_[[app_DIR]]_[[app_NAME]]_PROGNAME)

include $(APPDIR)/Application.mk
