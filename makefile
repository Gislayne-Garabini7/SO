all: myfs
myfs: myfs.c
	gcc -o myfs myfs.c -lfuse
clean:
	rm -f myfs
