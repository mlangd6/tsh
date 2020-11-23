CC=gcc
CFLAGS=-g -Wall
LDLIBS = -lreadline
EXEC=tsh
TEST=tsh_test

BIN=bin/
SRC=src/
TARGET=target/
MAIN_DIR=main/
TEST_DIR=test/
CMD_DIR=cmd/
TYPES_DIR=types/
INCLUDE=$(SRC)include/
TEST_INCLUDE=$(SRC)test_include/
TSH_DIR=${HOME}/.tsh

# TSH
MAIN_FILES=$(wildcard $(SRC)$(MAIN_DIR)*.c)
OBJS=$(notdir $(MAIN_FILES:.c=.o))
OBJS:=$(addprefix $(TARGET)$(MAIN_DIR), $(OBJS))

OBJS_NO_MAIN=$(filter-out $(TARGET)$(MAIN_DIR)main.o, $(OBJS))

# TEST
TEST_FILES=$(wildcard $(SRC)$(TEST_DIR)*.c)
TEST_OBJS=$(notdir $(TEST_FILES:.c=.o))
TEST_OBJS:=$(addprefix $(TARGET)$(TEST_DIR), $(TEST_OBJS))

# BIN
CMD_FILES=$(wildcard $(SRC)$(CMD_DIR)*.c)
BIN_FILES=$(notdir $(basename $(CMD_FILES)))
BIN_FILES:=$(addprefix $(BIN), $(BIN_FILES))

# TYPES
TYPES_FILES=$(wildcard $(SRC)$(TYPES_DIR)*.c)
TYPES_OBJS=$(notdir $(TYPES_FILES:.c=.o))
TYPES_OBJS:=$(addprefix $(TARGET)$(TYPES_DIR), $(TYPES_OBJS))

# Prevent deletion of file
.PRECIOUS: 	$(TARGET)$(CMD_DIR)%.o

all: types $(EXEC) cmd $(TEST)

cmd: $(BIN_FILES)
	@mkdir -p $(TSH_DIR)
	@cp -r $(BIN) $(TSH_DIR)

types: $(TYPES_OBJS)


$(EXEC): $(OBJS) $(TYPES_OBJS)
	@$(CC) -I $(INCLUDE) $(CFLAGS) -o $(EXEC) $^ $(LDLIBS)

$(TEST): $(OBJS_NO_MAIN) $(TEST_OBJS) $(TYPES_OBJS)
	@$(CC) -I $(INCLUDE) -I $(TEST_INCLUDE) $(CFLAGS) -o $(TEST) $^


$(TARGET)$(MAIN_DIR)%.o : $(SRC)$(MAIN_DIR)%.c
	@mkdir -p $(dir $@)
	@$(CC) -I $(INCLUDE) -c $(CFLAGS) -o $@ $<

$(TARGET)$(TEST_DIR)%.o : $(SRC)$(TEST_DIR)%.c
	@mkdir -p $(dir $@)
	@$(CC) -I $(INCLUDE) -I $(TEST_INCLUDE) -c $(CFLAGS) -o $@ $<

$(TARGET)$(CMD_DIR)%.o : $(SRC)$(CMD_DIR)%.c
	@mkdir -p $(dir $@)
	@$(CC) -I $(INCLUDE) -c $(CFLAGS) -o $@ $<

$(BIN)% : $(OBJS_NO_MAIN) $(TARGET)$(CMD_DIR)%.o
	@mkdir -p $(BIN)
	@$(CC) -I $(INCLUDE) $(CFLAGS) -o $@ $^

$(TARGET)$(TYPES_DIR)%.o : $(SRC)$(TYPES_DIR)%.c
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -rf $(TARGET) $(EXEC) $(TEST) $(BIN)
