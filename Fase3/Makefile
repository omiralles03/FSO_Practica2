LIBDIR    := library
BUILDDIR  := build
LOGDIR    := logs

BINS      := tron3 oponent3

CFLAGS    := -Wall -I$(LIBDIR)
LDFLAGS   := -lcurses
TRON_LDFLAGS := $(LDFLAGS) -lpthread

SRCS      := $(wildcard $(LIBDIR)/*.c)
OBJS      := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BINS)

$(BUILDDIR)/%.o: $(LIBDIR)/%.c
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

tron3: tron3.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(TRON_LDFLAGS)

oponent3: oponent3.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./tron3 7 3 $(LOGDIR)/file.log 50 70

clean:
	rm -rf $(BUILDDIR) $(BINS) $(LOGDIR)
