
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
	int res, i;
	FILE *fr;
	const char *filename;
	const char *blobname;
	uint8_t buf[1024];
	int col = 0;

	if (argc < 3)
	{
		fprintf(stderr, "Bad number of params\n");
		return 1;
	}

	filename = argv[1];
	blobname = argv[2];

	if ((fr = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Can't open file %s\n", filename);
		return 1;
	}

	printf("\n");
	printf("static const char %s[] = \n", blobname);
	printf("{\n");

	while((res = fread(buf, sizeof(char), sizeof(buf), fr)) > 0)
	{
		for (i = 0; i < res; i++)
		{
			printf("0x%2.2X, ", buf[i]);

			if (++col == 16)
			{
				col = 0;
				printf("\n");
			}
		}
  }

	printf("0x00\n};\n");
	fclose(fr);

	return 0;
}
