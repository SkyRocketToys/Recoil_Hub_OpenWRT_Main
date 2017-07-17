#     CC = mips-openwrt-linux-musl-gcc
# CXX = mips-openwrt-linux-musl-g++

ifeq ($(CC),cc)
# We are building for a test system?
CC = gcc
CFLAGS = -Wall -DBUILD_USER=\"$(USER)\" -g
TARGET_CXX = g++
TARGET_CXXFLAGS = -std=c++11 -g -O2

else
# We are building for the target system?
CXX = mips-openwrt-linux-musl-g++
CFLAGS = -DBUILD_USER=\"$(USER)\" -g -DENABLE_BIG_ENDIAN_BUILD -DENABLE_PRODUCTION
TARGET_CXX = mips-openwrt-linux-musl-g++
TARGET_CXXFLAGS = -std=c++11 -g -O2 -DENABLE_BIG_ENDIAN_BUILD -DENABLE_PRODUCTION -minterlink-compressed

endif
OBJS = recoil.o power.o ping.o upgrade.o utils.o sync.o discovery.o time.o recoilapp.o event.o
EXECUTABLES = recoil header
all: $(EXECUTABLES)

recoil: $(OBJS)
	$(MAKE) -C UnityGameNetworkServer \
                CC="$(TARGET_CC)" \
                CXX="$(TARGET_CXX)" \
                CFLAGS="$(TARGET_CFLAGS) -Wall" \
                CXXFLAGS="$(TARGET_CXXFLAGS) -Wall" \
                LDFLAGS="$(TARGET_LDFLAGS)"
	$(MAKE) -C jsoncpp/lib_json \
                CC="$(TARGET_CC)" \
                CXX="$(TARGET_CXX)" \
                CFLAGS="$(TARGET_CFLAGS) -Wall" \
                CXXFLAGS="$(TARGET_CXXFLAGS) -Wall" \
                LDFLAGS="$(TARGET_LDFLAGS)"
	$(CXX) $(OBJS) -L./UnityGameNetworkServer -L./jsoncpp/lib_json -lpthread -lUnityGameNetworkServer -ljson -static-libstdc++ -o recoil

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

header: header.c
	$(CC) header.c -o header

clean:
	$(RM) recoil
	$(RM) header
	$(RM) *.o
	cd UnityGameNetworkServer && make clean
	cd jsoncpp/lib_json && make clean

