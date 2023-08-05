bm: bm.c
	$(CC) -obm bm.c -lX11

clean: bm
	rm bm
