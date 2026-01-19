################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Jaime/8led.c \
../Jaime/Bmp.c \
../Jaime/button.c \
../Jaime/cola.c \
../Jaime/lcd.c \
../Jaime/led.c \
../Jaime/main.c \
../Jaime/sudoku_2025.c \
../Jaime/timer.c \
../Jaime/timer1.c \
../Jaime/timer2.c \
../Jaime/timer3.c \
../Jaime/tp.c 

ASM_SRCS += \
../Jaime/init_b.asm 

OBJS += \
./Jaime/8led.o \
./Jaime/Bmp.o \
./Jaime/button.o \
./Jaime/cola.o \
./Jaime/init_b.o \
./Jaime/lcd.o \
./Jaime/led.o \
./Jaime/main.o \
./Jaime/sudoku_2025.o \
./Jaime/timer.o \
./Jaime/timer1.o \
./Jaime/timer2.o \
./Jaime/timer3.o \
./Jaime/tp.o 

C_DEPS += \
./Jaime/8led.d \
./Jaime/Bmp.d \
./Jaime/button.d \
./Jaime/cola.d \
./Jaime/lcd.d \
./Jaime/led.d \
./Jaime/main.d \
./Jaime/sudoku_2025.d \
./Jaime/timer.d \
./Jaime/timer1.d \
./Jaime/timer2.d \
./Jaime/timer3.d \
./Jaime/tp.d 

ASM_DEPS += \
./Jaime/init_b.d 


# Each subdirectory must supply rules for building sources it contributes
Jaime/%.o: ../Jaime/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\hlocal\workspace_Hardware\practica3\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Jaime/%.o: ../Jaime/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I"C:\hlocal\workspace_Hardware\practica3\common" -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


