CC = gcc
CFLAGS = -fsanitize=address
LIBDIR = ./lib/
SOURCEDIR = ./source/

all: client server
	
server: server.c $(LIBDIR)liblist.a $(LIBDIR)libmessenger.a $(LIBDIR)libmessengerserver.a 
	$(CC) -o server server.c -I$(SOURCEDIR) -L$(LIBDIR) -llist -lmessenger -lmessengerserver -lpthread -lm $(CFLAGS)

client: client.c $(LIBDIR)liblist.a $(LIBDIR)libmessenger.a
	$(CC) -o client client.c -I$(SOURCEDIR) -L$(LIBDIR) -llist -lmessenger $(CFLAGS) `pkg-config --cflags --libs gtk+-2.0  gthread-2.0` 

$(LIBDIR)libmessengerserver.a: $(SOURCEDIR)messengerserver.c $(LIBDIR)liblist.a $(LIBDIR)libmessenger.a
	$(CC) -c $(SOURCEDIR)messengerserver.c -o $(SOURCEDIR)messengerserver.o 
	$ ar crv $(LIBDIR)libmessengerserver.a $(SOURCEDIR)messengerserver.o $(SOURCEDIR)list.o $(SOURCEDIR)messenger.o

$(LIBDIR)liblist.a: $(SOURCEDIR)list.c
	$(CC) -c $(SOURCEDIR)list.c -o $(SOURCEDIR)list.o
	$ ar crv $(LIBDIR)liblist.a $(SOURCEDIR)list.o
		
$(LIBDIR)libmessenger.a: $(SOURCEDIR)messenger.c
	$(CC) -c $(SOURCEDIR)messenger.c -o $(SOURCEDIR)messenger.o
	$ ar crv $(LIBDIR)libmessenger.a $(SOURCEDIR)messenger.o

clean:
	rm -f *.o
	rm -f server client
	rm -f $(SOURCEDIR)*.o
	rm -f $(LIBDIR)*.a
