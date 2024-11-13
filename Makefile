CCFLAGS = -O3 -Wall -Wextra -Wcast-qual -fPIC

all: build/testbench build/rtap.so

build/rtap_adpcm.o: src/rtap_adpcm.c src/rtap_adpcm.h src/rtap_river.h src/rtap_spring.h
	$(CC) $(CCFLAGS) -c src/rtap_adpcm.c -o $@

build/rtap_river.o: src/rtap_river.c src/rtap_river.h src/rtap_spring.h
	$(CC) $(CCFLAGS) -c src/rtap_river.c -o $@

build/rtap_spring.o: src/rtap_spring.c src/rtap_spring.h
	$(CC) $(CCFLAGS) -c src/rtap_spring.c -o $@

build/main.o: src/main.c
	$(CC) $(CCFLAGS) -c src/main.c -o $@

build/rtap.so: build/rtap_adpcm.o build/rtap_river.o build/rtap_spring.o
	$(CC) $(CCFLAGS) -shared $^ -fvisibility=hidden -Wl,-soname=rtap.so -o $@

build/testbench: build/main.o build/rtap_adpcm.o build/rtap_river.o build/rtap_spring.o
	$(CC) $(CCFLAGS) $^ -o $@

clean:
	rm -rf build
