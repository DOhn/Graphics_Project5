all:
	gcc -o main texdemo.c -L/usr/local/lib -lglfw3 -lrt -lXrandr -lXi -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp

p3:
	./main pic_p3.ppm

p6:
	./main pic_p6.ppm
