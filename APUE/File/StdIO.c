#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: %s <filename>\n", argv[0]);
		exit(-1);
	}
	FILE *fp = fopen(argv[1], "w+");
	if (!fp) {
		printf("fopen file:%s error\n", argv[1]);
		exit(-1);
	}

	printf("sizeof(int) = %d\n", sizeof(int));
	int arr[3] = {1,2,3};
	size_t wr_len = fwrite(arr, sizeof(int), 3, fp);
	printf("fwrite wr_len[%d]\n", wr_len);

	rewind(fp);
	int rd_arr[3];
	fread(rd_arr, sizeof(int), 3, fp);
	for (int i = 0; i < 3; ++i) {
		printf("%d ", rd_arr[i]);
	}
	printf("\n");
	// int c;
	// while ((c = fgetc(fp)) != EOF) {
	// 	fputc(c, stdout);
	// }

	fclose(fp);
	return 0;
}
