# Directories
LIBDIR   := libs
SRCDIR   := src
BUILDDIR := build
LOGDIR   := logs

# Compiler Flags
CFLAGS    := -Wall -I$(LIBDIR)
LDFLAGS   := -lcurses
TRON_LDFLAGS := $(LDFLAGS) -lpthread

# Object files from libraries
SRCS_LIB := $(wildcard $(LIBDIR)/*.c)
OBJS     := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS_LIB))

.PHONY: all clean run

all: tron3 oponent3

# Build .c files
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

