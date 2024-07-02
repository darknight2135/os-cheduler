all:
		gcc -o simpleScheduler simpleScheduler.c 
		gcc -o simpleShell simpleShell.c
	gcc -o fib fib.c
	gcc -o fib2 fib2.c

run:
	./simpleShell

clean:
		rm -f simpleScheduler simpleShell fib fib2
