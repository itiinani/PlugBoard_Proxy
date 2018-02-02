


C_FILES := $(wildcard source/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
CC_FLAGS := -I include/

pbproxy: $(OBJ_FILES)
	gcc -o $@ $^  -lcrypto -lpthread

obj/%.o: source/%.c
	gcc $(CC_FLAGS) -c -o $@ $<

clean :
	\rm -fr obj/*
	\rm -fr mydump
	\rm -fr *~
	\rm -f pbproxy
