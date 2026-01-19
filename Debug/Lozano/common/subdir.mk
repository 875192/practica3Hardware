################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lozano/common/44blib.c \
../Lozano/common/AscII6x8.c \
../Lozano/common/AscII8x16.c 

ASM_SRCS += \
../Lozano/common/44binit.asm 

OBJS += \
./Lozano/common/44binit.o \
./Lozano/common/44blib.o \
./Lozano/common/AscII6x8.o \
./Lozano/common/AscII8x16.o 

C_DEPS += \
./Lozano/common/44blib.d \
./Lozano/common/AscII6x8.d \
./Lozano/common/AscII8x16.d 

ASM_DEPS += \
./Lozano/common/44binit.d 


# Each subdirectory must supply rules for building sources it contributes
Lozano/common/%.o: ../Lozano/common/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I"C:\hlocal\workspace_Hardware\practica3\common" -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Lozano/common/%.o: ../Lozano/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\hlocal\workspace_Hardware\practica3\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


