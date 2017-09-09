#!kconfig

config [[app_DIR]]_[[app_NAME]]
	bool "[[app_desc]]"
	default n
	---help---
		Enable application residing in "[[app_dir]]/[[app_name]]"

if [[app_DIR]]_[[app_NAME]]

config [[app_DIR]]_[[app_NAME]]_PROGNAME
	string "Program name"
	default "[[app_name]]"
	depends on BUILD_KERNEL
	---help---
		This is the name of the program that will be use when the NSH ELF
		program is installed.

config [[app_DIR]]_[[app_NAME]]_PRIORITY
	int "[[app_name]] task priority"
	default [[app_default_priority]]

config [[app_DIR]]_[[app_NAME]]_STACKSIZE
	int "[[app_name]] stack size"
	default [[app_default_stacksize]]

endif
