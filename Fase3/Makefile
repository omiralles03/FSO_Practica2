LIBDIR   := library
SRCDIR   := src
BUILDDIR := build
LOGDIR   := logs

CFLAGS    := -Wall -I$(LIBDIR)
LDFLAGS   := -lcurses
TRON_LDFLAGS := $(LDFLAGS) -lpthread

# ara game.o, memoria.o, semafor.o, winsuport2.o
OBJS     := $(BUILDDIR)/game.o \
            $(BUILDDIR)/memoria.o \
            $(BUILDDIR)/semafor.o \
            $(BUILDDIR)/winsuport2.o

.PHONY: all clean run

all: tron3 oponent3

$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

tron3: $(SRCDIR)/tron3.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(TRON_LDFLAGS)

oponent3: $(SRCDIR)/oponent3.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./tron3 7 3 $(LOGDIR)/file.log 50 70

clean:
	rm -rf $(BUILDDIR) tron3 oponent3 $(LOGDIR)

