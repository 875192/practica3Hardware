################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lozano/8led.c \
../Lozano/Bmp.c \
../Lozano/button.c \
../Lozano/calibracion.c \
../Lozano/cola.c \
../Lozano/lcd.c \
../Lozano/led.c \
../Lozano/main_sudoku.c \
../Lozano/sudoku_2025.c \
../Lozano/sudoku_lcd.c \
../Lozano/tecladoMat.c \
../Lozano/timer.c \
../Lozano/timer1.c \
../Lozano/timer2.c \
../Lozano/tp_simple.c 

OBJS += \
./Lozano/8led.o \
./Lozano/Bmp.o \
./Lozano/button.o \
./Lozano/calibracion.o \
./Lozano/cola.o \
./Lozano/lcd.o \
./Lozano/led.o \
./Lozano/main_sudoku.o \
./Lozano/sudoku_2025.o \
./Lozano/sudoku_lcd.o \
./Lozano/tecladoMat.o \
./Lozano/timer.o \
./Lozano/timer1.o \
./Lozano/timer2.o \
./Lozano/tp_simple.o 

C_DEPS += \
./Lozano/8led.d \
./Lozano/Bmp.d \
./Lozano/button.d \
./Lozano/calibracion.d \
./Lozano/cola.d \
./Lozano/lcd.d \
./Lozano/led.d \
./Lozano/main_sudoku.d \
./Lozano/sudoku_2025.d \
./Lozano/sudoku_lcd.d \
./Lozano/tecladoMat.d \
./Lozano/timer.d \
./Lozano/timer1.d \
./Lozano/timer2.d \
./Lozano/tp_simple.d 


# Each subdirectory must supply rules for building sources it contributes
Lozano/%.o: ../Lozano/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\hlocal\workspace_Hardware\practica3\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


