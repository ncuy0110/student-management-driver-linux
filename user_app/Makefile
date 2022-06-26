CC = gcc

CFLAGS = -Wall -Wextra -O3

md5: md5.c user_test.c
	@$(CC) $(CFLAGS) -o user_app md5.c user_test.c

clean:
	@$(RM) user_app
