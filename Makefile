
# Build environment can be configured the following
# environment variables:
#   CC : Specify the C compiler to use
#   CFLAGS : Specify compiler options to use

CFLAGS += -I. -DPACKAGE_VERSION=\"1.0.4\"

LIB_HDRS = libtidal.h
LIB_SRCS = astronomical.c constituents.c tidal.c
LIB_OBJS = $(LIB_SRCS:.c=.o)
LIB_A = libtidal.a

all: static tides

LDFLAGS =
LDLIBS = -lm

static: $(LIB_A)

tides: tides.o $(LIB_A)
	$(CC) $(CFLAGS) -o $@ tides.o $(LIB_A) $(LDFLAGS) $(LDLIBS)

# Build static library
$(LIB_A): $(LIB_OBJS) $(LIB_HDRS)
	rm -f $(LIB_A)
	ar -crs $(LIB_A) $(LIB_OBJS)

clean:
	rm -f $(LIB_OBJS) $(LIB_A) tides tides.o

install:
	@echo
	@echo "No install target, copy the library and header as needed"
	@echo


.SUFFIXES: .c .o

# Standard object building
.c.o: $(LIB_HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

