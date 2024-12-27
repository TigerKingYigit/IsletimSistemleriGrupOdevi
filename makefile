all: main increment

main: main.c
	gcc -o main main.c

increment: increment.c
	gcc -o increment increment.c

clean:
	rm -f main increment