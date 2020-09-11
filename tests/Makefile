TARGET=$(shell ${CC} -dumpmachine)
INC+=-I../src -I../inc -I../deps/xbc/src
LIBS+=-L../lib/$(TARGET) -L/usr/lib
LINK+= ../lib/$(TARGET)/librohm.a -lm -ltiff
CXX_FLAGS+= -g -std=c++11 -O0

TESTS=$(patsubst %.cpp,bin/%,$(wildcard *.cpp))

bin:
	mkdir bin

bin/%: %.cpp bin
	$(CXX) $(CXX_FLAGS) $(INC) $(LIBS) $< $(SRC_OBJS) -o $@ $(LINK)

src/%.o: src/% lib/$(TARGET)
	$(CXX) $(CXX_FLAGS) $(INC) $(LIB) -c $< -o $@ $(LINK)

.PHONY: test clean
test: $(TESTS)
	@./.test-runner.py
clean:
	rm -rf bin/
