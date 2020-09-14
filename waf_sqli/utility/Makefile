all: config_parser.o http_parser.o sqli.o

config_parser: config_parser.h nlohmann/json.hpp
	cc -c config_parser.cc -o config_parser.o
