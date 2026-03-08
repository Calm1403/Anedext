
OBJDR = Build
FILES = main.o file.o input.o process.o mappings.o
OBJS = $(addprefix $(OBJDR)/, $(FILES))

./Build/anedext: $(OBJS)
	cc -Wall -Werror $^ -o $(OBJDR)/anedext

./Build/%.o: ./Src/%.c
	cc -Wall -Werror -c $< -o $@

clean:
	rm Build/*.o Build/anedext

clear:
	rm Build/*.o

$(OBJS) : | $(OBJDR)

$(OBJDR):
	mkdir Build
