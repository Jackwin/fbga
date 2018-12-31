CC = arm-xilinx-linux-gnueabi-gcc
CFLAGS = -static
SRCS := gpio.c app.c cdma.c network.c 
OBJS =  gpio.o app.o cdma.o network.o
TARGET = app
$(TARGET):$(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS)
#$(OBJS):%.o:%.c
#	$(CC) -c $(CFLAGS) $< -o $@
clean:
	-rm -f $(TARGET) $(OBJS)

