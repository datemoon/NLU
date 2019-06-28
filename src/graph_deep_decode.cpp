#include <stdio.h>
#include <string.h>
#include "mkgraph.h"
#include "parse_deep_search_decode.h"

int main(int argc,char *argv[])
{
	if(argc < 4 || argc > 9)
	{
		LOGERR("%s path file inframes test.file (gram) (dict) (netlist) (outframes)\n",argv[0]);
		return -1;
	}
	char *frames = NULL;
	char gram[128] = "gram.net";
	char dict[128] = "dict.txt";
	char netlist[128] = "net.list";
	char outframes[128] = "frames.txt";
	char *testfile = NULL;
	if(argc > 3)
	{
		if(0 != strncmp(argv[3],"null",4))
			frames = argv[3];
	}
	if(argc > 4)
	{
		testfile = argv[4];
	}
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
	MkGraph *mkgraph = new MkGraph();
	mkgraph->ConstructGraph(argv[1],argv[2]);
	mkgraph->ConstructSearchNet(argv[1],frames);
	mkgraph->SortSearchNet();
	mkgraph->WriteGraph(gram,dict,netlist,outframes);
	
	ParseDeepDecode  *decode = new ParseDeepDecode(static_cast<BaseGraph *> (mkgraph));


	if(decode != NULL)
	{
		char sent[1024];
		FILE *fp = fopen(testfile,"r");
		if(NULL == fp)
		{
			LOGERR("fopne %s error!\n",argv[1]);
			return -1;
		}
		while(NULL != fgets(sent,sizeof(sent),fp))
		{
			sent[strlen(sent)-1] = '\0';
			if(sent[0] == '#')
				continue;
			decode->Decoder(sent);
			memset(sent,0x00,sizeof(sent));
		}
		fclose(fp);
	}
	delete decode;
	delete mkgraph;
	return 0;
}

