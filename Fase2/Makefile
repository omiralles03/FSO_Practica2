all : winsuport.o tron2

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

memoria.o : memoria.c memoria.h
	gcc -Wall -c memoria.c -o memoria.o

semafor.o : semafor.c semafor.h
	gcc -Wall -c semafor.c -o semafor.o

tron2 : tron2.c winsuport.o winsuport.h memoria.o memoria.h semafor.o semafor.h
	gcc -Wall tron2.c winsuport.o memoria.o semafor.o -o tron2 -lcurses

run: tron2
	./tron2 7 3 file.log 50 70

clean: 
	rm winsuport.o memoria.o semafor.o tron2
