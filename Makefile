FBINK_INC_DIR ?= ./third_party/FBInk
FBINK_LIB_DIR ?= ./third_party/FBInk/lib

APP := myapp
BUILD_DIR := build
APP_PATH := $(BUILD_DIR)/$(APP)

CC ?= arm-nickel-linux-gnueabihf-gcc

CFLAGS += -std=c11 -O2 -Wall -Wextra
CFLAGS += -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=hard -mthumb
CFLAGS += -I$(FBINK_INC_DIR)

LDFLAGS += -Wl,-rpath,/usr/local/Kobo
LDFLAGS += -Wl,-rpath,/usr/local/Qt-5.2.1-arm/lib
LDFLAGS += -L$(FBINK_LIB_DIR)
LDLIBS += -lfbink

SRC := src/main.c

all: $(APP_PATH)

$(APP_PATH): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)
