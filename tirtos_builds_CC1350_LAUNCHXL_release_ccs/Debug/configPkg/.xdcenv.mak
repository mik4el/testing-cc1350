#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/source;/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages;/Applications/ti/ccsv7/ccs_base
override XDCROOT = /Applications/ti/xdctools_3_32_01_22_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/source;/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages;/Applications/ti/ccsv7/ccs_base;/Applications/ti/xdctools_3_32_01_22_core/packages;..
HOSTOS = MacOS
endif
