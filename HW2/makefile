hw2: testcase.c thread.o scheduler.o TestCase1.o TestCase2.o TestCase3.o
	gcc -g testcase.c thread.o scheduler.o TestCase1.o TestCase2.o TestCase3.o -o hw2

thread.o: thread.h init.h thread.c
	gcc -c -g thread.c

scheduler.o: scheduler.h scheduler.c
	gcc -c -g scheduler.c

TestCase1.o: TestCase1.h TestCase1.c
	gcc -c -g TestCase1.c

TestCase2.o: TestCase2.h TestCase2.c
	gcc -c -g TestCase2.c

TestCase3.o: TestCase3.h TestCase3.c
	gcc -c -g TestCase3.c

clean:
	rm -f *.o
