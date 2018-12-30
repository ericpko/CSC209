CFLAGS = -Wall -g -std=gnu99

helpcentre: helpcentre.o hcq.o
	gcc $(CFLAGS) -o helpcentre helpcentre.o hcq.o

helpcentre.o: helpcentre.c hcq.h
	gcc $(CFLAGS) -c helpcentre.c

hcq.o: hcq.c hcq.h
	gcc $(CFLAGS) -c hcq.c

clean:
	rm helpcentre *.o
