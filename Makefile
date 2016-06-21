OBJS = main.o aeye.o
CFLAGS = -O2 -std=c++11 -I. -I/usr/local/include -I/usr/include/ -fpermissive
CFLAGS := -g -Os -lm -DFPGA_REG_02 -fpermissive
all: $(OBJS)
	g++ $(CFLAGS) -o a.out $(OBJS)

main.o: main.cpp align68.hpp
	g++ $(CFLAGS) -c -o main.o main.cpp

aeye.o: AeyeNet.cpp AeyeNet.hpp
	g++ $(CFLAGS) -c -o aeye.o AeyeNet.cpp

clean:
	rm -rf *.o
	rm -rf *.out
