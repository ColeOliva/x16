CC=gcc
CFLAGS=-I. -g
CPPFLAGS = -I. -g -std=c++17
DEPS = translation.h page_table.h
OBJ = translation.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CPPFLAGS)

default: $(OBJ)
	$(CC) -o translation $^ $(CFLAGS)

clean:
	rm -f *.o *.out

run: default
	./translation

test-build: translation.o test/test_main.o
	g++ -o translation_test $^ $(CPPFLAGS)

test: test-build
	./translation_test

test-to-page: test-build
	./translation_test Translation.ToPage

test-to-offset: test-build
	./translation_test Translation.ToOffset

test-to-address: test-build
	./translation_test Translation.ToAddress

test-lookup-found: test-build
	./translation_test Translation.LookupFound

test-lookup-not-found: test-build
	./translation_test Translation.LookupNotFound

test-order: test-build
	./translation_test Translation.Order

test-load: test-build
	./translation_test Translation.Load

test-lookup-and-load: test-build
	./translation_test Translation.LookupAndLoad
