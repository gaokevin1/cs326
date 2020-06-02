bin=scheduler

CFLAGS += -Wall -g
LDFLAGS +=

src=scheduler.c workload.c file_format.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $(bin)

scheduler.o: scheduler.c scheduler.h file_format.h workload.h
workload.o: workload.c workload.h
file_format.o: file_format.c file_format.h

clean:
	rm -f $(bin) $(obj)

