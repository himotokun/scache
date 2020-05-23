OBJ=table.o config.o generator.o main.o
EXEC=generator
CFLAGS=--std=c++17 -g
CXX=g++

all:$(EXEC)
	cp $(EXEC) ../../$(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $^ -o $@

.cpp.o:
	$(CXX) $< -o $@ $(CFLAGS) -c

clean:
	rm $(EXEC) $(OBJ)

remake: clean all
