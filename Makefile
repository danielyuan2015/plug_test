TARGET = plug
#EXEC = ./bin/$(TARGET)
EXEC = ./$(TARGET)

#OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
OBJS = plug.o 
SRC  = $(wildcard *.c)
#PROG_LIB		= logger.a
#SHARED_LIB      = logger.so

CURRENT_PATH		= $(shell pwd)
LOGGER_PATH        = $(CURRENT_PATH)
LOGGER_INCLUDE_PATH = $(LOGGER_PATH)
LOGGER_LIB_PATH = $(LOGGER_PATH)
#COMPILER_PATH = /home/daniel/workspace/imx6solox/arm-poky-toolchain/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi/usr/include
#COMPILER_PATH = ./

#PROG_LIB		= liblogger.a
#SHARED_LIB      = liblogger.so

#CROSS_COMPILE		= arm-poky-linux-gnueabi

#AS			= $(CROSS_COMPILE)-as
#LD			= $(CROSS_COMPILE)-ld
#CC			= $(CROSS_COMPILE)-gcc
#CPP			= $(CC) -E
#AR			= $(CROSS_COMPILE)-ar
#RANLIB		= $(CROSS_COMPILE)-ranlib
#NM			= $(CROSS_COMPILE)-nm
#STRIP		= $(CROSS_COMPILE)-strip
#OBJCOPY		= $(CROSS_COMPILE)-objcopy
#OBJDUMP		= $(CROSS_COMPILE)-objdump

#CFLAGS += -O2 -Wall -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=/home/daniel/workspace/imx6solox/arm-poky-toolchain/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
#CFLAGS += -O2 -Wall  -marm  --sysroot=/home/daniel/workspace/imx6solox/arm-poky-toolchain/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
#LDFLAGS += -I$(LOGGER_INCLUDE_PATH) -I$(COMPILER_PATH) -L$(LOGGER_LIB_PATH) -L$(CURRENT_PATH)
#LDFLAGS += -L$(CURRENT_PATH)

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC)  -o $@ $(OBJS)
	#$(CC) $(LDFLAGS)  -o $@ $(OBJS) -ldcl
#%.o:%.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ -pthread


clean:
	@rm -vf $(EXEC) $(OBJS) *.o *~ *.a