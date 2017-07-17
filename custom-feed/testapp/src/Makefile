# CC = mips-openwrt-linux-musl-gcc
# CXX = mips-openwrt-linux-musl-g++
ifeq ($(CC),cc)

CC = gcc
CFLAGS = -DBUILD_USER=\"$(USER)\" -g
TARGET_CXX = g++
TARGET_CXXFLAGS = -std=c++11 -g -O2

else

CXX = mips-openwrt-linux-musl-g++
CFLAGS = -DBUILD_USER=\"$(USER)\" -g -DENABLE_BIG_ENDIAN_BUILD -DENABLE_PRODUCTION
TARGET_CXX = mips-openwrt-linux-musl-g++
TARGET_CXXFLAGS = -std=c++11 -g -O2 -DENABLE_BIG_ENDIAN_BUILD -DENABLE_PRODUCTION

endif
OBJS = recoil.o ping.o utils.o sync.o discovery.o time.o testapp.o event.o
EXECUTABLES = testapp header
all: $(EXECUTABLES)

testapp: $(OBJS)
	$(CXX) $(OBJS) -lpthread -static-libstdc++ -o testapp

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

header:
	$(CC) header.c -o header

clean:
	$(RM) recoil
	$(RM) header
	$(RM) *.o

