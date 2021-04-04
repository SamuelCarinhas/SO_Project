CC				:= gcc
CFLAGS			:= -Wall -Wextra -g -pthread
OBJS			:= race_simulator.o read_config.o functions.o malfunction_manager.o race_manager.o team_manager.o
PROG			:= project

all:			$(PROG)

deps 			:= $(patsubst %.o,%.d,$(OBJS))
-include $(deps)
DEPFLAGS 		= -MMD -MF $(@:.o=.d)

project: 		$(OBJS)
			$(CC) $(CFLAGS) $^ -o $@

%.o:			%.c
			$(CC) $(CFLAGS) -c $< $(DEPFLAGS)

clean:
			rm -f $(OBJS) $(deps) $(PROG)
