# CXX=mips-openwrt-linux-musl-g++
# CXXFLAGS = -std=c++11 -g -O2 -I../include
OBJS = json_reader.o json_value.o json_writer.o
STATLIB = libjson.a

all: $(STATLIB)

$(STATLIB): $(OBJS)
	ar rcs $@ $^

json_reader.o:
	$(CXX) $(CXXFLAGS) -c json_reader.cpp -o json_reader.o

json_value.o:
	$(CXX) $(CXXFLAGS) -c json_value.cpp -o json_value.o

json_writer.o:
	$(CXX) $(CXXFLAGS) -c json_writer.cpp -o json_writer.o

clean:
	$(RM) *.o
	$(RM) *.a
