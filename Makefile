# makefile

CC=gcc
CFLAGS=-Wall -Werror -g

systemMonitoringTool:	main.o	stats_functions.o
	$(CC) $(CFLAGS) $^ -o $@

main.o:	main.c	stats_functions.h
	$(CC) $(CFLAGS) -c $< -o $@

stats_functions.o:	stats_functions.c	stats_functions.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:	clean

clean:
	rm -f *.o systemMonitoringTool
