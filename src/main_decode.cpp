#include <string.h>
#include "parsedecode.h"


int main(int argc,char *argv[])
{
	if(argc < 2 || argc > 6)
	{
		LOGERR("%s testlist (gram) (dict) (netlist) (inframes)\n",argv[0]);
		return -1;
	}
	char gram[128] = "gram.net";
	char dict[128] = "dict.txt";
	char netlist[128] = "net.list";
	char outframes[128] = "frames.txt";

	if(argc > 2)
	{
		memset(gram,0x00,sizeof(gram));
		strcpy(gram,argv[2]);
	}
	if(argc > 3)
	{
		memset(dict,0x00,sizeof(dict));
		strcpy(dict,argv[3]);
	}
	if(argc > 4)
	{
		memset(netlist,0x00,sizeof(netlist));
		strcpy(netlist,argv[4]);
	}
	if(argc > 5)
	{
		memset(outframes,0x00,sizeof(outframes));
		strcpy(outframes,argv[5]);
	}
	ParseDecode decode;
	int ret = decode.ReadGraph(gram,dict,netlist,outframes);
	if(0 == ret )
	{
		char sent[1024];
		FILE *fp = fopen(argv[1],"r");
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
			//printf("for example \"the next morning\"\n");
			//printf("Please input sentence:");
			//fgets(sent,sizeof(sent),stdin);
			//printf("sentence: %s\n",sent);
			decode.Decoder(sent);
			memset(sent,0x00,sizeof(sent));
		}
	}
	else
	{
		LOGERR("read source failed.\n");
		return -1;
	}
	return 0;
}


