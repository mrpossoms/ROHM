TARG=$(shell ${CXX} -dumpmachine)
OBJS=$(patsubst src/%.cpp,src/%.cpp.o,$(wildcard src/*.cpp))
INC+=
LIB+=
LINK+=-lm
CPP_FLAGS+=

src/%.cpp.o: src/%.cpp
	$(CXX) $(CPP_FLAGS) $(INC) $(LIB) -c $< -o $@

lib/$(TARG):
	mkdir -p $@

lib/$(TARG)/librohm.a: $(OBJS)
	ar -crs $@ $^	

.PHONY: librohm
librohm: lib/$(TARG)/librohm.a
	@echo "Built librohm"
	
