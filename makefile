CC=gcc -std=gnu11 -Wall -Wextra -g -O3
# -fsanitize=address -fsanitize=leak -fsanitize=undefined -O1
CFLAGS=
LIBS=-lm -lrt -lpthread 
SRCS=$(wildcard *.c)
OUT= main
FILENAMES=$(basename $(SRCS))
OBJ=$(addsuffix .o,$(FILENAMES)) 

.SUFFIXES:

all: $(OUT) 

$(OUT):$(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) $< -c

clean:
	rm -f $(OUT) $(OBJ) 

run:
	./main ./wi29.tsp 8 7	


