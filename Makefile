all: me.h me_status.c intelmetool.c Makefile
	gcc intelmetool.c me_status.c -o intelmetool -g -lpci

clean:
	rm intelmetool *~
