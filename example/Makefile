IVT=../ivt_change
GEN=../ivt_gen

CONFIG=ivt.ivt
CONBAK=ivt.bak
CONGEN=ivt.cpp

# original makefile
default: snd_orig rec_orig

snd_orig: snd.cpp cls.cpp cls.hpp
	clang++ -lrt -lpthread snd.cpp cls.cpp -o snd

rec_orig: rec.cpp cls.cpp cls.hpp
	clang++ -lrt -lpthread rec.cpp cls.cpp -o rec

# ivt makefile
ivt: snd_bld rec_bld $(CONGEN) snd_ivt rec_ivt
	@echo "finished"

$(CONGEN): $(CONFIG)
	$(GEN) $^ > $@

snd_bld: snd.s cls.s

rec_bld: rec.s cls.s

snd_ivt: snd.s cls.s $(CONGEN)
	clang++ -lrt -lpthread $^ -o $@

rec_ivt: rec.s cls.s $(CONGEN)
	clang++ -lrt -lpthread $^ -o $@

%.s: %.bcc
	llc $^ -o $@
#	llvm-dis $^ -o $^.ll

%.bcc: %.bc $(CONFIG)
	$(IVT) $^ > $@ 2>/dev/null

%.bc: %.cpp
	clang++ -emit-llvm -c $^ -o $@
#	clang++ -emit-llvm -S $^ -o $@.ll

# cleaning
.PHONY: clean
clean:
	rm -f *.o *.ll *.bc *.bcc *.s $(CONGEN) snd rec snd_ivt rec_ivt $(CONFIG)
	cp $(CONBAK) $(CONFIG)

