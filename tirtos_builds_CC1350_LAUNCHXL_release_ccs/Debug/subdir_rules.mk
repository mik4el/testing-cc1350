################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
build-62969583:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-62969583-inproc

build-62969583-inproc: ../release.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"/Applications/ti/xdctools_3_32_01_22_core/xs" --xdcpath="/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/source;/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages;/Applications/ti/ccsv7/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1350F128 -r release -c "/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS" --compileOptions " -DDEVICE_FAMILY=cc13x0 " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-62969583
configPkg/compiler.opt: build-62969583
configPkg/: build-62969583


