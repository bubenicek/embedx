#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>


int main(int argc, char **argv)
{
   int res;
   FILE *fr;
   uint8_t c;
   char buf[3] = {0, 0, 0};
   
   if (argc < 2)
   {
      printf("Enter hex filename\n");
      return 1;
   }
   
   if ((fr = fopen(argv[1], "r")) == NULL)
   {
      printf("Can't open file %s\n", argv[1]);
      return 1;
   }
   
   while((res = fread(buf, sizeof(char), 2, fr)) == 2)
   {
      c = strtol(buf, NULL, 16);
      fwrite(&c, sizeof(char), 1, stdout);

      // Skip ' '
      if ((res = fread(buf, sizeof(char), 1, fr)) != 1)
         break;
   }
   
   if (res < 0)
   {
      printf("Read failed\n");
      return 1;
   }
     
   fclose(fr);
   
   return 0;
}
