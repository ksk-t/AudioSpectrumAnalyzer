all: audiospectrum

audiospectrum: audiospectrum.c
	$(CC) -o $@ $<	-liio -lfftw3 -lm

.PHONY: all clean

clean:
	@echo 'CLEANING'
