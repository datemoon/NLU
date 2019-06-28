#ifndef __PARSEDECODE_H__
#define __PARSEDECODE_H__ 1

/*
 * this is a breadth traversal way decoder and it can work.
 **/
#include <algorithm>
#include "mkgraph.h"
#include "convert.h"

class ParseDecode:public BaseGraph
{
	friend int convert2xml(char *text, char* sen,char *xml_buff,char *history);
private:
	struct NetBound;
	class Token;
public:
	ParseDecode(): _search_start_tok(NULL),_num_toks(0), _num_links(0),  _num_word(0) { }

	~ParseDecode(){_num_word =0;_num_links = 0 ;_num_toks=0;_search_start_tok = NULL;}

	int Decoder(char *sent);
private:

	// initialize decoder source.
	void Init();

	// process non-empty edge
	void ProcessEmitting(int wordid);

	// process empty edge
	void ProcessNonemitting(int end = 0);

private:
	/**************
	 * search ordered arc.
	 **************/
	void SortEmitting(int wordid);

	// find word in current node .
	// return >= 0 find ,else no find.
	int FindWordArc(ParseNode *node,int wordid);

	// find or add token to history.
	Token* FindOrAddToken(int nodeid,float tot_cost,
			vector<NetBound> & net_bound, float tot_scale);

	// because current token can director current token ,
	// so must be add some recursion.
	// in same wordid,first execute PruneForwardLinkCurToCur ,
	// after PruneForwardLinkForWord
	void PruneForwardLinkCurToCur(int wordid);

	// judge whether delete this link.
	bool JudgeUseLink(Token *tok );

	// clear all active tokens
	void ClearActiveTokens();

	// cut off token if current token is not final
	// and it have not next token. From first word start
	// and must be continue cut
	void PruneActiveToken(int num_word);

	// prune next word token before ,must be execute this function.
	void PruneForwardLinkForWord(int wordid);

	// because "_active_toks _toks" is reverse add, so eps edge arrive
	// node at "_toks" front,so this function can be right work.
	void PruneTokenForWord(int wordid);

private:
	struct ForwardLink
	{
		ArcInput _ilabel;
		int      _olabel;
		float    _graph_cost;
		Token *  _next_tok;
		ForwardLink *_next;//reverse save.first in last out.

		ForwardLink():
			_ilabel(0,0),_olabel(0),
			_graph_cost(0.0),_next_tok(NULL), _next(NULL) { }

		~ForwardLink()
		{
			_ilabel = {0,0};
			_olabel=0;
			_graph_cost = 0.0;
		   	_next_tok = NULL;
			_next = NULL;
		}
		ForwardLink(ArcInput ilabel, int olabel, float graph_cost, 
				Token *next_tok,ForwardLink *next):
			_ilabel(ilabel), _olabel(olabel), _graph_cost(graph_cost),
			_next_tok(next_tok), _next(next) { }

	};

	inline Token * NewToken(int nodeid,float tot_cost,int wordid,
			Token* down,ForwardLink *links, 
			vector<NetBound> &net_bound, float tot_scale)
	{
		ParseNode *node = _search_net.GetNode(nodeid);
		int final =0;
		if(node->IsFinal())
			final = 1;
		Token *new_tok = new Token(nodeid,tot_cost, final, wordid,
			   	down, links, net_bound, tot_scale);
		
		_num_toks++;
		
		return new_tok;
	}
	struct NetBound
	{
		int _tonode;
		float _scale;
		NetBound():_tonode(0),_scale(1.0){}
		NetBound(int tonode,float scale):_tonode(tonode),_scale(scale){}
	};
	struct Token
	{
		int _nodeid;
		float _path_score;
		int _final;
		int _wordid;

		Token* _down;//reverse save.first in last out.
		ForwardLink *_links;
		vector<NetBound> _net_bound;//record access net skip tonode id
		float _tot_scale;

		Token(int nodeid, float path_score, int final, int wordid,
				Token* down, ForwardLink *links ,
				vector<NetBound> &net_bound, float tot_scale):
			_nodeid(nodeid), _path_score(path_score), _final(final),_wordid(wordid),
			_down(down), _links(links) ,_net_bound(net_bound),_tot_scale(tot_scale){ }
		Token()
			:_nodeid(0), _path_score(0),_final(0),_wordid(0),
			_down(NULL), _links(NULL), _net_bound(),_tot_scale(1.0){}
		~Token()
		{
			_nodeid = 0, _path_score = 0,_final = 0;
			_down = NULL,_links=NULL;
			_net_bound.clear();
			_tot_scale = 1.0;
		}
		inline bool IsEndToken()
		{
			if(_final == 1 && _net_bound.size() == 0 && _links == NULL)
				return true;
			else
				return false;
		}
		inline void DeleteForwardLinks()
		{
			ForwardLink *l = _links , *next;
			while(NULL != l)
			{
				next = l->_next;
				delete l;
				l = next;
			}
			_links = NULL;
		}
	};


	struct TokenList
	{
		Token *_toks;//reverse save.first in last out.
		bool _must_prune_forward_links;
		bool _must_prune_tokens;
		TokenList():_toks(NULL), _must_prune_forward_links(true), 
		_must_prune_tokens(true) { }
	};

#define DebugInfo() { \
	LOGDEBUG("@search_start_tok=%p,_num_toks=%d,_num_links=%d,_num_word=%d\n", \
				_search_start_tok,_num_toks,_num_links,_num_word); \
	}
	//record current _active_toks whether have start nodeid,
	//NULL have not ,else have.
	Token *_search_start_tok;
	// at current word nodeid map Token *,because nodeid can't be appears twice.
	// I think a nodeid can appear more then once ,except start nodeid
	// so I add variable _have_start_node flag. It's no neccssary.
	unordered_map<Token *,int> _map_toks;
	vector<Token *> _queue;
	
	int _num_toks;
	int _num_links;
	int _num_word;//how many words. init is 0.
	vector<TokenList> _active_toks;//Lists of tokens,indexed by word 

	//
	vector<string> _words_arr;
	vector<int> _wordid_arr;

private:
	unsigned SendSentence(char *sent);

	void PrintResult();

	void PrintToken(ParseDecode::Token *tok,int start);
private:
// At here,I use inversion link save result.
	struct ResNode;

	struct ResArc
	{
		int _inword;
		int _outnet;
		int _wordscore;
		ResNode *_prev_node;

		ResArc():_inword(0),_outnet(0),_wordscore(0.0),_prev_node(NULL){}
		ResArc(int inword,int outnet,int wordscore,ResNode *prev_node):
			_inword(inword),_outnet(outnet),_wordscore(wordscore),
			_prev_node(prev_node){}
	};
	struct ResNode
	{
		//arc
		int _inword;      // input word
		int _outnet;      // output net.
		float _wordscore; // word score.
		//
		int _nword;       // how many words ,start from 1.
		float _score;     // all path score.
		bool _is_end;     // whether it's net end.
		ResNode *_prev_node; // prev node

		ResNode():_inword(0),_outnet(0),_wordscore(0.0),
		_nword(0),_score(0.0),_is_end(false),_prev_node(NULL){}
		ResNode(int inword,int outnet,float wordscore,int nword,
				float score,bool is_end,ResNode *prev_node):
			_inword(inword),_outnet(outnet),_wordscore(wordscore),
			_nword(nword),_score(score),_is_end(is_end),
			_prev_node(prev_node){}

	};
	inline bool Comp(ResNode*A,ResNode*B)
	{
		return (A->_score > B->_score);
	}
	inline void Exchange(ResNode*&A,ResNode*&B)
	{
		ResNode*tmp = B;
		B = A;
		A = tmp;
	}
	void SortRes();
	void GetOneBest();

	typedef vector<ResNode*> CurWord;
	vector<CurWord> _reslist;    // record arrive current word score result path.
	vector<ResNode*> _allreslist;// record all end node for get best result.
	vector<ResNode*> _recordlist;// record all ResNode*,use delete
	vector<ResNode*> _onebest;   // record onebest result.

	void GetResStruct();
	inline void DelResStruct()
	{
		while(!_recordlist.empty())
		{
			ResNode* node = _recordlist.back();
			_recordlist.pop_back();
			delete node;
		}
		_reslist.clear();
		_allreslist.clear();
		_onebest.clear();

	}
	/****************
	 * if prev is null arc,prev node must be transfer,else prev is NULL.
	 * tok is cur node; num_word is word number; start expression start or not.
	 * return : Token is end or not. If word is end ,so do nothing.
	 ****************/
	void RecordToken(Token *tok,ResNode *prev,int num_word,int start);

	void ExchangeResFormat(string &res_str);
};

#endif
