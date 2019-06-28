#include <stdio.h>
#include <string.h>
#include "Api.h"

int main(int argc,char *argv[])
{
	if(argc < 4 || argc > 9)
	{
		printf("%s path file inframes test.file (gram) (dict) (netlist) (outframes)\n",argv[0]);
		return -1;
	}
	char *frames = NULL;
	char gram[128] = "gram.net";
	char dict[128] = "dict.txt";
	char netlist[128] = "net.list";
//	char outframes[128] = "frames.txt";
	char *testfile = NULL;
	if(argc > 3)
	{
		if(0 != strncmp(argv[3],"null",4))
			frames = argv[3];
	}
	if(argc > 4)
		testfile = argv[4];
	if(argc > 5)
	{
		memset(gram,0x00,sizeof(gram));
		strcpy(gram,argv[5]);
	}
	if(argc > 6)
	{
		memset(dict,0x00,sizeof(dict));
		strcpy(dict,argv[6]);
	}
	if(argc > 7)
	{
		memset(netlist,0x00,sizeof(netlist));
		strcpy(netlist,argv[7]);
	}
	void *source = mkgraph_source(argv[1],argv[2],frames);
	if(source == NULL)
	{
		fprintf(stderr,"init source failed.\n");
		return -1;
	}

	void *parse = init_hubo_parse(source);
	if(NULL == parse)
	{
		fprintf(stderr,"init parse failed.\n");
		return -1;
	}

	char sent[1024];
	char result[4000];
	float score = 0.0;
	FILE *fp = fopen(testfile,"r");
	if(NULL == fp)
	{
		printf("fopne %s error!\n",argv[1]);
		return -1;
	}
	while(NULL != fgets(sent,sizeof(sent),fp))
	{
		sent[strlen(sent)-1] = '\0';
		if(sent[0] == '#')
			continue;
		int ret = hubo_parse(parse,sent , result, &score);
		if(ret < 0)
		{
			printf("no match.\n");
		}
		printf("score %f@%s\n",score,result);
		memset(sent,0x00,sizeof(sent));
		memset(result,0x00,sizeof(result));
		score = 0;
	}

	destory_hubo_parse(parse);
	destory_source(source);
	fclose(fp);
	return 0;
}
