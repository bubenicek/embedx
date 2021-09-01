/**
 * \file main.c         \brief Read information from APP header
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../core/lib/crc32.h"
#include "../../core/lib/bootloader/app_header.h"


int main(int argc, char *argv[])
{
   FILE *fw;
   char *filename;
   uint32_t hdr_offset;
   app_header_t hdr;

   if (argc < 3)
   {
      printf("Bad number of params, usage: ./apphdr {filename} {app_header_offset}\n");
      return 1;
   }
   filename = argv[1];
   sscanf(argv[2], "%x", &hdr_offset);   // must be given as HEX number!

   // Open firmware file
   if ((fw = fopen(filename, "r+")) == NULL)
   {
      printf("Can't open firmware file %s\n", filename);
      return 1;
   }

   // Read fw header
   if (fseek(fw, hdr_offset, SEEK_SET) < 0)
   {
      printf("Seek to header failed\n");
      return 1;
   }
   if (fread(&hdr, sizeof(char), sizeof(hdr), fw) != sizeof(hdr))
   {
      printf("Read fw header failed\n");
      return 1;
   }
   
   if (hdr.magic != APP_HEADER_MAGIC)
   {
      printf("Bad app header magic\n");
      return 1;
   }

   fclose(fw);

   printf("%d.%d\n", APP_VERSION_MAJOR(hdr.fw_version), APP_VERSION_MINOR(hdr.fw_version));

   return 0;
}
