LIBDIR := library
BUILDDIR := build
LOGDIR := logs

BIN := tron0
CFLAGS := -Wall -I$(LIBDIR)
LDFLAGS := -lcurses

SRCS := $(wildcard $(LIBDIR)/*.c)
OBJS := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BIN)

$(BUILDDIR)/%.o: $(LIBDIR)/%.c
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

$(BIN): tron0.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./$(BIN) 3 50

clean:
	rm -rf $(BUILDDIR) $(BIN)
