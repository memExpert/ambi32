# STM32F4 family (Cortex-M4F). Input: AMBI_MCU (e.g. STM32F411xE).
# Output: AMBI_FAMILY_* variables used by the top CMakeLists.txt.
# Based on umcub cmake/families/stm32f4.cmake.

set(AMBI_FAMILY_DIR ${AMBI_ROOT}/port/stm32f4)
string(TOLOWER "${AMBI_MCU}" _mcu_lc)

set(AMBI_CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

set(AMBI_FAMILY_INCLUDES
  ${AMBI_FAMILY_DIR}/include
  ${AMBI_ROOT}/port/arch/cortexm
  ${AMBI_ROOT}/third_party/st/cmsis_device_f4/Include
  ${AMBI_ROOT}/third_party/st/stm32f4xx_hal_driver/Inc
  ${AMBI_ROOT}/third_party/cmsis_core/Include
)
set(AMBI_FAMILY_DEFS ${AMBI_MCU} USE_FULL_LL_DRIVER)

set(AMBI_FAMILY_SOURCES
  ${AMBI_FAMILY_DIR}/clock.c
  ${AMBI_FAMILY_DIR}/sys.c
  ${AMBI_ROOT}/port/arch/cortexm/cortexm_sys.c
  ${AMBI_ROOT}/third_party/st/cmsis_device_f4/Source/Templates/system_stm32f4xx.c
  ${AMBI_ROOT}/third_party/st/cmsis_device_f4/Source/Templates/gcc/startup_${_mcu_lc}.s
)
set(AMBI_FAMILY_LOG_SOURCES ${AMBI_FAMILY_DIR}/log_uart.c)

set(AMBI_FAMILY_LD_IN ${AMBI_FAMILY_DIR}/ld/app.ld.in)
