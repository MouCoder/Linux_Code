
	cc=gcc
	
proc:main.o proc.o
	cc -o $@ $^ 
*.o:*.c
	cc -o $@ -c $^
.PHONY:clean
clean:
	@rm -rf *.o proc
