PORT=53975
CFLAGS = -DPORT=\$(PORT) -Wall -g -std=gnu99

all: hcq_server

hcq_server: hcq_server.o hcq.o socket.o rdwrn.o
	gcc ${CFLAGS} -o $@ $^

%.o: %.c hcq.h socket.h rdwrn.h hcq_server.h
	gcc ${CFLAGS} -c $<

clean:
	rm hcq_server *.o





# PORT=53975
# CFLAGS = -DPORT=\$(PORT) -Wall -g -std=gnu99

# all: helpcentre

# helpcentre: helpcentre.o hcq.o
# 	gcc $(CFLAGS) -o helpcentre helpcentre.o hcq.o

# helpcentre.o: helpcentre.c hcq.h
# 	gcc $(CFLAGS) -c helpcentre.c

# hcq.o: hcq.c hcq.h
# 	gcc $(CFLAGS) -c hcq.c

# clean:
# 	rm helpcentre *.o
