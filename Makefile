CC		=	mpicc
CFLAGS	=	-pipe -g3 -ggdb -m64 -std=gnu99 -fopenmp -Wall -Wshadow \
			-Wpointer-arith -Wformat-security -Wclobbered -Wcast-align
PFLAGS	=	-D_FORTIFY_SOURCE=2 -D_GNU_SOURCE 
LDFLAGS	=	-lrt
INCDIR	=	-I. -I./include/

OBJDIR	=	obj
BIN		=	mpi_sorter

SRCS	=	mpi_sorter.c tweet.c mpi_master.c file_io.c caching.c
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
	$(CC) $(OBJS) -o $(BIN)

.PHONY: clean clean-all

clean:
	rm -rf core.* $(OBJS) $(OBJDIR)

clean-all: clean
	rm -rf $(BIN)
	cd doc && make clean-all
