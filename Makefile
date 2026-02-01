
FILES = main.o file.o input.o process.o mappings.o

anedext: $(FILES)
	cc $^ -o anedext

$(FILES): %.o: %.c
	cc -c $< -o $@

clean:
	rm *.o anedext

clear:
	rm *.o
