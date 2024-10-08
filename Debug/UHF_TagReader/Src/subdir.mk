################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../UHF_TagReader/Src/RFIDreader.c 

OBJS += \
./UHF_TagReader/Src/RFIDreader.o 

C_DEPS += \
./UHF_TagReader/Src/RFIDreader.d 


# Each subdirectory must supply rules for building sources it contributes
UHF_TagReader/Src/%.o UHF_TagReader/Src/%.su UHF_TagReader/Src/%.cyclo: ../UHF_TagReader/Src/%.c UHF_TagReader/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/UHF_TagReader/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-UHF_TagReader-2f-Src

clean-UHF_TagReader-2f-Src:
	-$(RM) ./UHF_TagReader/Src/RFIDreader.cyclo ./UHF_TagReader/Src/RFIDreader.d ./UHF_TagReader/Src/RFIDreader.o ./UHF_TagReader/Src/RFIDreader.su

.PHONY: clean-UHF_TagReader-2f-Src

