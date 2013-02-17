.PHONY:
	all

SOURCES = src/UCI.cc

all: $(SOURCES)
	c++ -o bin/DeepBlur_debug -Wall $(SOURCES)

o: $(SOURCES)
	c++ -o bin/DeepBlur -O3 $(SOURCES)
