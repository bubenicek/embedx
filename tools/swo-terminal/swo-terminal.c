
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>


static void usage(void)
{
   printf("Simple SWO terminal\n");
   printf("---------------\n");
   printf("Options:\n");
   printf("   -d <devname> Modem device\n");
   printf("   -b <baudrate> Baudrate (921600, 460800)\n");
   printf("\n");
}


int main(int argc, char *argv[])
{
   int ix, fd, state = 0;
   const char *devname = NULL;
   uint8_t c;
   struct termios options;
   speed_t speed = B921600;


   // Check args
   if (argc < 2)
   {
      fprintf(stderr, "Bad number of params\n");
      usage();
      return 1;
   }

   // Get input params
   for (ix = 1; ix < argc; ix++)
   {
      if (!strcmp(argv[ix], "-d"))
      {
         devname = argv[++ix];
      }
      else if (!strcmp(argv[ix], "-b"))
      {
         int baudrate = atoi(argv[++ix]);
         switch(baudrate)
         {
            case 115200:
                speed = B115200;
                break;

            case 230400:
                speed = B230400;
                break;

            case 460800:
                speed = B460800;
                break;

            case 921600:
                speed = B921600;
                break;

            default:
            {
                fprintf(stderr, "Not supported baudrate %d\n", baudrate);
                return 1;
            }
         }
      }
      else if (!strcmp(argv[ix], "-h"))
      {
         usage();
         return 0;
      }
      else
      {
         fprintf(stderr, "Bad option %s\n", argv[ix]);
         return 1;
      }
   }

   // Check input params
   if (devname == NULL)
   {
      fprintf(stderr, "Modem device not specified\n");
      exit(1);
   }

   // Open modem device
   if ((fd = open(devname, O_RDWR)) < 0)
   {
      fprintf(stderr, "Can't open device %s\n", devname);
      exit(1);
   }

  //
   // Set uart badrate
   //

   // Get the current options for the port...
   tcgetattr(fd, &options);

   // Set the baud rates
   cfsetispeed(&options, speed);
   cfsetospeed(&options, speed);

   // Enable the receiver and set local mode
   options.c_cflag |= (CLOCAL | CREAD);
   options.c_cflag &= ~PARENB;
   options.c_cflag &= ~CSTOPB;
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8;
   options.c_iflag = IGNPAR;     // vypneme terminalove rizeni linky
   options.c_oflag = 0;
   options.c_lflag = 0;          //ICANON;
   options.c_cc[VMIN] = 1;
   options.c_cc[VTIME] = 0;

   // Set the new options for the port...
   tcsetattr(fd, TCSANOW, &options);

   // Read commands from device and execute
   while(1)
   {
      if (read(fd, &c, 1) == 0)
      {
         fprintf(stderr, "Device %s unplugged, shutting down\n", devname);
         close(fd);
         return -1;

      }

      switch(state)
      {
         case 0:
            if (c == 0x1)
               state = 1;
            break;

         case 1:
            putchar(c);
            fflush(stdout);
            state = 0;
            break;
      }
   }

   close(fd);

   return 0;
}

