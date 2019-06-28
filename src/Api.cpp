#include "parse_deep_search_decode.h"
#include "Api.h"


void *mkgraph_source(char *path ,char *gram, char * frames)
{
	MkGraph *mkgraph = new MkGraph();
	mkgraph->ConstructGraph(path, gram);
	mkgraph->ConstructSearchNet(path,frames);
	mkgraph->SortSearchNet();
#ifdef DEBUG
	mkgraph->WriteGraph("graphdir/gram.txt","graphdir/dict.txt","graphdir/netlist.txt","graphdir/outframes.txt");
#endif
	return static_cast<void*> (mkgraph);
}

void *read_source(char *gram,char *dict,char *netlist,char *outframes)
{
	MkGraph *mkgraph = new MkGraph();
	mkgraph->ReadGraph(gram,dict,netlist,outframes);
	return static_cast<void*> (mkgraph);
}

int write_source(void *source, char *gram,char *dict,char *netlist,char *outframes)
{
	MkGraph *mkgraph = static_cast<MkGraph *>(source);
	mkgraph->SortSearchNet();
	return mkgraph->WriteGraph(gram,dict,netlist,outframes);
}

void *init_hubo_parse(void *source)
{
	MkGraph *mkgraph = static_cast<MkGraph *>(source);
	ParseDeepDecode  *decode = new ParseDeepDecode(static_cast<BaseGraph *> (mkgraph));
	return static_cast<void*>(decode);
}

int hubo_parse(void *parse, char *sent, char *result, float *score)
{
	ParseDeepDecode  *decode = static_cast<ParseDeepDecode *>(parse);
//	decode->Decoder(sent);
	*score = decode->Decoder(sent,result);
	return 0;
}

void destory_hubo_parse(void *parse)
{
	ParseDeepDecode  *decode = static_cast<ParseDeepDecode *>(parse);
	delete decode;
}


void destory_source(void *source)
{
	MkGraph *mkgraph = static_cast<MkGraph *>(source);
	delete mkgraph;
}

char *get_history()
{
	return NULL;
}


