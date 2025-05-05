# Directories
LIBDIR   := libs
SRCDIR   := src
BUILDDIR := build

# Compiler Flags
CFLAGS  := -Wall -I$(LIBDIR)
LDFLAGS := -lcurses

# Object files from libraries
SRCS_LIB := $(wildcard $(LIBDIR)/*.c)
OBJS     := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS_LIB))

.PHONY: all clean run

all: tron0

# Build .c files
$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

tron0: $(SRCDIR)/tron0.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	./tron0 3 50

clean:
	rm -rf $(BUILDDIR) tron0

