all: me.c me.h me_status.c intelmetool.c Makefile
	gcc intelmetool.c me_status.c me.c -o intelmetool -g -lpci

clean:
	rm intelmetool *~
