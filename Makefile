CXX_SOURCES= zcash/Address.cpp \
uint256.cpp \
utilstrencodings.cpp \
zcash/NoteEncryption.cpp \
crypto/sha256.cpp \
zcash/prf.cpp \
support/cleanse.cpp \
main.cpp


INCLUDES=-Ilibrustzcash/include/ \
-I.

CXXFLAGS = $(INCLUDES)

vpath %.cpp $(sort $(dir $(CXX_SOURCES)))
OBJECTS = $(notdir $(CXX_SOURCES:.cpp=.o))
OBJECTS += target/release/librustzcash.a \
snark/libsnark.a

LIBS = -lsodium -ldl -lcrypto -lpthread

LDFLAGS = $(LIBS)

all: main

snark/libsnark.a:
	make -C snark libsnark.a CXXFLAGS="-DBINARY_OUTPUT -DNO_PT_COMPRESSION=1 -fstack-protector-all" CURVE=ALT_BN128 NO_PROCPS=1 NO_DOCS=1 STATIC=1 NO_SUPERCOP=1 FEATUREFLAGS=-DMONTGOMERY_OUTPUT NO_COPY_DEPINST=1 NO_COMPILE_LIBGTEST=1

target/release/librustzcash.a:
	cargo build  --locked --lib --release

%.o:%.cpp
	g++ -c $(CXXFLAGS) $< -o $@

main: $(OBJECTS) 
	g++ $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm *.o
	make -C snark
	cargo clean

#g++ -I. -Izcash/Address.cpp  uint256.cpp utilstrencodings.cpp zcash/NoteEncryption.cpp crypto/sha256.cpp zcash/prf.cpp support/cleanse.cpp snark/libsnark.a 