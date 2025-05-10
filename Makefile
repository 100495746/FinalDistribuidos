CC = gcc
CFLAGS = -Wall -g
SRC = server.c server_utils.c
OBJ = $(SRC:.c=.o)
EXEC = servidor

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC) -lpthread

run: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(EXEC) *.o