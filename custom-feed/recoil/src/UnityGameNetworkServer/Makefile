#     CXX=mips-openwrt-linux-musl-g++
# CXXFLAGS = -std=c++11 -g -O2 -DENABLE_BIG_ENDIAN_BUILD -DENABLE_PRODUCTION
OBJS = GameEvents.o GameNetworkServer.o GameNetworkServerWrapper.o GamePlayerState.o GameState.o RPCNetworkClient.o Vector3.o
STATLIB = libUnityGameNetworkServer.a

all: $(STATLIB)

$(STATLIB): $(OBJS)
	ar rcs $@ $^

GameEvents.o:
	$(CXX) $(CXXFLAGS) -c GameEvents.cpp -o GameEvents.o

GameNetworkServer.o:
	$(CXX) $(CXXFLAGS) -c GameNetworkServer.cpp -o GameNetworkServer.o

GameNetworkServerWrapper.o:
	$(CXX) $(CXXFLAGS) -c GameNetworkServerWrapper.cpp -o GameNetworkServerWrapper.o

GamePlayerState.o:
	$(CXX) $(CXXFLAGS) -c GamePlayerState.cpp -o GamePlayerState.o

GameState.o:
	$(CXX) $(CXXFLAGS) -c GameState.cpp -o GameState.o

RPCNetworkClient.o:
	$(CXX) $(CXXFLAGS) -c RPCNetworkClient.cpp -o RPCNetworkClient.o

Vector3.o:
	$(CXX) $(CXXFLAGS) -c Vector3.cpp -o Vector3.o

testboost:
	g++ -std=c++11 -O testboost.cpp -o testboost

testwstring:
	g++ -std=c++11 -O testwstring.cpp -o testwstring

clean:
	$(RM) *.o
	$(RM) *.a
