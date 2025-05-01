all : winsuport.o tron0

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

tron0 : tron0.c winsuport.o winsuport.h
	gcc -Wall tron0.c winsuport.o -o tron0 -lcurses

run : tron0
	./tron0 3 50

clean: 
	rm winsuport.o tron0 tron1
