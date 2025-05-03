LIBDIR := library
BUILDDIR := build
LOGDIR := logs

BIN := tron3
CFLAGS := -Wall -I$(LIBDIR)
LDFLAGS := -lcurses

SRCS := $(wildcard $(LIBDIR)/*.c)
OBJS := $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BIN)

$(BUILDDIR)/%.o: $(LIBDIR)/%.c
	@mkdir -p $(BUILDDIR)
	gcc $(CFLAGS) -c $< -o $@

$(BIN): tron3.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@mkdir -p $(LOGDIR)
	./$(BIN) 7 3 $(LOGDIR)/file.log 50 70

clean:
	rm -rf $(BUILDDIR) $(BIN)
