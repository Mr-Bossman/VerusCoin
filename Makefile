NAME = main

OUTPUT = build

CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

WPATH =  $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

HOST = $(shell gcc -v 2> /dev/stdout | awk  -e '/Target:/ {print $$2}')
PREFIX = arm-none-eabi

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)-gcc
CXX = $(GCC_PATH)/$(PREFIX)-g++
AS = $(GCC_PATH)/$(PREFIX)-gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)-objcopy
SZ = $(GCC_PATH)/$(PREFIX)-size
else
CC = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
AS = $(PREFIX)-gcc -x assembler-with-cpp
CP = $(PREFIX)-objcopy
SZ = $(PREFIX)-size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
AR = $(PREFIX)-ar



CXX_SOURCES= zcash/Address.cpp \
uint256.cpp \
utilstrencodings.cpp \
zcash/NoteEncryption.cpp \
crypto/sha256.cpp \
zcash/prf.cpp \
support/cleanse.cpp \
run.cpp

C_SOURCES = base58.c

INCLUDES=-Ilibrustzcash/include/ \
-I. \
-I./include \
-I./sodium/include

DEFINES =
CXXFLAGS = $(INCLUDES) $(DEFINES) $(MCU) -MMD -MP -fpermissive
CFLAGS = $(INCLUDES) $(DEFINES) $(MCU) -MMD -MP
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS = $(addprefix $(OUTPUT)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
OBJECTS += $(addprefix $(OUTPUT)/,$(notdir $(C_SOURCES:.c=.o)))
STATIC = target/thumbv7em-none-eabihf/debug/librustzcash.a \
snark/libsnark.a \
libsodium/libsodium.a


LIBS =
LDFLAGS = $(LIBS)


all: libs

dirs:
	mkdir libs 2> /dev/null || true
	mkdir sodium 2> /dev/null || true
	mkdir include 2> /dev/null || true
	mkdir build 2> /dev/null || true
#cp -r /usr/include/boost/ include/

snark/libsnark.a:
	$(MAKE) -C snark libsnark.a CPPFLAGS="-Wno-deprecated -DBINARY_OUTPUT -DNO_PT_COMPRESSION=1 -fstack-protector-all" CURVE=ALT_BN128 NO_PROCPS=1 NO_DOCS=1 STATIC=1 NO_SUPERCOP=1 FEATUREFLAGS=-DMONTGOMERY_OUTPUT NO_COPY_DEPINST=1 NO_COMPILE_LIBGTEST=1

target/thumbv7em-none-eabihf/debug/librustzcash.a:
	cargo build --lib --target thumbv7em-none-eabihf -Z build-std=core,std,panic_abort --no-default-features -j10
	$(PREFIX)-ar t target/thumbv7em-none-eabihf/debug/librustzcash.a | grep compiler_builtins | xargs -n 15 -I % $(PREFIX)-ar dv target/thumbv7em-none-eabihf/debug/librustzcash.a %

libsodium/libsodium.a:
	#$(shell cd libsodium && ./configure --host=$(PREFIX)  --prefix=$(WPATH)sodium CFLAGS='$(CFLAGS)' LDFLAGS='--specs=nosys.specs')
	$(MAKE) -C libsodium install


$(OUTPUT)/%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OUTPUT)/%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

libs:dirs $(STATIC) $(NAME).a
	rm libs/*.a || true
	mv target/thumbv7em-none-eabihf/debug/librustzcash.a libs || true
	mv snark/libsnark.a libs || true
	mv sodium/lib/libsodium.a libs || true
	mv $(NAME).a libs || true

$(NAME).a:$(OBJECTS)
	$(AR) rcs $(LDFLAGS) $@ $^


clean:
	rm *.o || true
	rm libs/*.a || true
	rm *.a || true
	rm $(OUTPUT)/*.o || true
	rm $(OUTPUT)/*.a || true
	make -C snark clean
	make -C libsodium clean
	cargo clean