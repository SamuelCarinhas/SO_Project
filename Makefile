CC				:= gcc
CFLAGS			:= -Wall -Wextra -g -pthread
SRC				:= src
OBJ				:= extra
BINDIR			:= .
SRCS			:= $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/**/*.c)
OBJS			:= $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BIN				:= $(BINDIR)/project
SUBMITFILE		:= teste.zip
BUILD_DIR		:= build

all:			$(BIN)

DEPS			:= $(patsubst $(OBJ)/%.o, $(OBJ)/%.d, $(OBJS))
-include $(DEPS)
DEPFLAGS 		= -MMD -MF $(@:.o=.d)

$(BIN):			$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o:		$(SRC)/%.c
	rsync -a --include='*/' --exclude='*' $(SRC)/ $(OBJ)/
	$(CC) $(CFLAGS) -c $< -o $@ $(DEPFLAGS)

clean:
	$(RM) -r $(BIN) $(OBJS) $(DEPS)
	$(RM) -r $(OBJ)

deploy:
	mkdir -p $(BUILD_DIR)
	cp	$(BIN)	$(BUILD_DIR)/$(BIN)

submit:
	$(RM) $(SUBMITFILE)
	zip -r $(SUBMITFILE) .