hw3: testcase.c thread.o scheduler.o semaphore.o
	gcc -g testcase.c thread.o scheduler.o semaphore.o -o hw3

thread.o: thread.h init.h thread.c
	gcc -c -g thread.c

scheduler.o: scheduler.c scheduler.h
	gcc -c -g scheduler.c

semaphore.o: semaphore.c semaphore.h
	gcc -c -g semaphore.c

clean:
	rm -rf *.o
