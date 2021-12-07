TARGET_EXEC ?= histo

OPT ?= -O3 -ftree-vectorize 
GCC ?= gcc

CFLAGS ?= -MMD -MP -g $(OPT) -Wall -std=c11 -D_GNU_SOURCE

LDFLAGS ?= -lm -pthread


FILE = $(shell basename "$$PWD").tar.gz
TARGETS = histo.c cookie.txt comment.txt
SERVER = lnxsrv07
PORT = 15214
API = http://${SERVER}.seas.ucla.edu:${PORT}/upload

$(TARGET_EXEC): main.o histo.o histo-check.o
	$(GCC) $^ -o $@ $(LDFLAGS)

# Add this option "-fopt-info-vec-all" to see the compiler's vectorization report!! It's so much fun.
histo.o: histo.c histo.h
	$(GCC) $(CFLAGS) $(OPT) -c $< -o $@

main.o: main.c main.h histo.h
	$(GCC) $(CFLAGS) $(OPT) -c $< -o $@

submit: build
		echo 'sending request to the server...'
		echo "it might take a few seconds..."
		curl -F 'file=@$(FILE)' $(API)

.SILENT:
build:
		@tar -czvf $(FILE) $(TARGETS) 2>/dev/null

.PHONY: clean

clean:
	$(RM) -r $(TARGET_EXEC) histo.o main.o