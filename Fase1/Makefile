all : winsuport.o tron1 

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

tron1 : tron1.c winsuport.o winsuport.h
	gcc -Wall tron1.c winsuport.o -o tron1 -lcurses

run: tron1
	./tron1 7 3 file.log 50 100

clean: 
	rm winsuport.o tron1
