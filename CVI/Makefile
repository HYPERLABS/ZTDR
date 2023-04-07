CC = gcc
CFLAGS = -fPIC -Wall -Wextra -g -Wno-unused-but-set-variable
LIB_LDFLAGS = -shared -lm -lftd2xx
TEST_LDFLAGS = -L. -lztdr
RM = rm -f
LIB_TARGET = libztdr.so
LIB_SRCS = ZTDR_2XX.c
TEST_TARGET = ztdr_test
TEST_SRCS = ZTDR_2XX_test.c

.PHONY: all
all: ${LIB_TARGET} $(TEST_TARGET)

$(LIB_TARGET): $(LIB_SRCS)
	$(CC) ${CFLAGS} $^ -o $@ ${LIB_LDFLAGS}

$(TEST_TARGET): $(TEST_SRCS)
	$(CC) ${CFLAGS} $^ -o $@ ${TEST_LDFLAGS}


.PHONY: clean
clean:
	-${RM} ${LIB_TARGET} ${LIB_OBJS} ${TEST_TARGET} ${TEST_OBJS}

