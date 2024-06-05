all: audiospectrum

audiospectrum: audiospectrum.c
	$(CC) -o $@ $<	

.PHONY: all clean

clean:
	@echo 'CLEANING'
