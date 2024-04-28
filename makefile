CC=gcc -std=gnu11 -Wall -Wextra -g 
#-fsanitize=address -fsanitize=leak -fsanitize=undefined ->ralentit execution
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

%.o: %.c
	$(CC) $(CFLAGS) $< -c

clean:
	rm -f $(OUT) $(OBJ) 
	


