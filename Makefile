CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -pedantic-errors \
         -std=c99 -D_POSIX_C_SOURCE=200809L \
         -g0 -Os
LDFLAGS = -lldap

ldap-auth-external: ldap-auth-external.o
	$(CC) -o $@ $< $(LDFLAGS)

ldap-auth-external.o: ldap-auth-external.c
	$(CC) $(CFLAGS) -c -o $@ $<
