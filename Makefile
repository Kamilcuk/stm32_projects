#!make
## create links inside nuttx directory

DEBUG:=true
BOARD:=kc-stm32f103-minimum/usbnsh

# 1366:0101 is the hex of jlink
has_jlink=$(shell lsusb | cut -d' ' -f6 | grep -x "1366:0101")

ifeq ($(DEBUG),true)
set_CONFIG_DEBUG_SYMBOLS=y
else
set_CONFIG_DEBUG_SYMBOLS=n
endif



src_apps_dir=src/apps
nuttx_apps_dir=nuttx/apps

src_configs_dir=src/nuttx/configs
nuttx_dir=nuttx/nuttx
nuttx_configs_dir=$(nuttx_dir)/configs
nuttx_config=$(nuttx_dir)/.config
nuttx_edit_config=scripts/edit_kernel_config.sh -c $(nuttx_config)

.PHONY: prebuild flash st-flash jlink configure_board TODO
all: nuttx

# flashing targets ##################################################
# st-flash utility for flashing using st-link
st-flash-hex: nuttx/nuttx/nuttx.hex
	st-flash --reset --format ihex write $<
st-flash-bin: nuttx/nuttx/nuttx.bin
	st-flash --reset write $< 0x08000000

# JLinkExe utility for flashing using jlink
jlink-flash-hex: nuttx/nuttx/nuttx.bin
	echo -ne "si 1\nloadfile $<, 0x08000000\nr\ng\nexit\n" | JLinkExe -device STM32F103C8 -if SWD -speed 4000 -autoconnect 1
jlink-flash-bin: nuttx/nuttx/nuttx.bin
	echo -ne "si 1\nloadfile $<, 0x08000000\nr\ng\nexit\n" | JLinkExe -device STM32F103C8 -if SWD -speed 4000 -autoconnect 1

# proxy rule, depending if jlink or stlink is connected
ifeq ($(has_jlink),)
flash: st-flash-hex
else
flash: jlink-flash-hex
endif

# configure #########################################################

configure_board: distclean prebuild nuttx/nuttx/.config
	# todo: realpath
nuttx/nuttx/.config:
	$(foreach path,$(wildcard $(src_apps_dir)/*),\
		ln -f -s ../../$(path) $(nuttx_apps_dir) )
	$(foreach path,$(wildcard $(src_configs_dir)/*),\
		ln -f -s ../../../$(path) $(nuttx_configs_dir) )
	cd nuttx/nuttx/tools && ./configure.sh $(BOARD)
prebuild: nuttx/nuttx/.config TODO
	$(nuttx_edit_config) CONFIG_DEBUG_SYMBOLS $(set_CONFIG_DEBUG_SYMBOLS)
	$(nuttx_edit_config) CONFIG_INTELHEX_BINARY y
	$(nuttx_edit_config) CONFIG_START_DAY   $(shell date +%-d)
	$(nuttx_edit_config) CONFIG_START_MONTH $(shell date +%-m)
	$(nuttx_edit_config) CONFIG_START_YEAR  $(shell date +%-Y)

# proxy rules to nuttx kernel #########################################
nuttx/nuttx/%: nuttx prebuild
	$(MAKE) -C nuttx/nuttx $(subst nuttx/nuttx/,,$@)
%config: prebuild
	$(MAKE) -C nuttx/nuttx $@
all nuttx clean distclean: prebuild
	$(MAKE) -C nuttx/nuttx $@

