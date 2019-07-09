LLVM_CONFIG=llvm-config
CXX=clang++

ifndef VERBOSE
QUIET:=@
endif

LDFLAGS+=$(shell $(LLVM_CONFIG) --ldflags) -lpthread
COMMON_FLAGS=-Wall -Wextra
CXXFLAGS+=$(COMMON_FLAGS) $(shell $(LLVM_CONFIG) --cxxflags)
CPPFLAGS+=$(shell $(LLVM_CONFIG) --cppflags) -I$(SRC_DIR)

TARGETS=ivt_change ivt_gen
DEPS=common.h

default: $(TARGETS)

%.o: %.cpp $(DEPS)
	@echo Compiling $*.cpp
	$(QUIET)$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

ivt_change: ivt_change.o
	@echo Linking $@
	$(QUIET)$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^ `$(LLVM_CONFIG) --libs bitreader core support nativecodegen`

ivt_gen: ivt_gen.o
	@echo Linking $@
	$(QUIET)$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^ `$(LLVM_CONFIG) --libs bitreader core support nativecodegen`

.PHONY:clean
clean:
	$(QUIET)rm -f *.o $(TARGETS)

