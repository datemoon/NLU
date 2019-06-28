#include "mkgraph.h"
#include <stdio.h>
#include <string.h>

MkGraph::MkGraph()
{
//	_net_startid = 0;

	_net_inter = 0;
	_num_macro = 1;
	_cur_line_nodeid = 0;
	_cur_nodeid = 0;
	_cur_net_endnodeid = 0;
}
int MkGraph::GetFileNetNet(char *grammarfile)
{
	_net_index.push_back("NULL");
	char line[1024];
	memset(line,0x00,sizeof(line));
	FILE * fp = fopen(grammarfile,"r");
	if( NULL == fp)
	{
		LOGERR("fopen %s error!\n",grammarfile);
		return ERROR;
	}
	int net_num=1;
	int flag = 0;
	while(NULL != fgets(line, sizeof(line), fp))
	{
		if(line[0] == '[')
		{
			if(flag == 1)
			{
				LOGERR("find %s net up is error,loss ';'.\n",line);
				return ERROR;
			}
			char net_name[256];
			memset(net_name,0x00,sizeof(net_name));
			char *s = strchr(line, ']');
			memcpy(net_name,line,s-line+1);
			if(_net.find(net_name) == _net.end())
			{
				_net[net_name] = net_num++;
				_net_index.push_back(net_name);
				flag = 1;
			}
			else
			{
				LOGERR("find %s net is retetition define.\n",net_name);
				return ERROR;
			}
		}
		else if(line[0] == ';')
		{
			flag = 0;
		}
	}
	fclose(fp);
	return net_num-1;
}

int MkGraph::ConstructGraph(char *grammarpath,char *grammarfile)
{

	char fname[128];
	memset(fname,0x00,sizeof(fname));
	sprintf(fname, "%s/%s", grammarpath, grammarfile);
	if(ERROR == GetFileNetNet(fname))
	{
		LOGERR("get net list error!\n");
		return ERROR;
	}
	FILE *fp = fopen(fname,"r");
	if(NULL == fp)
	{
		LOGERR("fopen %s failed!\n",fname);
		return ERROR;
	}
	char line[1024];
	memset(line,0x00,sizeof(line));

	int n_line = 0;
	while(memset(line,0x00,sizeof(line)) &&  NULL != fgets(line, sizeof(line), fp) ) 
	{
		n_line++;
		LOGDEBUG("%s",line);
		if(line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		if(BlankLine(line) == 1)
			continue;
		/* include file */
		if( 0 == strncmp(line, "#incl", 5) ) 
		{
			if(_net_inter == 0)
			{
				LOGERR("%s in net inter,it's error!\n",line);
				return ERROR;
			}
			char name[128];
			memset(name,0x00,sizeof(name));
			sscanf(line, "#include %s", name);

			if(SUCC != ConstructGraph(grammarpath ,name))
			{
				LOGERR("construct %s failed\n",line);
				return ERROR;
			}
			continue;
		}
		else if( line[0] == '#' ) continue;
		/* net definition end */
		else if( line[0] == ';' )
		{
			ProcessCurNet();
			ResetCurInfo();
			continue;
		}
		else  if(line[0] == '[')
		{
			char net_name[256];
			memset(net_name,0x00,sizeof(net_name));
			char *s = strchr(line, ']');
			memcpy(net_name,line,s-line+1);
			int netid = GetNetId(net_name);
			if(SUCC != AddNetToCur(netid))//add net to _cur_net
			{
				LOGERR("file(%s)@:AddNetToCur (%d %s) error.\n",
						grammarfile,n_line,line);
			}
			_net_inter = 1;//now is net inter.
		}
		else if( isspace( (int) line[0]) )
		{
			if( !index(line, (int) '(' ) ||
					!index(line, (int) ')' ) )
			{
				LOGERR("file(%s)@ERROR: Bad format for rule: %d %s\n",
						grammarfile,n_line,line);
				continue;
			}
			if(SUCC != AddLineToNet(line))
			{
				LOGERR("%s AddLineToNet error!\n",line);
			}
		}
		else if( isupper( (int) line[0]) )
		{
			char sym[128];
			memset(sym,0x00,sizeof(sym));
			int nullarc =0,looparc =0;
			char *line_ptr = line;
			line_ptr = GetSymbol(sym,line_ptr,&nullarc,&looparc);
			if(line_ptr != NULL)
			{
				LOGERR("%s format error!\n",line);
				return ERROR;
			}
			if(_macro.find(sym) == _macro.end())
			{
				LOGERR("no use macro %s\n",sym);
				//continue;
			}
			if(SUCC != AddMacNetToCur(_macro[sym]) )
			{
				LOGERR("AddMacNetToCur error!\n");
				return ERROR;
			}
		}
		else
		{
			LOGERR("ERROR: bad format: %s\n",line);
		}
	}
	fclose(fp);
	return SUCC;
}

int MkGraph::ConstructSearchNet(char *path,char *framesfile)
{
	char file[1024] ;
	memset(file,0x00,sizeof(file));
	sprintf(file,"%s/%s",path,framesfile);
	FILE *fp = fopen(file,"r");
	if(NULL == fp)
	{
		LOGERR("fopen %s failed.",file);
		return ERROR;
	}
	ParseNode *startnode = _search_net.GetNode((unsigned)_net_startid);
	if(NULL == startnode)
	{
		LOGERR("_search_net start node is NULL,error.\n");
		return ERROR;
	}
	_frames.push_back("NULL");
	int cur_index = 0;
	int flag_frames_inter = 0;
	char line[1024];
	while(memset(line,0x00,sizeof(line)) && NULL != fgets(line,sizeof(line),fp))
	{
		line[strlen(line)-1] = '\0';

		if( '#' == line[0] )
			continue;
		else if( ';' == line[0])
		{
			flag_frames_inter = 0;
			continue;
		}
		else if(BlankLine(line))
			continue;
		char *line_ptr = line;
		char str[128];
		memset(str,0x00,sizeof(str));
		line_ptr = GetStr(str,line);
		if(strncmp(str,"FRAME:",6) == 0)
		{
			if(flag_frames_inter != 0)
			{	
				LOGERR("frames %s file format is error.\n",file);
				return ERROR;
			}
			flag_frames_inter = 1;
			memset(str,0x00,sizeof(str));
			line_ptr = GetStr(str,line_ptr);
			_frames.push_back(str);
			cur_index++;
		}
		else if(strncmp(str,"NETS:",5) == 0)
		{
			if(flag_frames_inter == 0)
			{
				LOGERR("frames(%s) file line(%s) format is error.\n",file,line);
				return ERROR;
			}
			flag_frames_inter = 2;
		}
		else
		{
			if(flag_frames_inter != 2)
			{
				LOGERR("frames(%s) file format line(%s) is error.\n",file,line);
				return ERROR;
			}

			if(_net.find(str) == _net.end())
			{
				LOGERR("net(%s) not found.\n",str);
				continue;
			}
			//add arc
			int netid = _net[str];
			int netnodeid = _cnetid_to_snetid[netid];
			ArcInput input;
			ParseArc arc(input,-1 * cur_index,netnodeid,0.0);
			startnode->AddArc(arc);
		}
	}
	fclose(fp);
	return SUCC;
}

int MkGraph::AddNetToCur(int netid)
{
	int cur_nodeid = _cur_net.AddNode();
	int next_nodeid = _cur_net.AddNode();
	_cur_net_endnodeid = _cur_net.AddNode(1);
	_cur_line_nodeid = next_nodeid;
	ArcInput input(0,0);
	ParseArc arc(input,netid,next_nodeid,0.0);
	ParseNode *node = _cur_net.GetNode(cur_nodeid);
	node->AddArc(arc);
	return SUCC;
}

int MkGraph::AddMacNetToCur(int macroid)
{
	if(_macro_nodeid.size() < (unsigned)_num_macro)
		_macro_nodeid.resize(_num_macro);

	_cur_line_nodeid  = _cur_net.AddNode();
	_cur_net_endnodeid = _cur_net.AddNode(1);
	if(_macro_nodeid.size() <= (unsigned)macroid)
	{
		//LOGERR("it's bug!\n");
		//return ERROR;
		_macro_nodeid.resize(macroid+1);
	}
	_macro_nodeid[macroid] = _cur_line_nodeid;
	return SUCC;
}

char *MkGraph::GetSymbol(char *sym,char *str,int *null_arc,int *loop_arc)
{
	while(isspace(*str) || *str == '(' )
		++str;
	int i=0;
	*null_arc = 0;
	*loop_arc = 0;
	while(!isspace(*str) && *str != ')' && *str != '\0' )
	{
		if(*str == '+')
		{
			str++;
			*loop_arc = 1;
			continue;
		}
		else if(*str == '*')
		{
			str++;
			*null_arc = 1;
			continue;
		}
		sym[i++] = *str;
		++str;
	}
	sym[i] = '\0';
	while(isspace(*str))
		++str;
	if(*str == '\0' || *str == ')')
		return NULL;
	else
		return str;
}

float MkGraph::GetWeight(char *sym)
{
	char str[128];
	int flag = 0;
	memset(str,0x00,sizeof(str));
	int j = 0;
	for(int i=0 ; sym[i] != '\0' ; ++i)
	{
		if(1 == flag && sym[i] != '}')
			str[j++] = sym[i];
		if(sym[i] == '{')
			flag = 1;
		if(1 == flag)
			sym[i] = '\0';
	}
	if(j>0)
	{
		float weight = 0;
		str[j] = '\0';
		sscanf(str,"%f",&weight);
		if(weight - 1e-5 < 0)
			LOGERR("It's too little,you should change it big.");
		return weight;
	}
	return 1.0;
}

int MkGraph::AddLineToNet(char *line)
{
	char sym[128];
	memset(sym,0x00,sizeof(sym));
	char *line_ptr = line;
	_cur_nodeid = _cur_line_nodeid;
	do{
		int nullarc =0,looparc =0;
		// analysis string
		line_ptr = GetSymbol(sym,line_ptr,&nullarc,&looparc);
		float weight = GetWeight(sym);
		int word =0 , net = 0;
		if(sym[0] == '[')
		{
			if(_net.find(sym) == _net.end())
			{
				LOGERR("net %s isn't find.\n",sym);
				return ERROR;
			}
			net = _net[sym];
		}
		else if(isupper((int)sym[0]))
		{
			if(_macro.find(sym) == _macro.end())
			{
				_macro[sym] = _num_macro++;
			}
			net = -1 * _macro[sym];
		}
		else
		{
			if(_dict.find(sym) == _dict.end())
			{
				if(_dict_index.size() == 0)
					_dict_index.push_back("null");
				_dict[sym] = _dict_index.size();
				_dict_index.push_back(sym);
			}
			word = _dict[sym];
		}
		ArcInput input(word,net);
/*
		if(input._net != 0)
			weight = 1;
		else
			weight = 1;
*/		
		// add node
		int tonodeid = 0;
		if(NULL == line_ptr) 
		{ // sentence end
			tonodeid = _cur_net_endnodeid;
		}
		else
		{
			tonodeid = _cur_net.AddNode();
		}
		// add arc
		// first add loop arc, because it can be end the sentence 
		if(looparc == 1)
		{ // because if have more 2 + at start, it possible have two loop in  only one node,it's a bug and make search very slow.
			// so I change the loop add next node , and in the end I add a eps arc.
/*old
			ParseArc larc(input,0,_cur_nodeid,0.8);
			node->AddArc(larc);
*/
			// if it's sentence end add a node
			if(NULL == line_ptr)
			{
				tonodeid = _cur_net.AddNode();
				ParseArc larc(input,0,tonodeid,0.8);
				ArcInput epsinput(0,0);
				ParseArc insert2endarc(epsinput,0,_cur_net_endnodeid,0);
				ParseNode *tonode = _cur_net.GetNode(tonodeid);
				tonode->AddArc(larc);
				tonode->AddArc(insert2endarc);
			}
			else
			{
				ParseArc larc(input,0,tonodeid,0.8);
				ParseNode *tonode = _cur_net.GetNode(tonodeid);
				tonode->AddArc(larc);
			}
		}
		ParseNode *node = _cur_net.GetNode(_cur_nodeid);
		if(NULL == node)
		{
			LOGERR("%s(%d) node is not exist.\n",line,_cur_nodeid);
			exit(-1);
		}
		ParseArc arc(input,0,tonodeid,weight);
		node->AddArc(arc);
		if(nullarc == 1)
		{
			ArcInput ninput(0,0);
			ParseArc narc(ninput,0,tonodeid,0.0);
			node->AddArc(narc);
		}
		if(NULL != line_ptr)
			_cur_nodeid = tonodeid;
		else
			break;
	}while(1);
	return SUCC;
}

void MkGraph::ProcessCurNet()
{
#ifdef DEBUG
	PrintCurNet();
#endif
	if(_search_net.GetNetSize() == 0)
		_net_startid = _search_net.AddNode();
	//first put _cur_net node to _search_net
	//and create node map relation.
	//depth-fist traversal
	int cur_nodeid = 0;
	ParseNode *cur_node = _cur_net.GetNode(cur_nodeid);

	ParseArc *arc = cur_node->GetArc(0);
	int netid = arc->_output;
	int search_nodeid =_search_net.AddNode();
	_c_map_s[cur_nodeid] = search_nodeid;
	//do this for _net variable map to new net start node id.
	_cnetid_to_snetid[netid] = search_nodeid;

	//if(netid == _net["[Number]"])
	//	printf("[Number] %d\n",netid);
	ExtendCurNetToSearchNet(cur_nodeid,1.0,-1);
	ResetCurInfo();
	return ;
}
/****************
 * curnodeid :_cur_net is extending node id.
 * macrotonodeid : if -1,isn't macro arc extend,else it's macro's end node id.
 * */
void MkGraph::ExtendCurNetToSearchNet(int curnodeid,float scale, int macrotonodeid)
{
	ParseNode *cur_node = _cur_net.GetNode(curnodeid);
	int final = 0;
	int search_nodeid = -1;
	if(_c_map_s.find(curnodeid) != _c_map_s.end())//find it
	{
		search_nodeid = _c_map_s[curnodeid];
	}
	else
	{
		// it's bug
		LOGERR("it's a seriousness bug.\n");
		exit(-1);
	}
	int search_tonodeid = -1;
	int tonodeid = -1;
	int toflag = 0;// if tonodeid have been in _c_map_s,toflag = 1.
	unsigned int i=0;
	// first traverse node and add node and add arc
	for(i=0;i<cur_node->GetArcSize();++i)
	{
		ParseArc *arc = cur_node->GetArc(i);
		tonodeid = arc->_to;
		ParseNode *tonode = _cur_net.GetNode(tonodeid);
		if(tonode->IsFinal() )
			final = 1;
		else
			final = 0;

		if(tonode->IsFinal() && macrotonodeid != -1)
		{
			// macro end
			if(_c_map_s.find(macrotonodeid) == _c_map_s.end())
			{
				LOGERR("it's a bug\n");
			}
			search_tonodeid = _c_map_s[macrotonodeid];
			toflag = 1;
		}
		else
		{
			if(_c_map_s.find(tonodeid) == _c_map_s.end())
			{
				// add node
				search_tonodeid = _search_net.AddNode(final);
				_c_map_s[tonodeid] = search_tonodeid;
				toflag = 0;
			}
			else
			{
				search_tonodeid = _c_map_s[tonodeid];
				toflag = 1;
			}
		}
		if(arc->_input._net < 0)
		{// access macr
			// for macro in macro,and macro is macro end
			if(tonode->IsFinal() && macrotonodeid != -1)
			{
				tonodeid = macrotonodeid;
			}
			int macroid = -1 * arc->_input._net;
			if(_c_map_s.find(_macro_nodeid[macroid]) == _c_map_s.end())
			{
				_c_map_s[_macro_nodeid[macroid]] = search_nodeid;
			}
			else
			{// macro can't be nested themselves
				LOGERR("have macro loop,it's error!\n");
				exit(-1);
			}

			ExtendCurNetToSearchNet(_macro_nodeid[macroid],scale * arc->_w,
					tonodeid);
//			_c_map_s.erase(_macro_nodeid[macroid]);
		}
		else// it's not macro
		{
			ParseArc newarc(arc->_input,arc->_output,search_tonodeid,scale * arc->_w);
			// if same arc have been add, not add.this is not optimal.
			// this is possible a bug

			// add arc to search_node
			ParseNode *search_node = NULL;
			search_node = _search_net.GetNode(search_nodeid);
			if(!search_node->HaveSameArc(newarc))
				search_node->AddArc(newarc);
			else
			{// if line start '+' and the word is same,have same arc.it's same loop.
				LOGERR("this is possible a bug\n");
//				PrintCurNet();
			}
		}
		if(tonode->IsFinal()) // end this edge.
			continue;
		if(curnodeid == tonodeid) // if it's a loop,I need some process.
   			continue;
		// beacuse a node possible extend repeatedly,
		// if a node can be arrive repeatedly
		// the same node can be extend repeatedly.
		if(toflag == 0)
			ExtendCurNetToSearchNet(tonodeid , scale , macrotonodeid);
		
	} // node all arc traverse
	// this is macro exit. macro inter node _c_map_s should be erase.
	// add this, because macro node must be uniq. 
	// Every arrive a new macro node ,must be realloc.
	// if have loop,it,s bug ,and code can be process this situation.
	if(macrotonodeid != -1)
	{
		if(_c_map_s.find(curnodeid) != _c_map_s.end())
			// && _macro_nodeid.size() > 1 && curnodeid >= _macro_nodeid[1])
		{
	   		_c_map_s.erase(curnodeid);//tonodeid);
		}
		else
		{
			LOGERR("it should be a bug. macrotonodeid:%5d,curnodeid:%3d,_macro_nodeid%3d\n",macrotonodeid,curnodeid,_macro_nodeid[1]);
		}
	}
	return;
}

void MkGraph::PrintCurNet()
{
	//marc list
#ifdef DEBUG
	printf("******macro start******\n");
	for(auto it = _macro.begin();it != _macro.end();++it)
		cout << it->first << " " << it->second << " node id " <<_macro_nodeid[it->second] << endl;
	printf("******macro end******\n");
#endif
	unsigned int i=0,j=0;
	for(i=0;i<_cur_net.GetNetSize();++i)
	{
		ParseNode *node = _cur_net.GetNode(i);
		for(j=0;j<node->GetArcSize();++j)
		{
			ParseArc *arc = node->GetArc(j);
			printf("%3d (%3d,%3d) %3d %3d %6f\n",i,arc->_input._word,
					arc->_input._net,arc->_output,arc->_to,arc->_w);
		}
		if(node->IsFinal())
			printf("%3d %3d\n",i,-1);
	}
	fflush(stdout);
}

void MkGraph::ResetCurInfo()
{
	_net_inter = 0;
	_num_macro = 1;
	_macro.clear();
	_macro_nodeid.clear();

	_c_map_s.clear();
	
	_cur_line_nodeid = 0;
	_cur_nodeid = 0;
	_cur_net_endnodeid = 0;
	_cur_net.Clear();
}

void MkGraph::SortSearchNet()
{
	unsigned i=0;
	for(i=0;i<_search_net.GetNetSize();++i)
	{
		ParseNode *node = _search_net.GetNode(i);
		node->SortArc(0,node->GetArcSize()-1);
	}
}

int BaseGraph::WriteGraph(char *netfile,char *dictfile,char *netindexfile,char *framesfile)
{
	FILE *fp = fopen(dictfile,"w");
	if(fp == NULL)
	{
		LOGERR("dictfile %s open file.\n",dictfile);
		return ERROR;
	}
	unsigned int i=0;
	for(i=0;i<_dict_index.size();++i)
		fprintf(fp,"%-6d %-10s\n",i,_dict_index[i].c_str());
	fclose(fp);

	fp = fopen(netindexfile,"w");
	if(fp == NULL)
	{
		LOGERR("netindexfile %s open file.\n",netindexfile);
		return ERROR;
	}
	for(i=0;i<_net_index.size();++i)
		fprintf(fp,"%-6d %-10s %-6d\n",i,_net_index[i].c_str(),_cnetid_to_snetid[i]);
	fclose(fp);

	fp = fopen(netfile,"w");
	if(fp == NULL)
	{
		LOGERR("netfile %s open file.\n",netfile);
		return ERROR;
	}
	for(i=0;i<_search_net.GetNetSize();++i)
	{
		unsigned int j=0;
		ParseNode *node = _search_net.GetNode(i);
		if(node == NULL)
			continue;
		for(j=0 ; j<node->GetArcSize() ; ++j)
		{
			ParseArc *arc = node->GetArc(j);
		   //"input node id" "arc input word id" "arc input net id"
		   //"arc output net id" "output node id" "weight"
#ifdef DEBUGGRAPH
			fprintf(fp,"%-5d %-10s %-10s %-10s %-5d %-6.3f\n",i,_dict_index[arc->_input._word].c_str(),
					_net_index[arc->_input._net].c_str(),
					arc->_output > 0 ? _net_index[arc->_output].c_str():_frames[-1 * arc->_output].c_str(),arc->_to,arc->_w);
#else
			fprintf(fp,"%-5d %-5d %-5d %-5d %-5d %-6.3f\n",i,arc->_input._word,
					arc->_input._net,arc->_output,arc->_to,arc->_w);
#endif
		}
		if(node->IsFinal())
			fprintf(fp,"%-5d %-5d\n",i,-1);
	}
	fclose(fp);
	
	if(framesfile != NULL)
	{
		fp = fopen(framesfile,"w");
		if(fp == NULL)
		{
			LOGERR("framesfile(%s) open file.\n",framesfile);
			return ERROR;
		}
		for(i=0 ; i<_frames.size() ; ++i)
			fprintf(fp,"%-5d %-10s\n",i,_frames[i].c_str());
		fclose(fp);
	}
	return SUCC;
}


int BaseGraph::ReadGraph(char *netfile,char *dictfile,char *netindexfile,char *framesfile)
{
	FILE *fp = fopen(dictfile,"r");
	if(NULL == fp)
	{
		LOGERR("dictfile %s open file.\n",dictfile);
		return ERROR;
	}
	char line[1024];
	char str[128];
	memset(str,0x00,sizeof(str));
	memset(line,0x00,sizeof(line));
	int id = -1;
	while(NULL != fgets(line,sizeof(line),fp))
	{//dictfile index must be start from 0 and continuous.
		line[strlen(line)-1] = '\0';
		if (2 !=sscanf(line,"%d %s",&id,str))
		{
			LOGERR("sscanf %s:%s error!\n",dictfile,line);
			return -1;
		}
		_dict_index.push_back(str);
		_dict[str] = id;
		memset(str,0x00,sizeof(str));
		memset(line,0x00,sizeof(line));
		id = -1;
	}
	fclose(fp);

	fp = fopen(netindexfile,"r");
	if(fp == NULL)
	{
		LOGERR("netindexfile %s open file.\n",netindexfile);
		return ERROR;
	}
	int id1 = -1,id2 = -1;
	while(NULL != fgets(line,sizeof(line),fp))
	{//netindexfile index must be start from 0 and continuous.
		line[strlen(line)-1] = '\0';
		if(3 != sscanf(line,"%d %s %d",&id1,str,&id2))
		{
			LOGERR("sscanf %s:%s error!\n",netindexfile,line);
			return ERROR;
		}
		_net_index.push_back(str);
		_cnetid_to_snetid[id1] = id2;
		_net[str] = id1;
		memset(str,0x00,sizeof(str));
		memset(line,0x00,sizeof(line));
		id1 = id2 = -1;
	}
	fclose(fp);

	fp = fopen(netfile,"r");
	if(fp == NULL)
	{
		LOGERR("netfile %s open file.\n",netfile);
		return ERROR;
	}
	while(NULL != fgets(line,sizeof(line),fp))
	{
		int elem1 = -1,elem2 = -1,
			elem3 = -1,elem4 = -1,elem5 = -1;
		float w = 0.0;
		int ret = 0;
		ret = sscanf(line,"%d %d %d %d %d %f",
				&elem1,&elem2,&elem3,&elem4,&elem5,&w);
		while(_search_net.GetNetSize() < (unsigned)elem1+1)
			_search_net.AddNode();
		ParseNode *node = _search_net.GetNode((unsigned)elem1);
		if(NULL == node)
		{
			LOGERR("it's bug.|%s|\n",line);
			return ERROR;
		}
		if(6 == ret)
		{
			ArcInput input(elem2,elem3);
			ParseArc arc(input,elem4,elem5,w);
			node->AddArc(arc);
		}
		else if(2 == ret)
		{
			node->SetNodeFinal();
		}
		else
		{
			LOGERR("sscanf %s:%s error!\n",netfile,line);
			return ERROR;
		}
		memset(line,0x00,sizeof(line));
	}
	fclose(fp);

	if(framesfile != NULL)
	{
		fp = fopen(framesfile,"r");
		if(fp == NULL)
		{
			LOGERR("framesfile(%s) open file.\n",framesfile);
			return ERROR;
		}
		while(NULL != fgets(line,sizeof(line),fp))
		{//dictfile index must be start from 0 and continuous.
			line[strlen(line)-1] = '\0';
			if (2 !=sscanf(line,"%d %s",&id,str))
			{
				LOGERR("sscanf %s:%s error!\n",framesfile,line);
				return ERROR;
			}
			_frames.push_back(str);
			memset(str,0x00,sizeof(str));
			memset(line,0x00,sizeof(line));
			id = -1;
		}
		fclose(fp);
	}	
	return SUCC;
}



