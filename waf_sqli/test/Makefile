CXX = g++
BINS = config_parser http_parser sqli

all: config_parser_test http_parser_test sqli_test

config_parser_test:
	g++ config_parser_test.cc ../utility/config_parser.cc -o config_parser_test
	./config_parser_test
http_parser_test:
	g++ http_parser_test.cc ../utility/http_parser.cc -o http_parser_test
	./http_parser_test
sqli_test:
	g++ sqli_test.cc ../utility/sqli.cc ../utility/libinjection/*.c -o sqli_test
	./sqli_test

clean:
	rm -f *_test
