#include <stdio.h>
#include <string.h>
#include "mkgraph.h"


int main(int argc,char *argv[])
{
	if(argc < 3 || argc > 8)
	{
		LOGERR("%s path file (inframes(null)) (gram) (dict) (netlist) (outframes)\n",argv[0]);
		return -1;
	}
	char *frames = NULL;
	char gram[128] = "gram.net";
	char dict[128] = "dict.txt";
	char netlist[128] = "net.list";
	char outframes[128] = "frames.txt";

	if(argc > 3)
	{
		if(0 != strncmp(argv[3],"null",4))
			frames = argv[3];
	}
	if(argc > 4)
	{
		memset(gram,0x00,sizeof(gram));
		strcpy(gram,argv[4]);
	}
	if(argc > 5)
	{
		memset(dict,0x00,sizeof(dict));
		strcpy(dict,argv[5]);
	}
	if(argc > 6)
	{
		memset(netlist,0x00,sizeof(netlist));
		strcpy(netlist,argv[6]);
	}
	MkGraph mkgraph;
	mkgraph.ConstructGraph(argv[1],argv[2]);
	mkgraph.ConstructSearchNet(argv[1],frames);
	mkgraph.SortSearchNet();
	mkgraph.WriteGraph(gram,dict,netlist,outframes);
	return 0;
}

