CC := gcc
CFLAG := -Wall -O2

INC := \
-I./libbmp

SRC := \
./cmos2bmp.c \
./libbmp/libbmp.c

OPT := $(CFLAG) $(INC)

all: init_submodule cmos2bmp


init_submodule:
	git submodule init
	git submodule update

cmos2bmp: $(SRC)
	$(CC) $(OPT) $^ -o $@