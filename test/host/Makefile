CXX ?= g++
CORE = ../../lib/PhotoCore
test: test_core
	./test_core
test_core: test_core.cpp $(CORE)/films.cpp $(CORE)/exposure.cpp $(CORE)/exposure_state.cpp $(CORE)/countdown.cpp $(CORE)/pinhole_state.cpp $(CORE)/solar.cpp
	$(CXX) -std=c++14 -Wall -Wextra -I$(CORE) -o $@ $^ -lm
clean:
	rm -f test_core
