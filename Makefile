all: intelmetool.c Makefile
	gcc intelmetool.c -o intelmetool -g -lpci

clean:
	rm intelmetool *~
