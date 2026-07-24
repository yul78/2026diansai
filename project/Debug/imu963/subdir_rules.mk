################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
imu963/%.o: ../imu963/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"imu963/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


