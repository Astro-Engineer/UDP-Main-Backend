CC = g++
CFLAGS = -std=c++11

SERVERMAIN_SRC = servermain.cpp
SERVERA_SRC = serverA.cpp
SERVERB_SRC = serverB.cpp
SERVERC_SRC = serverC.cpp

all: servermain serverA serverB serverC

servermain: $(SERVERMAIN_SRC)
	$(CC) $(CFLAGS) -o servermain $(SERVERMAIN_SRC)

serverA: $(SERVERA_SRC)
	$(CC) $(CFLAGS) -o serverA $(SERVERA_SRC)

serverB: $(SERVERB_SRC)
	$(CC) $(CFLAGS) -o serverB $(SERVERB_SRC)

serverC: $(SERVERC_SRC)
	$(CC) $(CFLAGS) -o serverC $(SERVERC_SRC)

clean:
	rm -f servermain serverA serverB serverC

.PHONY: all clean

