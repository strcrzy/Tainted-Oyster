# OK, you should modify these line so that they reflect 
# the directories of glib.h and glibconfig.h
GLIBFLAGS=`pkg-config --cflags --libs  glib-2.0`
CFLAGS=-Wall $(GLIBFLAGS) -Itests -Iheaders -Isrc
DEBUG_FLAGS=-g3
PROFILE_FLAGS= -fprofile-arcs -ftest-coverage -pg
TARGET=tainted

all:
	gcc $(CFLAGS) src/$(TARGET).c  -o $(TARGET)  

debug:
	gcc $(CFLAGS) -o src/$(TARGET) $(TARGET).c $(DEBUG_FLAGS)

profile:
	gcc $(CFLAGS) src/$(TARGET).c -o $(TARGET) $(PROFILE_FLAGS) 

test:
	@gcc $(CFLAGS) tests/test_all.c -o testes $(DEBUG_FLAGS)
	@G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind -q ./testes 
#	@rm testes

valgrind-heap:
	@gcc $(CFLAGS) tests/test_all.c -o testes $(DEBUG_FLAGS)
	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full --show-reachable=yes ./testes
	@rm ./testes

valgrind-test:
	@gcc $(CFLAGS) tests/test_all.c -o testes $(DEBUG_FLAGS)
	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --num-callers=20 ./testes
	@rm ./testes
