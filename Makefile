bm: bm.c
	$(CC) -o$@ $(CFLAGS) bm.c -lX11

clean: bm
	rm bm
