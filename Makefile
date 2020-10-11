CC=gcc
CFLAGS=-g -Wall
LDLIBS = -lreadline
EXEC=tsh
TEST=tsh_test
SRC=src/
TARGET=target/
MAIN_DIR=main/
TEST_DIR=test/
INCLUDE=$(SRC)include/
TEST_INCLUDE=$(SRC)test_include/
MAIN_FILES=$(wildcard $(SRC)$(MAIN_DIR)*.c)
OBJS=$(notdir $(MAIN_FILES:.c=.o))
OBJS:=$(addprefix $(TARGET)$(MAIN_DIR), $(OBJS))
OBJS_NO_MAIN=$(filter-out $(TARGET)$(MAIN_DIR)main.o, $(OBJS))
TEST_FILES=$(wildcard $(SRC)$(TEST_DIR)*.c)
TEST_OBJS=$(notdir $(TEST_FILES:.c=.o))
TEST_OBJS:=$(addprefix $(TARGET)$(TEST_DIR), $(TEST_OBJS))


all: $(EXEC) $(TEST)

$(EXEC): $(OBJS)
	@$(CC) -I $(INCLUDE) $(CFLAGS) -o $(EXEC) $(OBJS) $(LDLIBS)

$(TEST): $(OBJS_NO_MAIN) $(TEST_OBJS)
	$(CC) -I $(INCLUDE) -I $(TEST_INCLUDE) $(CFLAGS) -o $(TEST) $(OBJS_NO_MAIN) $(TEST_OBJS)


$(TARGET)$(MAIN_DIR)%.o : $(SRC)$(MAIN_DIR)%.c
	@mkdir -p $(dir $@)
	$(CC) -I $(INCLUDE) -c $(CFLAGS) -o $@ $<

$(TARGET)$(TEST_DIR)%.o : $(SRC)$(TEST_DIR)%.c
	@mkdir -p $(dir $@)
	$(CC) -I $(INCLUDE) -I $(TEST_INCLUDE) -c $(CFLAGS) -o $@ $<

clean:
	@rm -rf $(TARGET) $(EXEC) $(TEST)
