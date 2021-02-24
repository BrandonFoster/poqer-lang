ifeq ($(OS), Windows_NT)
WIN_FLAGS := -municode -posix
endif

all: devel

debug:
	gcc -std=c99 -g -Wall -Wpedantic -Werror -o program src/pq_string.c src/pq_scanner.c src/pq_parser.c src/pq_syntax_tree.c src/pq_unicode.c src/pq_utils.c src/pq_main.c $(WIN_FLAGS)

devel:
	gcc -std=c99 -g -Wall -Wpedantic -o program src/pq_string.c src/pq_scanner.c src/pq_parser.c src/pq_syntax_tree.c src/pq_unicode.c src/pq_utils.c src/pq_main.c $(WIN_FLAGS)