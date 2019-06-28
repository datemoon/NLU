#include <string.h>
#include "parsedecode.h"


ParseDecode::Token* ParseDecode::FindOrAddToken(int nodeid,float tot_cost,
		vector<NetBound> & net_bound, float tot_scale)
{
	assert((unsigned)_num_word < _active_toks.size() && "num_word greater then active_toks size.");
	Token *toks = _active_toks[_num_word]._toks;
	// now don't to repeat nodeid.
	if(nodeid == _net_startid &&  _search_start_tok != NULL)
	{
		LOGERR("************it's bug**************\n");
		Token *tmp_tok = _search_start_tok;
		if(tmp_tok->_path_score < tot_cost)
			tmp_tok->_path_score = tot_cost;
		return tmp_tok;
	}
	else
	{
//		ParseNode *node = _search_net.GetNode(nodeid);
//		int final =0;
//		if(node->IsFinal())
//			final = 1;
		Token *new_tok = NewToken(nodeid,tot_cost,_num_word, toks, NULL, 
				net_bound, tot_scale);
		//new_tok->_net_bound = net_bound;
		_active_toks[_num_word]._toks = new_tok;

		return new_tok;
	}
}
void ParseDecode::ProcessNonemitting(int end)
{
	assert(!_active_toks.empty() && "_active_toks is NULL,it's error.");
//	int cur_num_word = _num_word;

	assert(_queue.empty());
	// for next word prepare, 
	// add start node to current TokenList at _active_toks,
	// if _active_toks have not _net_startid.

	if( end != 1 && _search_start_tok == NULL)
	{
		int start = _net_startid;
		vector<NetBound> _null_vector(0);
		Token *start_tok = FindOrAddToken(start,0.0,_null_vector,1.0);
		_search_start_tok = start_tok;
		_queue.push_back(start_tok);
	//	ProcessNonemitting();
	}

//	for(auto it = _map_toks.begin(); it != _map_toks.end();++it)
//		_queue.push_back(static_cast<int>(it->first));

	for(Token * trav_tok = _active_toks[_num_word]._toks ; 
			trav_tok != NULL ; trav_tok = trav_tok->_down)
	{
		_queue.push_back(trav_tok);
	}

	// it's net propagates	
	Token *new_tok = NULL;

	while(!_queue.empty())
	{
		Token * tmp_tok = _queue.back();
		_queue.pop_back();
		int nodeid = tmp_tok->_nodeid;
		assert(tmp_tok != NULL && "node shouldn't be NULL,it's bug.");

		ParseNode *node = _search_net.GetNode(nodeid);
		// now it's not necessary ,because token is unique.
		//if "tmp_tok" has any existing forward links, delete them,
		//because we're about to regenerate them.  This is a kind
		//of non-optimality (remember, this is the simple decoder),
		//but since most states are emitting it's not a huge issue.
		//tmp_tok->DeleteForwardLinks();//necessary when re-visiting
		//tmp_tok->_links = NULL;
		
		//if node is final and GetArcSize() is zero and _net_bound.size() == 0
		//node is final node and do nothing.
		if(node->IsFinal() && (0 == node->GetArcSize()) 
				&& tmp_tok->_net_bound.size() == 0 )
		{
			//new_tok = _search_start_tok;
			
			//ArcInput input(0,0);
			//tmp_tok->_links = new ForwardLink(input,
			//		0 , 0.0 , new_tok, tmp_tok->_links);
			//_num_links++;
			continue;
		}
		else if(node->IsFinal() && (0 == node->GetArcSize()) 
				&& tmp_tok->_net_bound.size() != 0 )
		{
			// net jump out
			NetBound &net_bound = tmp_tok->_net_bound.back();
			int tonode = net_bound._tonode;
			float scale = net_bound._scale;
			tmp_tok->_net_bound.pop_back();
			
			float tot_scale = tmp_tok->_tot_scale / scale;
			float tot_cost = tmp_tok->_path_score;
			new_tok = FindOrAddToken(tonode ,tot_cost,
					tmp_tok->_net_bound,tot_scale);
			_queue.push_back(new_tok);
			ArcInput input(0,0);
			tmp_tok->_links = new ForwardLink(input,
					0 , 0.0 , new_tok, tmp_tok->_links);
			_num_links++;
			continue;
		}
		
		int i = 0;
		for(i=0; i < static_cast<int>(node->GetArcSize()); ++i)
		{
			ParseArc *arc = node->GetArc((unsigned)i);
			float cur_cost = tmp_tok->_path_score;
			float cur_scale = tmp_tok->_tot_scale;
			
			//if input word is 0,it's null arc,else it's non null.
			if(arc->_input._word == 0)
			{
				// null weight not add tot_cost
				float tot_cost = cur_cost;

				int tonode = 0;
				if(arc->_input._net != 0)
				{
					new_tok = FindOrAddToken(GetNetNodeId(arc->_input._net),
							tot_cost, tmp_tok->_net_bound, cur_scale * arc->_w);
					NetBound net_jump_in(arc->_to, arc->_w);
				
					new_tok->_net_bound.push_back(net_jump_in);
					tonode = GetNetNodeId(arc->_input._net);//find net map start node
				}
				else
				{
					// add arc to current token ForwardLink and next _active_toks
					// if find token already exists,replace relation variable.
					assert(arc->_w == 0);
					new_tok = FindOrAddToken(arc->_to,tot_cost,
							tmp_tok->_net_bound , cur_scale);
					tonode = arc->_to;
				}
				tmp_tok->_links = new ForwardLink(arc->_input,
						arc->_output,0.0,new_tok,tmp_tok->_links);
				_num_links ++;
				//add new nodeid to _queue
				_queue.push_back(new_tok);
			}
			// because it's ordered , so if it's not 0,I can break .
			else
				break;
		}
	}
}

int ParseDecode::FindWordArc(ParseNode *node,int wordid)
{
	int start = 0,end = node->GetArcSize()-1;
	int mid = (start+end) /2;
	while(start <= end)
	{
		ParseArc *arc = node->GetArc((unsigned)mid);
		if(arc->_input._word > wordid)
		{
			end = mid - 1;
			mid = (start+end) /2;
		}
		else if(arc->_input._word < wordid)
		{
			start = mid + 1;
			mid = (start+end) /2;
		}
		else
			break;
	}
	if(start > end)
		return -1;// no search
	ParseArc *arc = node->GetArc((unsigned)mid);
	if(arc->_input._word == wordid)
	{
		// search first arc _word == wordid arcid.
		int i = 0;
		for(i = mid-1 ; i>=0; --i)
		{
			arc = node->GetArc((unsigned)i);
			if(arc->_input._word == wordid)
				continue;
			else
				break;
		}
		++i;
		return i;
	}
	return -1;
}
void ParseDecode::SortEmitting(int wordid)
{
	for(Token *tok = _active_toks[_num_word-1]._toks ; tok != NULL ; tok = tok->_down)
	{
		int nodeid = tok->_nodeid;
		ParseNode *node = _search_net.GetNode(nodeid);
		float scale = tok->_tot_scale;
		float cur_cost = tok->_path_score;
		 // Here I can do some speed up,because arc is continuous save,
		 // so I can search through binary search, if it's ordered.
		 // I will do some work in mkgraph ,make it;s ordered.

		int i = FindWordArc(node,wordid);
		if(i < 0)
			continue; ;

		for(; i < static_cast<int>(node->GetArcSize()); ++i)
		{
			ParseArc *arc = node->GetArc((unsigned)i);
			if(arc->_input._word == wordid)// if searchword at arc.
			{
				float arc_weight = arc->_w * scale;
				float tot_cost = cur_cost + arc_weight;
				Token *new_tok = FindOrAddToken(arc->_to,tot_cost,
						tok->_net_bound,scale);
				tok->_links = new ForwardLink(arc->_input,
						arc->_output, arc_weight, new_tok, tok->_links);
				_num_links++;
			}
			else
				break;
		}//for
	}//for
}

void ParseDecode::ProcessEmitting(int wordid)
{
	_num_word++;
	//the zero is start ,first word id is 1.
	assert(static_cast<int>(_active_toks.size()) == _num_word && "it's error.");
	_active_toks.resize(_active_toks.size() + 1);
//	_have_start_node = 0;

	_search_start_tok = NULL;
//	_map_toks.clear();

	SortEmitting(wordid);
	return ;
	//expend all token in previous word.
	for(Token *tok = _active_toks[_num_word-1]._toks ; tok != NULL ; tok = tok->_down)
	{
		int nodeid = tok->_nodeid;
		ParseNode *node = _search_net.GetNode(nodeid);

		float scale = tok->_tot_scale;
		float cur_cost = tok->_path_score;
		int i=0;
//		i = FindWordArc(node,wordid);
//		if(i < 0)
//			continue; ;
		// Here I can do some speed up,because arc is continuous save,
		// so I can search through binary search, if it's ordered.
		// I will do some work in mkgraph ,make it;s ordered.
		for(; i < static_cast<int>(node->GetArcSize()); ++i)
		{
			ParseArc *arc = node->GetArc((unsigned)i);
			if(arc->_input._word == wordid)// if search word at arc.
			{
				float arc_weight = arc->_w * scale;

				float tot_cost = cur_cost + arc_weight;
				Token *new_tok = FindOrAddToken(arc->_to,tot_cost,
						tok->_net_bound,scale);

				tok->_links = new ForwardLink(arc->_input,
						arc->_output, arc_weight, new_tok, tok->_links);
				_num_links++;
			}
		}//all arc
	}//all token
}

void ParseDecode::Init()
{
	ClearActiveTokens();

	int start = _net_startid;
	_num_toks = 0;
	_num_links = 0;
	_num_word = 0;

	vector<NetBound> net_bound(0);
	Token *start_tok = NewToken(start,0.0,_num_word,NULL,NULL,net_bound,1.0);

	_search_start_tok = start_tok;

	_active_toks.resize(1);
	_active_toks[0]._toks = start_tok;
	ProcessNonemitting();
}

void ParseDecode::ClearActiveTokens()
{
	int i=0;
	for(i=0;i<static_cast<int>(_active_toks.size());++i)
	{
		for(Token *tok = _active_toks[i]._toks; tok != NULL ; )
		{
			for(ForwardLink *link = tok->_links ; link != NULL ; )
			{
				ForwardLink * tmp_link = link->_next;
				delete link;
				_num_links--;
				link = tmp_link;
			}
			tok->_links = NULL;
			Token *tmp_tok = tok->_down;
			delete tok;
			_num_toks--;
			tok = tmp_tok;
		}
		_active_toks[i]._toks = NULL;
	}
	_active_toks.clear();
	_search_start_tok = NULL;
	_num_word = 0;
	_num_links = 0;
}

/*************
 * cut off token if current token is not final 
 * and it have not next token. From first word start 
 * and must be continue cut 
 *************/
void ParseDecode::PruneActiveToken(int num_word)
{
	int i = 0;
	for(i = num_word ; i >= 0 ; --i)
	{
		PruneForwardLinkCurToCur(i);
		PruneForwardLinkForWord(i-1);
		PruneTokenForWord(i);
	}
}
/****************
 * because current token can director current token ,
 * so must be add some recursion.
 * in same wordid,first execute PruneForwardLinkCurToCur ,
 * after PruneForwardLinkForWord
 ****************/
void ParseDecode::PruneForwardLinkCurToCur(int wordid)
{
	if(wordid < 0)
		return ;
	Token *toks = _active_toks[wordid]._toks;
	for(;toks != NULL ;toks = toks->_down)
	{
		ForwardLink * &links = toks->_links;
		ForwardLink *link, *next_link = NULL, *prev_link = NULL;
		for(link = links ; link != NULL ; link = next_link)
		{
			next_link = link->_next;
			Token *next_tok = link->_next_tok;
			// if link is current node to current nodea,
			// if it's null edge ,then judge.
			// so recursion process,verify whether delete.
			ArcInput iput = link->_ilabel;
			if(iput._word == 0 && false == JudgeUseLink(next_tok))//judge whether delete this link
			{
//				if(false == JudgeUseLink(next_tok))
//				{
					if(prev_link != NULL)
						prev_link->_next = link->_next;
					else
						links = link->_next;
					delete link;
					_num_links--;
					link = NULL;
//				}
			}
			else
			{
				prev_link = link;
			}
		}
	}
}
bool ParseDecode::JudgeUseLink(Token *tok )
{
	if(tok->_final == 1 && tok->_net_bound.size() == 0)
		return true;
	ForwardLink *links = tok->_links;
	ForwardLink *link, *next_link = NULL;
	for(link = links ; link != NULL ; link = next_link)
	{
		next_link = link->_next;
		Token *next_tok = link->_next_tok;
//		if(next_tok->_final == 1)
//			return true;
		if(link->_ilabel._word == 0)//judge whether delete this link
		{
			if(true != JudgeUseLink(next_tok))
			{
				continue;
			}
			else
				return true;
		}
		else if(link->_ilabel._word != 0)
		{
			return true;
		}
	}
	return false;
}
/***********
 * prune next word token before ,must be execute this function.
 *
 ***********/
void ParseDecode::PruneForwardLinkForWord(int wordid)
{
	if(wordid < 0)
		return ;
	Token *toks = _active_toks[wordid]._toks;
	for(;toks != NULL ;toks = toks->_down)
	{
		ForwardLink * &links = toks->_links;

		ForwardLink *link, *next_link = NULL, *prev_link = NULL;
		for(link = links ; link != NULL ; link = next_link)
		{
			next_link = link->_next;
			Token *next_tok = link->_next_tok;
			if( next_tok->_links == NULL && (next_tok->_final == 0 || (next_tok->_final == 1 && next_tok->_net_bound.size() != 0)))//cut this edge
			{
				if(prev_link != NULL)
					prev_link->_next = link->_next;
				else
					links = link->_next;
				delete link;
				_num_links--;
				link = NULL;
			}
			else
			{
				prev_link = link;
			}
		}
	}
}
/***************
 * because "_active_toks _toks" is reverse add, so eps edge arrive
 * node at "_toks" front,so this function can be right work. 
 ***************/
void ParseDecode::PruneTokenForWord(int wordid)
{
	Token *&toks = _active_toks[wordid]._toks;
	Token *tok, *next_tok , *prev_tok = NULL;
	for(tok = toks ; tok != NULL ; tok = next_tok)
	{
		next_tok = tok->_down;
		if( tok->_links == NULL && (tok->_final == 0 ||(tok->_final == 1 && tok->_net_bound.size() != 0)))//cut this edge
		{
			if(prev_tok != NULL)
				prev_tok->_down = tok->_down;
			else
				toks = tok->_down;
			delete tok;
			_num_toks--;
		}
		else
		{
			prev_tok = tok;
		}
	}
}
/*******************
 * send sentence into grammer analysis
 *******************/
unsigned ParseDecode::SendSentence(char *sent)
{
	_wordid_arr.clear();
	_words_arr.clear();
	char word[128];
	char *s = sent;
	int i=0;
	memset(word,0x00,sizeof(word));
	while(*s != '\0')
	{
		if( !isspace((int)(*s)) )
		{
			word[i] = *s;
			i++;
		}
		else
		{
			word[i] = '\0';
			if(i > 0)
			{
				_words_arr.push_back(word);
				int wordid = 0;//no this word in dict
				if(_dict.find(word) != _dict.end())
					wordid = _dict[word];
				_wordid_arr.push_back(wordid);
			}
			i = 0;
			memset(word,0x00,sizeof(word));
		}
		s++;
	}
	word[i] = '\0';
	if(i > 0)
	{
		_words_arr.push_back(word);
		int wordid = 0;//no this word in dict
		if(_dict.find(word) != _dict.end())
		{
			wordid = _dict[word];
			_wordid_arr.push_back(wordid);
		}
	}
	return _wordid_arr.size(); 
}

int ParseDecode::Decoder(char *sent)
{
	if(sent == NULL)
		return SUCC;
	if(SendSentence(sent) == 0)
	{
		LOGERR("All word it's not in dictionary ,sent is null.\n");
		return ERROR;
	}
	DebugInfo();
	Init();
	DebugInfo();
	for(unsigned i=0; i < _wordid_arr.size() ; ++i)
	{
		if(_wordid_arr[i] == 0)
			continue;
		ProcessEmitting(_wordid_arr[i]);
		DebugInfo();
		ProcessNonemitting();
		DebugInfo();
//		PrintResult();
		PruneActiveToken(_num_word-1);
		DebugInfo();
//		PrintResult();
	}
	PruneActiveToken(_num_word);
//	PruneForwardLinkForWord(_num_word);//cut the last word ForwardLink
	DebugInfo();
//	PruneTokenForWord(_num_word);//cut the last word Token

	PrintResult();
	if((unsigned)_num_word != _wordid_arr.size())
		LOGERR("have oov word in send sentence.\n");
	
	GetResStruct();
	SortRes();
	GetOneBest();

	string outline;
	ExchangeResFormat(outline);
//#ifdef DEBUG
	printf("%s",outline.c_str());
//#endif
	char xml_result[4096];
	char *xml_res_p = xml_result;
	memset(xml_result,0x00,sizeof(xml_result));
	convert2xml((char*)sent,(char*)outline.c_str(),xml_res_p,NULL);

	printf("%s\n",xml_result);

	DelResStruct();
//	DebugInfo();
	ClearActiveTokens();
	DebugInfo();
	return SUCC;
}

void ParseDecode::PrintResult()
{
#ifndef DEBUG
	return ;
#endif
	for(unsigned i=0;i<_active_toks.size();++i)
	{
		//traverse cur _active_toks
		Token *tok = _active_toks[i]._toks;
		Token *next_tok = NULL;
		for(;tok != NULL;tok = next_tok)
		{
			next_tok = tok->_down;
			PrintToken(tok,1);
		}
	}
}
int n_sentence = 0;
void ParseDecode::PrintToken(ParseDecode::Token *tok,int start)
{
#ifndef DEBUG
	return ;
#endif
	if(NULL == tok->_links)
		return;
	ForwardLink *link = tok->_links;
	ForwardLink *next_link = NULL;
	for(; link != NULL ; link = next_link)
	{
		next_link = link->_next;
		Token *next_tok = link->_next_tok;
		if(start == 1)
		{
			if(link->_olabel >= 0)
				continue;
		}
	/*	if(link->_ilabel._word == 0 && link->_ilabel._net == 0 &&
				link->_olabel == 0)
			;
		else*/ if(link->_ilabel._word == 0 && link->_ilabel._net != 0)
			;
		else// if(link->_ilabel._word != 0 || link->_olabel != 0)
		{
			printf("%5d@word(%3d):ilabel(%3d,%3d),olabel(%3d),_graph_cost(%5.2f),tot_score(%5.2f)\n",
					n_sentence++,next_tok->_wordid,
					link->_ilabel._word,link->_ilabel._net,
					link->_olabel,link->_graph_cost,next_tok->_path_score);
		}
		fflush(stdout);
		PrintToken(next_tok,0);
		//printf("******************************************\n");
	}
}

void ParseDecode::GetResStruct()
{
	assert((unsigned)(_num_word + 1) == _active_toks.size());
		
	_reslist.resize(_num_word + 1);
	for(unsigned i=0;i<_active_toks.size();++i)
	{
		//traverse cur _active_toks
		Token *tok = _active_toks[i]._toks;
		Token *next_tok = NULL;
		for(;tok != NULL;tok = next_tok)
		{
			next_tok = tok->_down;
			RecordToken(tok,NULL,i,1);
		}
	}
}

void ParseDecode::RecordToken(Token *tok,ResNode *prev,int num_word,int start)
{
	if(NULL == tok->_links)
		return ;


	ForwardLink *link = tok->_links;
	ForwardLink *next_link = NULL;
	for(; link != NULL ; link = next_link)
	{
		ResNode *prev_res = prev;
		int num = num_word;
		next_link = link->_next;
		Token *next_tok = link->_next_tok;
		if(start == 1)
		{
			if(link->_olabel >= 0)
				continue;
		}
		bool end = next_tok->_final == 1 ? true: false;
		float score = next_tok->_path_score;
		//get link message
		int inword = link->_ilabel._word;
		int outnet = link->_olabel;
		float wordscore = link->_graph_cost;


		if(link->_ilabel._word != 0)
		{
			num = num_word+1;
			ResNode *resnode = new ResNode(inword,outnet,wordscore,num,score,end,prev_res);
			_recordlist.push_back(resnode);
			prev_res = resnode;
		}
		else if(next_tok->_final == 1 || (link->_ilabel._word == 0 && link->_olabel != 0))
		{
			ResNode *resnode = new ResNode(inword,outnet,wordscore,num,score,end,prev_res);
			_recordlist.push_back(resnode);
			prev_res = resnode;
		}
		else if(link->_ilabel._word == 0 && link->_olabel == 0)//null arc
		{
			;// do nothing.
		}
//		else if(link->_ilabel._word == 0 && link->_olabel != 0)
//		{
//			ResNode *resnode = new ResNode(inword,outnet,wordscore,num,score,end,prev_res);
//			_recordlist.push_back(resnode);
//			prev_res = resnode;
//		}
		if(next_tok->IsEndToken())
		{
			assert((unsigned)num < _reslist.size());
			assert(NULL == next_tok->_links);
			_reslist[num].push_back(prev_res);
			_allreslist.push_back(prev_res);
			continue;
		}
		RecordToken(next_tok,prev_res,num,0);
	}
}

void ParseDecode::SortRes()
{
	unsigned i = 0, j = 0;
	for(i=0;i<_allreslist.size();++i)
	{
		for(j=i+1;j<_allreslist.size();++j)
		{
			if(!Comp(_allreslist[i],_allreslist[j]))
				Exchange(_allreslist[i],_allreslist[j]);
		}
	}
}

void ParseDecode::GetOneBest()
{
	if(_allreslist.size() == 0)
		return ;
	_onebest.clear();
	ResNode *node = _allreslist[0];
	ResNode *prev_node = NULL;
	for(node = _allreslist[0];node != NULL ; node = prev_node)
	{
		_onebest.push_back(node);
		prev_node = node->_prev_node;
#ifdef DEBUG
		printf("@word(%3d):ilabel(%3d),olabel(%3d),is_end(%5s),_graph_cost(%5.2f),tot_score(%5.2f)\n",
				node->_nword,node->_inword,node->_outnet,
				node->_is_end ? "true":"false",
				node->_wordscore,node->_score);
#endif
	}
#ifdef DEBUG
	for(node = _allreslist[0];node != NULL ; node = prev_node)
	{
		prev_node = node->_prev_node;
		const char *out = NULL;
		if(node->_outnet < 0)
			out = _frames[-1 * node->_outnet].c_str();
		else
			out = _net_index[node->_outnet].c_str();
		printf("@word(%3d):ilabel(%5s),olabel(%5s),is_end(%5s),_graph_cost(%5.2f),tot_score(%5.2f)\n",
				node->_nword,_dict_index[node->_inword].c_str(),out,
				node->_is_end ? "true":"false",
				node->_wordscore,node->_score);
	}
#endif
}
void ParseDecode::ExchangeResFormat(string &res)
{
	vector<string> stack_action;
	int __flag = 0;
	string prev_line,cur_line;
	while(!_onebest.empty())
	{
		cur_line.clear();
		ResNode* node = _onebest.back();
		_onebest.pop_back();
		if(node->_outnet < 0)
		{
			stack_action.push_back(_frames[-1 * node->_outnet]);
		}
		else if(node->_outnet > 0)
		{
			stack_action.push_back(_net_index[node->_outnet]);
			const char *tmp = stack_action.back().c_str();
			if(tmp[0] == '[' && tmp[1] == '_')
				__flag ++;
		}

		if(node->_inword != 0)
		{
			string net = stack_action.back();
			const char *cnet = net.c_str();
			string &outline = cur_line;
			int n = 0;
//			if(cnet[0] == '[' && (isupper((int)cnet[1]) || islower((int)cnet[1]) || __flag == 2))
			{//output this word
				for(unsigned i = 0;i<stack_action.size();++i)
				{
					if(i == 0)
						outline = stack_action[i] + ":";
					else
					{
						cnet = stack_action[i].c_str();
						if(cnet[0] == '[' && cnet[1] == '_')
						{
							char option[128];
							memset(option,0x00,sizeof(option));
							const char *s = strchr(cnet,'_');
							const char *e = strchr(cnet,']');
							memcpy(option,s+1,e-s-1);
							if(n == 0)
							{
								outline += string(option) + " ";
								n++;
							}
							else if(n == 1)
							{
								outline += string(option);
								n++;
							}

						}
						else if(cnet[0] == '[' && isupper((int)cnet[1]))
						{
							outline += string(cnet) + ".";
						}
					}
				}//end for
				if(n < 2)
					outline += _dict_index[node->_inword];
			}
			if(prev_line != cur_line || __flag == 1)
			{
				if(strchr(cur_line.c_str(),'.') != NULL)
				{
					res += cur_line + '\n';
					prev_line = cur_line;
				}
			}
		}//end if
		if(node->_is_end)
		{
			const char *tmp = stack_action.back().c_str();
			if(tmp[0] == '[' && tmp[1] == '_')
				__flag --;
			stack_action.pop_back();
		}
	}//end while
}
