SCACHE=attrib.o slotizer.o motive.o scache.o lru.o sim.o main.o
LRU=lru.o sim.o lru_main.o
CFLAGS=--std=c++17 -g
CXX=g++

all: lru scache

lru: $(LRU)
	$(CXX) $^ -o $@
	cp $@ ../../$@

scache: $(SCACHE)
	$(CXX) $^ -o $@
	cp $@ ../../$@

.cpp.o:
	$(CXX) $< -o $@ $(CFLAGS) -c

clean:
	rm lru scache *.o

remake: clean all
