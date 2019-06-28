#ifndef __MKGRAPH_H_ 
#define __MKGRAPH_H_ 1

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include "graph.h"

using namespace std;
using std::unordered_map;
//using std::string;

#define __func__ __FUNCTION__

#define LOGERR(format,args...) \
	fprintf(stderr,"file:%s line:%d func:%s:" format,__FILE__,__LINE__,__func__,##args)

#ifdef DEBUG
#define LOGDEBUG(format,args...) \
	fprintf(stderr,"file:%s line:%d func:%s:" format,__FILE__,__LINE__,__func__,##args)
#else
#define LOGDEBUG(format,args...)
#endif

#define ERROR (-1)
#define SUCC  0
class BaseGraph
{
public:
	friend class ParseDeepDecode;


	BaseGraph():_net_startid(0){}
	virtual ~BaseGraph(){}
	//IO relation function.
	virtual int WriteGraph(char *netfile,char *dictfile,char *netindexfile,char *framesfile = NULL);
	virtual int ReadGraph(char *netfile,char *dictfile,char *netindexfile,char *framesfile);
	// cut line to str
	char * GetStr(char *str,char *line)
	{
		while(isspace(*line)) line++;
		while( !isspace(*line) && *line != '\0' && *line != '\n') *str++ = *line++;
		*str = '\0';
		return line;
	}
	int GetNetNodeId(int nodeindex)
	{
		return _cnetid_to_snetid[nodeindex];
	}
protected:
	int _net_startid;//forever is 0.
	ParseNet _search_net;
	
	unordered_map<string/* word*/,int /*wordid*/> _dict;//start 1
	vector<string> _dict_index;//id 0 is null.
	
	vector<string> _net_index;//id 0 is null.
	unordered_map<string /*net*/, int /*netid*/> _net;//start 1
	unordered_map<int,int> _cnetid_to_snetid; //_net variable map to _cur_netid_to_search_netid, in BaseGraph it's a vector

	vector<string> _frames;//0 is not use,index is 1,2,3...,and in _search_net ,output is negative.
};

class MkGraph:public BaseGraph
{
public:

	MkGraph();
	//MkGraph(const Mkgraph & cp);
	~MkGraph(){}
	/*************
	 * construct foundation graph
	 *************/
	int ConstructGraph(char *path,char *grammarfile);

	/*************
	 * construct search net from foundation graph and frames file.
	 *
	 *************/
	int ConstructSearchNet(char *path,char *framesfile);

	/*************
	 * sort net every node all arc,it's ascending order.
	 *************/
	void SortSearchNet();
private:
	float GetWeight(char *sym);

	int GetFileNetNet(char *grammarfile);
	int BlankLine(char *s){
		while( *s && isspace( (int) *s) ) s++;
		if( !*s ) return(1);
		else return(0);
	}
	void GetFileNetName(char *grammarfile);
	
	inline int GetNetNum() {return _net_index.size();}

	int WriteNet(char *net);
	int ReadNet(char *net);
	int GetWordId(char *word)
	{
		if(_dict.find(word) == _dict.end())
		{
			LOGERR("(%s) word is not find\n",word);
			return -1;
		}
		return _dict[word];
	}
	int GetNetId(char *net)
	{ 
		if(_net.find(net) == _net.end())
		{
			LOGERR("%s net is not find\n",net);
			exit(-1);
		}
		return _net[net]; 
	}
	int GetMacId(char *macro)
	{
		if(_macro.find(macro) == _macro.end())
		{
			LOGERR("%s macro is not find\n",macro);
			exit(-1);
		}
		return _macro[macro];
	}

	// add net to current net ,this is a initialize current net function.
	int AddNetToCur(int netid);//it's the same as init
	
	/******************
	 * line : file current line
	 * 
	 * analysis "line" and output to _cur_net
	 ******************/
	int AddLineToNet(char *line);
	
	/*************
	 * macroid : macro index
	 *
	 * macroid storage in _macro_nodeid
	 *************/
	int AddMacNetToCur(int macroid);
	
	/**************
	 * sym :get word from str,memory malloc in external.
	 * str :input string
	 * null_arc :sym whether have *
	 * loop_arc :sym whether have +
	 * 
	 * return :next str point for next call GetSymbol.
	 ************/
	char *GetSymbol(char *sym,char *str,int *null_arc,int *loop_arc);

	// process current net.
	// Because the net have macro , so I need extend these macro,
	// the function have this function.
	// But now this function don't process macro net the first word have '+'.
	void ProcessCurNet();

	// extend current net to total net
	void ExtendCurNetToSearchNet(int curnodeid,float scale,int serachtonodeid);
	
	// print current net info
	void PrintCurNet();

	// reset current net state.
	void ResetCurInfo();

private:

	int _net_inter;//0 is exter,1 is inter
	int _num_macro;//init is 1,need reset.
//	unordered_map<string/* word*/,int /*wordid*/> _dict;//start 1
//	vector<string> _dict_index;//id 0 is null.

//	unordered_map<string /*net*/, int /*netid*/> _net;//start 1
//	vector<string> _net_index;//id 0 is null.
//	unordered_map<int,int> _cnetid_to_snetid; //_net variable map to _cur_netid_to_search_netid, in BaseGraph it's a vector

	unordered_map<string/* macro*/,int /*macroid*/> _macro;//start 1 2 3
	vector<int> _macro_nodeid;//through _macro find macroid,through macroid,find macro start nodo id

	//int _net_startid;//forever is 0.
	//ParseNet _search_net;
	unordered_map<int,int> _c_map_s;//_cur_netnodeid_map_search_netnodeid;need clear.

	//current net parameter
	int _cur_line_nodeid;//init is 0.record current line start node id.
	int _cur_nodeid;//init is 0.
	int _cur_net_endnodeid;//init is 0.
	ParseNet _cur_net;//current compile grammar net
};

#endif
