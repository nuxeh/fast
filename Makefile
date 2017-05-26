CC=gcc

qr.pgm:
	qrencode -s 20 -v 2 'this is a test' -o qr.png
	mogrify -format pgm -colorspace gray qr.png
	rm qr.png

fastread: fastread.c
	$(CC) -ggdb -og fastread.c -o $@ -lm 
