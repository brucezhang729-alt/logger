CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
example: example.cpp personal_logger.hpp
	$(CXX) $(CXXFLAGS) -pthread example.cpp -o $@
.PHONY: clean
clean:
	rm -f example
	rm -rf log
