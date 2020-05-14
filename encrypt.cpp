#include<stdio.h>
#include<stdlib.h>
#include<string.h>

unsigned int key[] = {0x706D6156, 0x2E657269, 0x204A2E43, 0x53207461};
unsigned int hash_table[256];

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
#define FLAG_SIZE 0x20
#define INDEX_SIZE 0x400

void btea(unsigned int *v, int n) {
	unsigned int y, z, sum;
	unsigned int p, rounds, e;
	rounds = 6 + 52/n;
	sum = 0;
	z = v[n-1];
	do {
	sum += DELTA;
	e = (sum >> 2) & 3;
	for (p=0; p<n-1; p++) {
	  y = v[p+1]; 
	  z = v[p] += MX;
	}
	y = v[0];
	z = v[n-1] += MX;
	} while (--rounds);
}

void read_hash_table()
{
	FILE* fp=fopen("hash_table","rb");
	fread(hash_table, sizeof(int), sizeof(hash_table), fp);
	fclose(fp);
}

unsigned int hash(char *s)	//hash(crc-like) of filename (**.mp3)
{
	unsigned int i, init = 0;
	char v7;
	for(i=0;i<4;i++)
	{
		init <<= 8;
		init |= s[i];
	}
	s+=4;
	v7 = *s;
	for ( i = ~init; v7; ++s )
	{
	  i = hash_table[i >> 24] ^ (v7 | (i << 8));
	  v7 = s[1];
	}
	return ~i;
}

unsigned int hash_filename(char *path)
{
	int n = strlen(path);
	int slash;
	for(slash=n-1;slash>=0;slash--)
	{
		if(path[slash]=='/'||path[slash]=='\\')
			break;
	}
	n = n - slash - 1;
	path += slash + 1;
	char filename[30];
	for(int i=0;i<n;i++)
	{
		if(path[i]>='A'&&path[i]<='Z')
			filename[i]=path[i]-'A'+'a';
		else
			filename[i]=path[i];
	}
	filename[n]=0;
	strcat(filename, ".mp3");
	unsigned int h = hash(filename);
	return h;
}


void encrypt(char *input, char *output)
{
	FILE *fin = fopen(input, "rb");
	fseek(fin, 0, 2);
	int flen = ftell(fin) / sizeof(int) + 1;	//pad to multiple of sizeof int
	fseek(fin, 0, 0);
	unsigned int *music = (unsigned int*)calloc(flen, sizeof(int));
	fread(music, flen, sizeof(int), fin);
	fclose(fin);
	btea(music, flen);
	
	unsigned int flag[FLAG_SIZE] = {0};
	flag[0] = 0x1a545352;
	flag[3] = 0x100080;
	flag[4] = 0x8000;
	flag[1] = flag[5] = flag[8] = 1;
	flag[2] = flag[7] = 0x80;
	
	unsigned int index[INDEX_SIZE] = {0};
	index[0] = hash_filename(output);
	index[1] = 0x10005;
	index[3] = 0x10080;
	index[4] = index[5] = flen * sizeof(int);
	index[7] = 1;	//important
	btea(index, INDEX_SIZE);
	
	unsigned int pad[0x3c00] = {0};

	char filename[30];
	strcpy(filename,output);
	strcat(filename,".smp");
	FILE *fout = fopen(filename, "wb");
	fwrite(flag, sizeof(int), FLAG_SIZE, fout);
	fwrite(index, sizeof(int), INDEX_SIZE, fout);
	fwrite(pad, sizeof(int), 0x3c00, fout);
	fwrite(music, sizeof(int), flen, fout);
	fclose(fout);
	free(music);
}

int main(int argc, char *argv[])
{
	read_hash_table();
	char tmp[30];
	for(int i=1;i<argc;i++)
	{
		printf("%s -> ___.smp ( <20 chars)\n", argv[i]);
		scanf("%s", tmp);
		encrypt(argv[i], tmp);
	}
}
