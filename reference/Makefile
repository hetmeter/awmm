#YICES = /home/mvechev/yices-1.0.34
#Z3 = /home/mvechev/z3
YICES = /home/dani/workspace/yices-1.0.34
Z3 = /home/dani/workspace/z3

# all: c2bpy c2bpz3
all: c2bpy

c2bpy: c2bp1.5.c yices.c
	gcc -o c2bpy -DSMT_YICES -I $(YICES)/include `pkg-config --cflags --libs glib-2.0` -L $(YICES)/lib c2bp1.6.c yices.c  -lyices -lgmp -lglib-2.0

#c2bpz3: c2bp1.5.c z3.c
#	g++ -o c2bpz3 -DSMT_Z3 -I $(Z3)/lib `pkg-config --cflags --libs glib-2.0` -L $(Z3)/bin/external c2bp1.5.c z3.c -lz3 -lgmp -lglib-2.0

clean:
#	rm -f *.o c2bpz3 c2bpy c2bp
	rm -f *.o c2bpy c2bp
