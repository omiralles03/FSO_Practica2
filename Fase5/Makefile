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

all: tron5 oponent5

# Build .c files
$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

tron5: $(SRCDIR)/tron5.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(TRON_LDFLAGS)

oponent5: $(SRCDIR)/oponent5.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./tron5 7 3 $(LOGDIR)/file.log 50 70

clean:
	rm -rf $(BUILDDIR) tron5 oponent5 $(LOGDIR)

