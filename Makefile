TARG=$(shell ${CXX} -dumpmachine)
OBJS=$(patsubst src/%.cpp,src/%.cpp.o,$(wildcard src/*.cpp))
INC+=
LIB+=
LINK+=-lm
CPP_FLAGS+=-std=c++11 -g -fPIC

src/%.cpp.o: src/%.cpp lib/$(TARG)
	$(CXX) $(CPP_FLAGS) $(INC) $(LIB) -c $< -o $@

lib/$(TARG):
	mkdir -p $@

lib/$(TARG)/librohm.a: $(OBJS)
	ar -crs $@ $^

.PHONY: librohm clean
librohm: lib/$(TARG)/librohm.a
	@echo "Built librohm"

clean:
	rm src/*.o
	rm -rf lib

