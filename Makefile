map: map.c
	mkdir bin
	cc map.c -o bin/map

run: map
	./bin/map
