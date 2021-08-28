NAME = main

OUTPUT = build

CXX_SOURCES= zcash/Address.cpp \
uint256.cpp \
utilstrencodings.cpp \
zcash/NoteEncryption.cpp \
crypto/sha256.cpp \
zcash/prf.cpp \
support/cleanse.cpp \
main.cpp

C_SOURCES = base58.c

INCLUDES=-Ilibrustzcash/include/ \
-I.

CXXFLAGS = $(INCLUDES)
C_FLAGS = $(INCLUDES)
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS = $(addprefix $(OUTPUT)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
OBJECTS += $(addprefix $(OUTPUT)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += target/release/librustzcash.a \
snark/libsnark.a


LIBS = -lsodium -ldl -lpthread

LDFLAGS = $(LIBS)

all: $(NAME)

snark/libsnark.a:
	make -C snark libsnark.a CPPFLAGS="-Wno-deprecated -DBINARY_OUTPUT -DNO_PT_COMPRESSION=1 -fstack-protector-all" CURVE=ALT_BN128 NO_PROCPS=1 NO_DOCS=1 STATIC=1 NO_SUPERCOP=1 FEATUREFLAGS=-DMONTGOMERY_OUTPUT NO_COPY_DEPINST=1 NO_COMPILE_LIBGTEST=1

target/release/librustzcash.a:
	cargo build  --locked --lib --release

$(OUTPUT)/%.o:%.cpp
	g++ -c $(CXXFLAGS) $< -o $@

$(OUTPUT)/%.o:%.c
	gcc -c $(CFLAGS) $< -o $@

$(NAME):$(OBJECTS)
	g++ $(OBJECTS) $(LDFLAGS) -o $@

run:all
	./$(NAME)

clean:
	rm *.o || true
	rm $(OUTPUT)/*.o || true
	make -C snark clean
	cargo clean