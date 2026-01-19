################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Jaime/common/44blib.c \
../Jaime/common/AscII6x8.c \
../Jaime/common/AscII8x16.c 

ASM_SRCS += \
../Jaime/common/44binit.asm 

OBJS += \
./Jaime/common/44binit.o \
./Jaime/common/44blib.o \
./Jaime/common/AscII6x8.o \
./Jaime/common/AscII8x16.o 

C_DEPS += \
./Jaime/common/44blib.d \
./Jaime/common/AscII6x8.d \
./Jaime/common/AscII8x16.d 

ASM_DEPS += \
./Jaime/common/44binit.d 


# Each subdirectory must supply rules for building sources it contributes
Jaime/common/%.o: ../Jaime/common/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I"C:\hlocal\workspace_Hardware\practica3\common" -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Jaime/common/%.o: ../Jaime/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\hlocal\workspace_Hardware\practica3\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


