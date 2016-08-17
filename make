#!/bin/sh
#-g -D_DEBUG
gcc -c asttodb.c tcp_client.c db_manager_pgsql.c -I$(pg_config --includedir)
gcc asttodb.o tcp_client.o db_manager_pgsql.o -L$(pg_config --libdir)  -lpq -o asttodb
