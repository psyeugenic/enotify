all:
	gcc -o ebin/notify_drv.so -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/lib64/gtk-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/opt/lib/erlang/erts-5.7.1/include -fpic -shared  c_src/notify_drv.c -lnotify
	erlc -oebin src/enotify.erl
