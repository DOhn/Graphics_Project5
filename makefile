all:
	gcc -o main texdemo.c -lglfw3 -lpthread

run:
	./main
