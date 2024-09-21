# Variables
TIDY=clang-tidy
CC=gcc
AS=nasm
SRC_FILES=isos_inject.c find_pt_note.c code_injection.c overwrite_sh.c reorder_sh.c set_name_sh.c overwrite_pt_note.c hijacking_GOT.c
ASM_FILES=inject.asm
LIBS= -lz -lbfd
INCLUDE_DIR=include
CFLAGS = -Wall -Wextra -pedantic -I$(INCLUDE_DIR)
CLANG_CFLAGS = -Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align
GCC_CFLAGS = -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict

BINDIR=bin
OBJDIR=obj
SRCDIR=src
INJECTDIR=inject
TESTDIR=test
OBJS=$(SRC_FILES:%.c=$(OBJDIR)/%.o) 

TARGET=bin/isos_project
RM=rm -f

vpath %.c src/
vpath %.asm src/

$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(BINDIR))
$(shell mkdir -p $(INJECTDIR))


# Règles
all : $(TARGET) $(INJECTDIR)/inject


$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDE_DIR)/isos_inject.h $(INCLUDE_DIR)/find_pt_note.h $(INCLUDE_DIR)/code_injection.h $(INCLUDE_DIR)/overwrite_sh.h $(INCLUDE_DIR)/reorder_sh.h $(INCLUDE_DIR)/set_name_sh.h $(INCLUDE_DIR)/overwrite_pt_note.h $(INCLUDE_DIR)/hijacking_GOT.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(INJECTDIR)/%: $(SRCDIR)/%.asm
	$(AS) -f bin -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

debug: $(OBJS)
	$(CC) -g -o $(TARGET) $^ $(CFLAGS) $(LIBS)

build_dependencies : $(SRC_FILES:%.c=%.dep)
	@cat $^ > make.test
	@$(RM) $^

%.dep: %.c
	@$(CC) -I$(INCLUDE_DIR) -MM -MF $@ $<

tidy:
	$(TIDY) $(SRCDIR)/*.c -checks=clang-analyzer-*,performance-*,portability-*,readability-*,bugprone-* -- -I./$(INCLUDE_DIR)

# Tester avec clang
test-clang: $(OBJS)
	clang -o $(TESTDIR)/test-clang $^ $(CLANG_CFLAGS) $(LIBS)

# Tester avec gcc
test-gcc: $(OBJS)
	$(CC) -o $(TESTDIR)/test-gcc $^ $(GCC_CFLAGS) $(LIBS)

# Tester avec gcc et l'option -fanalyzer
test-gcc-analyzer: $(OBJS)
	$(CC) -fanalyzer -o $(TESTDIR)/test-gcc-analyzer $^ $(CFLAGS) $(LIBS)

# Tester avec clang et les options -fsanitize
test-clang-asan: $(OBJS)
	clang -o $(TESTDIR)/test-clang-asan $^ -fsanitize=address $(LIBS)

test-clang-msan: $(OBJS)
	clang -o $(TESTDIR)/test-clang-msan $^ -fsanitize=memory $(LIBS)

test-clang-ubsan: $(OBJS)
	clang -o $(TESTDIR)/test-clang-ubsan $^ -fsanitize=undefined $(LIBS)

clean:
	$(RM) $(OBJDIR)/*.o $(INJECTDIR)/* $(TARGET) $(TESTDIR)/* make.test

.PHONY: all debug build_dependencies tidy test-clang test-gcc test-gcc-analyzer test-clang-asan test-clang-msan test-clang-ubsan clean
