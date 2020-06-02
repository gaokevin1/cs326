
all: open-log.so rick-ropen.so hidden.so ghost.so

open-log.so: open-log.c
	gcc -shared -fPIC -ldl $< -o $@

rick-ropen.so: rick-ropen.c
	gcc -shared -fPIC -ldl $< -o $@

hidden.so: hidden.c
	gcc -shared -fPIC -ldl $< -o $@

ghost.so: ghost.c
	gcc -shared -fPIC -ldl $< -o $@

clean:
	rm -f *.so
