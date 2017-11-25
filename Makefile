.PHONY: arm

default:
	gcc -o viewcam -lm `pkg-config --libs opencv` viewcam.c
arm:
	arm-linux-gnueabi-gcc `pkg-config --cflags opencv` -o armviewcam `pkg-config --libs opencv` viewcam.c
