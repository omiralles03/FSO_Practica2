# Directories
LIBDIR   := libs
SRCDIR   := src
BUILDDIR := build
LOGDIR   := logs

# Compiler Flags
CFLAGS  := -Wall -I$(LIBDIR)
LDFLAGS := -lcurses

# Object files from libraries
SRCS_LIB := $(wildcard $(LIBDIR)/*.c)
OBJS     := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS_LIB))

.PHONY: all clean run

all: tron2

# Build .c files
$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

tron2: $(SRCDIR)/tron2.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./tron2 7 3 $(LOGDIR)/file.log 50 70

clean:
	rm -rf $(BUILDDIR) tron2 $(LOGDIR)

