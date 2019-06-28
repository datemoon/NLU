#ifndef __PARSE_DEEP_SEARCH_DECODE_H__
#define __PARSE_DEEP_SEARCH_DECODE_H__ 1

#include "mkgraph.h"
#include "parse_util.h"

class ParseDeepDecode:public Sentence //,public BaseGraph
{
private:
	struct Token;
	struct NetBound;
public:
	ParseDeepDecode():
		_graph(NULL), _stack_toks(100),_num_word(0),_tot_scale(1.0), _best_score(0.0), _best_tok(-1){}

	ParseDeepDecode(BaseGraph *graph):
		_graph(graph),  _stack_toks(100),_num_word(0),_tot_scale(1.0), _best_score(0.0), _best_tok(-1){}
	~ParseDeepDecode(){}
	void Decoder(const char *sent);

	float Decoder(const char *sent, char *const result);

	// here I use recursion way ,deep traverse this net.
	int ExpendPath(int tokid);

	int Init(BaseGraph *graph = NULL);
	
	void Reset();
	
	int ProcessEmitting(int tokid);
	int ProcessNonEmitting(int tokid);
private:
	
	int AddToken(int nodeid,float path_score, int prev, int wordid, int onet , bool final);

	int FindWordArc(ParseNode *node,int wordid);
	void DeleteToken();
	void GetOneBest();
	void PrintOneBest();
	void PrintToken(Token &tok);

	void ExchangeResFormat(string &res);

	void ExchangeResFormatXML(string &res);

	void ExchangeResFormatXMLOld(string &res);
private:
	BaseGraph *_graph;
private:
	struct Token
	{
		int _nodeid;
		float _path_score;
		int _wordid;
		int _onet;
		bool _final;
		int _prev;
		Token()
			:_nodeid(0),_path_score(0.0),_wordid(0),
			_onet(0), _final(false),_prev(-1){}
		void PrintTok(){ printf("%3d %5.2f %5d %3d %d %5d\n",_nodeid,_path_score,_wordid,_onet,_final==true?1:0,_prev);}

		Token& operator=(const Token &tok )
		{
			_nodeid = tok._nodeid; _path_score=tok._path_score;
			_wordid = tok._wordid; _onet = tok._onet;
			_final = tok._final; _prev = tok._prev;
			return *this;
		}
	};
	struct NetBound
	{
		int _tonode;
		float _scale;
		NetBound():_tonode(0),_scale(1.0){}
		NetBound(int tonode,float scale):_tonode(tonode),_scale(scale){}
	};

	vector<NetBound> _stack_netjump; // tonodeid

	vector<Token> _stack_toks;

	int _num_word;
	float _tot_scale;

	// result relation member
	// map _stack_toks
	vector<int> _res_tok_list;
	float _best_score;
	int _best_tok;

	vector<int> _onebest;

};




#endif
