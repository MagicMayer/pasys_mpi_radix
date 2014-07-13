# CC		=	/usr/lib64/openmpi/bin/mpicc
CC		=	mpicc
CFLAGS	=	-pipe -g3 -ggdb -m64 -std=c99 -fopenmp -Wall -Wextra \
			-Wpedantic -Wshadow -Wpointer-arith -Wclobbered 
PFLAGS	=	-D_GNU_SOURCE -D_XOPEN_SOURCE=600
LDFLAGS	=	-lrt -lm 
INCDIR	=	-I. -I./include/

OBJDIR	=	obj
BIN		=	mpi_sorter

SRCS	=	mpi_sorter.c tweet.c pasys_mpi.c file_io.c caching.c radix.c
OBJS	:=	$(SRCS:.c=.o)
OBJS	:=	$(addprefix $(OBJDIR)/,$(OBJS))

TARGETS	=	$(BIN)

vpath %.c src
vpath %.h include

.FAILED:
	@echo Make of `pwd` failed


all: $(OBJDIR) $(TARGETS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o : %.c
	$(CC) $(PFLAGS) $(INCDIR) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -fopenmp -o $(BIN)

.PHONY: clean clean-all

clean:
	rm -rf core.* $(OBJS) $(OBJDIR)

clean-all: clean
	rm -rf $(BIN)
	cd doc && make clean-all
