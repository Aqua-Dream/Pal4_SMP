#include<stdio.h>
#include<stdlib.h>
#include<string.h>

unsigned int key[] = {0x706D6156, 0x2E657269, 0x204A2E43, 0x53207461};

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void btea(unsigned int *v, int n) {	//decrypt code of xxtea
	unsigned int y, z, sum;
	unsigned int p, rounds, e;
	rounds = 6 + 52/n;
	sum = rounds*DELTA;
	y = v[0];
	do {
		e = (sum >> 2) & 3;
		for (p=n-1; p>0; p--) {
		  z = v[p-1];
		  y = v[p] -= MX;
		}
		z = v[n-1];
		y = v[0] -= MX;
	} while ((sum -= DELTA) != 0);
}

void decrypt(char *input, char *output)
{
	FILE *fin = fopen(input, "rb");
	fseek(fin, 0, 2);
	int flen = ftell(fin);
	fseek(fin, 0, 0);	//get file length
	char *buf = (char*)malloc(flen);
	fread(buf, flen, 1, fin);
	fclose(fin);
	unsigned int *cipher = (unsigned int*)(buf + 0x80);
	btea(cipher, 0x400);
	int pos = cipher[3];
	int len = cipher[4] / 4;
	unsigned int *music = (unsigned int*)(buf + pos);
	btea(music, len);
	FILE *fout = fopen(output, "wb");
	fwrite(music, sizeof(int) * len, 1, fout);
	fclose(fout);
	free(buf);
}

int main(int argc, char *argv[])
{
	for(int i=1;i<argc;i++)
	{
		char *s = argv[i];
		int n = strlen(s);
		char *t = (char*)malloc(n + 1);
		strcpy(t, s);
		strcpy(t + n - 3, "mp3");	//change smp to mp3
		decrypt(s, t);
		free(t);
	}
}
