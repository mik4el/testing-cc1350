
XOPTS = -I"/Applications/ti/xdctools_3_32_01_22_core/packages/" -Dxdc_target_types__=/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/ti/targets/arm/elf/std.h -Dxdc_target_name__=M3

vpath % /Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/ti/sysbios/
vpath %.c /Applications/ti/xdctools_3_32_01_22_core/packages/

CCOPTS    = --endian=little -mv7M3 --abi=eabi -q -ms --opt_for_speed=0  --program_level_compile -o3 -g --optimize_with_debug  -Dti_sysbios_knl_Task_minimizeLatency__D=FALSE -Dti_sysbios_family_arm_cc26xx_Boot_driverlibVersion=2  -Dti_sysbios_posix_Settings_debug__D=FALSE -Dti_sysbios_posix_Settings_supportsMutexPriority__D=FALSE -Dti_sysbios_knl_Clock_stopCheckNext__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_enableException__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_disablePriority__D=32U -Dti_sysbios_family_arm_m3_Hwi_numSparseInterrupts__D=0U

XDC_ROOT  = /Applications/ti/xdctools_3_32_01_22_core/packages/

BIOS_ROOT = /Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/ti/sysbios/

BIOS_DEFS =  -Dti_sysbios_BIOS_swiEnabled__D=TRUE -Dti_sysbios_BIOS_taskEnabled__D=TRUE -Dti_sysbios_BIOS_clockEnabled__D=TRUE -Dti_sysbios_BIOS_runtimeCreatesEnabled__D=TRUE -Dti_sysbios_knl_Task_moduleStateCheckFlag__D=FALSE -Dti_sysbios_knl_Task_objectCheckFlag__D=FALSE -Dti_sysbios_hal_Hwi_DISABLE_ALL_HOOKS -Dti_sysbios_knl_Swi_DISABLE_ALL_HOOKS -Dti_sysbios_BIOS_smpEnabled__D=FALSE -Dti_sysbios_Build_useHwiMacros -Dti_sysbios_knl_Swi_numPriorities__D=6 -Dti_sysbios_knl_Task_deleteTerminatedTasks__D=FALSE -Dti_sysbios_knl_Task_numPriorities__D=6 -Dti_sysbios_knl_Task_checkStackFlag__D=FALSE -Dti_sysbios_knl_Task_initStackFlag__D=TRUE -Dti_sysbios_knl_Task_DISABLE_ALL_HOOKS -Dti_sysbios_knl_Clock_TICK_SOURCE=ti_sysbios_knl_Clock_TickSource_TIMER -Dti_sysbios_knl_Clock_TICK_MODE=ti_sysbios_knl_Clock_TickMode_DYNAMIC -Dti_sysbios_hal_Core_delegate_getId=ti_sysbios_hal_CoreNull_getId__E -Dti_sysbios_hal_Core_delegate_interruptCore=ti_sysbios_hal_CoreNull_interruptCore__E -Dti_sysbios_hal_Core_delegate_lock=ti_sysbios_hal_CoreNull_lock__E -Dti_sysbios_hal_Core_delegate_unlock=ti_sysbios_hal_CoreNull_unlock__E -Dti_sysbios_hal_Core_numCores__D=1 -Dti_sysbios_hal_CoreNull_numCores__D=1 -Dti_sysbios_utils_Load_taskEnabled__D=TRUE -Dti_sysbios_utils_Load_swiEnabled__D=FALSE -Dti_sysbios_utils_Load_hwiEnabled__D=FALSE -Dti_sysbios_family_arm_m3_Hwi_dispatcherSwiSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherTaskSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherAutoNestingSupport__D=TRUE -Dti_sysbios_family_arm_m3_Hwi_dispatcherIrpTrackingSupport__D=TRUE -Dti_sysbios_knl_Semaphore_supportsEvents__D=FALSE -Dti_sysbios_knl_Semaphore_supportsPriority__D=FALSE -Dxdc_runtime_Assert_DISABLE_ALL -Dxdc_runtime_Log_DISABLE_ALL -DDEVICE_FAMILY=cc13x0 

BIOS_INC  = -I"/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/"

INCS =  $(BIOS_INC) -I"/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/source/" -I"/Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/" -I"/Applications/ti/ccsv7/ccs_base/" -I"/Applications/ti/xdctools_3_32_01_22_core/packages/" -I"../" -I"/packages//" 

CC = /Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armcl -c $(CCOPTS) -I /Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/include
ASM = /Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armcl -c $(CCOPTS) -I /Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/include
AR = /Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armar rq

DEL = rm -f
CP = cp -f

define RM
	$(if $(wildcard $1),$(DEL) $1,:)
endef

define ASSEMBLE
	@echo asmem3 $< ...
	@$(ASM) $(BIOS_DEFS) $(XOPTS) $(INCS) $<
endef

all: rom_sysbios.aem3

m3_Hwi_asm.obj: family/arm/m3/Hwi_asm.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=m3_Hwi_asm.obj

m3_Hwi_asm_switch.obj: family/arm/m3/Hwi_asm_switch.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=m3_Hwi_asm_switch.obj

m3_IntrinsicsSupport_asm.obj: family/arm/m3/IntrinsicsSupport_asm.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=m3_IntrinsicsSupport_asm.obj

m3_TaskSupport_asm.obj: family/arm/m3/TaskSupport_asm.sv7M makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=m3_TaskSupport_asm.obj


rom_sysbios.obj: BIOS.c knl/Clock.c knl/Idle.c knl/Intrinsics.c knl/Event.c knl/Mailbox.c knl/Queue.c knl/Semaphore.c knl/Swi.c knl/Swi_andn.c knl/Task.c family/arm/m3/Hwi.c family/arm/m3/TaskSupport.c family/arm/cc26xx/Boot.c family/arm/cc26xx/Seconds.c family/arm/cc26xx/Timer.c family/arm/cc26xx/TimestampProvider.c hal/Hwi.c hal/Hwi_stack.c hal/Hwi_startup.c hal/Seconds.c heaps/HeapMem.c posix/pthread.c posix/pthread_barrier.c posix/pthread_cond.c posix/pthread_key.c posix/pthread_mutex.c posix/pthread_rwlock.c posix/clock.c posix/mqueue.c posix/sched.c posix/semaphore.c posix/sleep.c posix/timer.c gates/GateHwi.c gates/GateMutex.c xdc/runtime/xdc_noinit.c xdc/runtime/Assert.c xdc/runtime/Core-mem.c xdc/runtime/Core-smem.c xdc/runtime/Core-label.c xdc/runtime/Core-params.c xdc/runtime/Diags.c xdc/runtime/Error.c xdc/runtime/Gate.c xdc/runtime/Log.c xdc/runtime/Memory.c xdc/runtime/Registry.c xdc/runtime/Startup.c xdc/runtime/System.c xdc/runtime/SysCallback.c xdc/runtime/Text.c xdc/runtime/Timestamp.c makefile
	@-$(call RM, $@)
	@echo clem3 $< ...
	@$(CC) $(BIOS_DEFS) $(XOPTS) $(INCS) --output_file=rom_sysbios.obj \
	$(BIOS_ROOT)BIOS.c \
	$(BIOS_ROOT)knl/Clock.c \
	$(BIOS_ROOT)knl/Idle.c \
	$(BIOS_ROOT)knl/Intrinsics.c \
	$(BIOS_ROOT)knl/Event.c \
	$(BIOS_ROOT)knl/Mailbox.c \
	$(BIOS_ROOT)knl/Queue.c \
	$(BIOS_ROOT)knl/Semaphore.c \
	$(BIOS_ROOT)knl/Swi.c \
	$(BIOS_ROOT)knl/Swi_andn.c \
	$(BIOS_ROOT)knl/Task.c \
	$(BIOS_ROOT)family/arm/m3/Hwi.c \
	$(BIOS_ROOT)family/arm/m3/TaskSupport.c \
	$(BIOS_ROOT)family/arm/cc26xx/Boot.c \
	$(BIOS_ROOT)family/arm/cc26xx/Seconds.c \
	$(BIOS_ROOT)family/arm/cc26xx/Timer.c \
	$(BIOS_ROOT)family/arm/cc26xx/TimestampProvider.c \
	$(BIOS_ROOT)hal/Hwi.c \
	$(BIOS_ROOT)hal/Hwi_stack.c \
	$(BIOS_ROOT)hal/Hwi_startup.c \
	$(BIOS_ROOT)hal/Seconds.c \
	$(BIOS_ROOT)heaps/HeapMem.c \
	$(BIOS_ROOT)posix/pthread.c \
	$(BIOS_ROOT)posix/pthread_barrier.c \
	$(BIOS_ROOT)posix/pthread_cond.c \
	$(BIOS_ROOT)posix/pthread_key.c \
	$(BIOS_ROOT)posix/pthread_mutex.c \
	$(BIOS_ROOT)posix/pthread_rwlock.c \
	$(BIOS_ROOT)posix/clock.c \
	$(BIOS_ROOT)posix/mqueue.c \
	$(BIOS_ROOT)posix/sched.c \
	$(BIOS_ROOT)posix/semaphore.c \
	$(BIOS_ROOT)posix/sleep.c \
	$(BIOS_ROOT)posix/timer.c \
	$(BIOS_ROOT)gates/GateHwi.c \
	$(BIOS_ROOT)gates/GateMutex.c \
	$(XDC_ROOT)xdc/runtime/xdc_noinit.c \
	$(XDC_ROOT)xdc/runtime/Assert.c \
	$(XDC_ROOT)xdc/runtime/Core-mem.c \
	$(XDC_ROOT)xdc/runtime/Core-smem.c \
	$(XDC_ROOT)xdc/runtime/Core-label.c \
	$(XDC_ROOT)xdc/runtime/Core-params.c \
	$(XDC_ROOT)xdc/runtime/Diags.c \
	$(XDC_ROOT)xdc/runtime/Error.c \
	$(XDC_ROOT)xdc/runtime/Gate.c \
	$(XDC_ROOT)xdc/runtime/Log.c \
	$(XDC_ROOT)xdc/runtime/Memory.c \
	$(XDC_ROOT)xdc/runtime/Registry.c \
	$(XDC_ROOT)xdc/runtime/Startup.c \
	$(XDC_ROOT)xdc/runtime/System.c \
	$(XDC_ROOT)xdc/runtime/SysCallback.c \
	$(XDC_ROOT)xdc/runtime/Text.c \
	$(XDC_ROOT)xdc/runtime/Timestamp.c \

rom_sysbios.aem3: rom_sysbios.obj m3_Hwi_asm.obj m3_Hwi_asm_switch.obj m3_IntrinsicsSupport_asm.obj m3_TaskSupport_asm.obj
	@-$(call RM, $@)
	@echo arem3 $^ ...
	@$(AR) $@ $^ /Users/mikaelandersson/ti/tirex-content/simplelink_cc13x0_sdk_1_00_00_13/kernel/tirtos/packages/ti/sysbios/rom/cortexm/cc13xx/golden/CC13xx/rom_sysbios_config.obj

clean:
	@$(DEL) ../makefile.libs
	@-$(call RM, *)
