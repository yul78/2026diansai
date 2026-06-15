################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-962488254: ../empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"E:/2026diansai/sysconfig_1.26.2/sysconfig_cli.bat" -s "E:/2026diansai/mspm0_sdk_2_10_00_04/.metadata/product.json" --script "E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-962488254 ../empty.syscfg
device.opt: build-962488254
device.cmd.genlibs: build-962488254
ti_msp_dl_config.c: build-962488254
ti_msp_dl_config.h: build-962488254
Event.dot: build-962488254

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: E:/2026diansai/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


