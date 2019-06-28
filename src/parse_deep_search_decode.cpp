

#include "parse_deep_search_decode.h"
#include "convert.h"

int ParseDeepDecode::FindWordArc(ParseNode *node,int wordid)
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

int ParseDeepDecode::ExpendPath(int tokid)
{
	int ret = 0;
	int ret_tmp = 0;
	// if use reference , tok can be change when _stack_toks realloc memory,
	// so I use copy , or every use &tok again assignment.
	Token tok = _stack_toks[tokid];
	int nodeid = tok._nodeid;
	float cur_cost = tok._path_score;
//	if(nodeid == 763)
//		printf("stop\n");
	ParseNode *node = _graph->_search_net.GetNode(nodeid);
	// if node is final ,record tok 
	if(node->IsFinal())
	{

		if(_stack_netjump.size() != 0)
		{
			NetBound jumpnet = _stack_netjump.back();
			int tonodeid = jumpnet._tonode;
			float scale = jumpnet._scale;
			_tot_scale = _tot_scale/scale;
			_stack_netjump.pop_back();

			float tot_cost = cur_cost;
			int next_tokid = AddToken(tonodeid, tot_cost, tokid, 0 , 0 , true);
			
			ret_tmp = ExpendPath(next_tokid);
			
			_stack_netjump.push_back(jumpnet);
			_tot_scale = _tot_scale*scale;
			if(0 == ret_tmp)
			{
				DeleteToken();
				return 0;
			}
			else
			{
				return 1;
			}
		}
		int next_tokid = AddToken(-1, tok._path_score, tokid , 0, 0,true);
		if(tok._path_score > _best_score)
		{
			_best_score = tok._path_score;
			_best_tok = next_tokid;
		}
		_res_tok_list.push_back(next_tokid);
		return 1; // this tok should be save.
	}
	int wordid = -1,s = -1;
	if((unsigned)_num_word < _wordid_arr.size())
	{
		wordid = _wordid_arr[_num_word];
		s = FindWordArc(node,wordid);
	}
	else
	{
		;// it's end word.
	}
	// search this node and the arcs is sort, so I can break
	// when not find word
	if(s >= 0)
	{
		for(; s < static_cast<int>(node->GetArcSize()); ++s)
		{
			ParseArc *arc = node->GetArc((unsigned)s);
			if(arc->_input._word == wordid)// if searchword at arc.
			{
				float arc_weight = arc->_w * _tot_scale;
				float tot_cost = cur_cost + arc_weight;
				int onet = arc->_output;
				int next_tokid = AddToken(arc->_to, tot_cost, tokid, wordid , onet, false);
				_num_word++;
				ret_tmp = ExpendPath(next_tokid);
				_num_word--;
				if(0 == ret_tmp)
				{
					DeleteToken();
				}
				else
				{
					ret = 1;
				}
			}
			else
				break;
		}
	}
	// no find word
	// expand eps arc
	int i = 0;
	for(i=0; i < static_cast<int>(node->GetArcSize()); ++i)
	{
		ParseArc *arc = node->GetArc((unsigned)i);

		int next_tokid = -1;
		if(arc->_input._word == 0)
		{
			float tot_cost = cur_cost;
			
			if(arc->_input._net != 0)
			{
				float scale = arc->_w;
				int tonode = arc->_to;

				NetBound netbound(tonode,scale);
				_stack_netjump.push_back(netbound);

				_tot_scale *= scale;
				int netnodeid = _graph->GetNetNodeId(arc->_input._net);
				next_tokid = AddToken(netnodeid, tot_cost, tokid , 0 , 0 ,false);
				ret_tmp = ExpendPath(next_tokid);

				_tot_scale /= scale;
				_stack_netjump.pop_back();
			}
			else if(arc->_input._net == 0)
			{
				int tonode = arc->_to;
				int onet = arc->_output;
				next_tokid = AddToken(tonode, tot_cost, tokid, 0, onet, false);
				ret_tmp = ExpendPath(next_tokid);
			}
			if(0 == ret_tmp)
			{
				DeleteToken();
			}
			else
			{
				ret = 1;
			}
		}
		else
			break;
	}
	return ret;
}

/*
int ParseDeepDecode::ProcessNonEmitting(int tokid)
{
	int ret = 0;
	// if use reference , tok can be change when _stack_toks realloc memory,
	// so I use copy , or every use &tok again assignment.
	Token tok = _stack_toks[tokid];
	int nodeid = tok._nodeid;
	float cur_cost = tok._path_score;
	ParseNode *node = _search_net.GetNode(nodeid);
	// if node is final ,record tok 
	if(node->IsFinal())
	{

		if(_stack_netjump.size() != 0)
		{
			NetBound jumpnet = _stack_netjump.back();
			int tonodeid = jumpnet._tonode;
			float scale = jumpnet._scale;
			_tot_scale = _tot_scale/scale;
			_stack_netjump.pop_back();

			float tot_cost = cur_cost;
			int next_tokid = AddToken(tonodeid, tot_cost, tokid, 0 , 0 , true);
			int ret1 = 0,ret2 = 0;
			ret1 = ProcessNonEmitting(next_tokid);
//			_num_word++;
			ret2 = ProcessEmitting(next_tokid);
//			_num_word--;
			_stack_netjump.push_back(jumpnet);
			_tot_scale = _tot_scale*scale;
			if(0 == ret1 && 0 == ret2)
			{
				DeleteToken();
				return 0;
			}
			else
			{
				return 1;
			}
		}
		int next_tokid = AddToken(-1, tok._path_score, tokid , 0, 0,true);
		if(tok._path_score > _best_score)
		{
			_best_score = tok._path_score;
			_best_tok = next_tokid;
		}
		_res_tok_list.push_back(next_tokid);
		return 1; // this tok should be save.
	}

	int i = 0;
	for(i=0; i < static_cast<int>(node->GetArcSize()); ++i)
	{
		ParseArc *arc = node->GetArc((unsigned)i);

		int next_tokid = -1;
		if(arc->_input._word == 0)
		{
			float tot_cost = cur_cost;
			
			int ret1 = 0, ret2 = 0;
			if(arc->_input._net != 0)
			{
				float scale = arc->_w;
				int tonode = arc->_to;

				NetBound netbound(tonode,scale);
				_stack_netjump.push_back(netbound);

				_tot_scale *= scale;
				int netnodeid = GetNetNodeId(arc->_input._net);
				next_tokid = AddToken(netnodeid, tot_cost, tokid , 0 , 0 ,false);
				ret1 = ProcessNonEmitting(next_tokid);
				ret2 = ProcessEmitting(next_tokid);

				_tot_scale /= scale;
				_stack_netjump.pop_back();
			}
			else if(arc->_input._net == 0)
			{
				int tonode = arc->_to;
				int onet = arc->_output;
				next_tokid = AddToken(tonode, tot_cost, tokid, 0, onet, false);
				ret1 = ProcessNonEmitting(next_tokid);
				ret2 = ProcessEmitting(next_tokid);
			}
			if(0 == ret1 && 0 == ret2)
			{
				DeleteToken();
			}
			else
			{
				ret = 1;
			}
		}
		else
			break;
	}
	return ret;
}

int ParseDeepDecode::ProcessEmitting(int tokid)
{
	int ret = 0;

	Token tok = _stack_toks[tokid];

	int nodeid = tok._nodeid;
	float cur_cost = tok._path_score;
	
	ParseNode *node = _search_net.GetNode(nodeid);
	// if node is final ,record 
	if(node->IsFinal())
	{
		if(_stack_netjump.size() != 0)
		{
			NetBound jumpnet = _stack_netjump.back();
			int tonodeid = jumpnet._tonode;
			float scale = jumpnet._scale;
			_tot_scale = _tot_scale/scale;
			_stack_netjump.pop_back();

			float tot_cost = cur_cost;
			int next_tokid = AddToken(tonodeid, tot_cost, tokid, 0, 0,true);
			int ret1 = 0,ret2 = 0;
			ret1 = ProcessNonEmitting(next_tokid);
			ret2 = ProcessEmitting(next_tokid);
			_stack_netjump.push_back(jumpnet);
			if(0 == ret1 && 0 == ret2)
			{
				DeleteToken();
				return 0;
			}
			else
			{
				return 1;
			}
		}
		int next_tokid = AddToken(-1, cur_cost, tokid, 0 , 0 ,true);
		if(cur_cost > _best_score)
		{
			_best_score = cur_cost;
			_best_tok = next_tokid;
		}
		_res_tok_list.push_back(next_tokid);
		return 1; // this tok should be save. it's true tail.
	}
	// if _num_word > _wordid_arr.size()-1
	if((unsigned)_num_word > _wordid_arr.size() - 1)
		return 0;
	int wordid = _wordid_arr[_num_word];
	
	int i = FindWordArc(node,wordid);
	if(i < 0)
		return 0; // no find word

	for(; i < static_cast<int>(node->GetArcSize()); ++i)
	{
		ParseArc *arc = node->GetArc((unsigned)i);
		if(arc->_input._word == wordid)// if searchword at arc.
		{
			float arc_weight = arc->_w * _tot_scale;
			float tot_cost = cur_cost + arc_weight;
			int onet = arc->_output;
			int next_tokid = AddToken(arc->_to, tot_cost, tokid, wordid , onet, false);
			int ret1 = 0,ret2 = 0;
			_num_word++;
			ret1 = ProcessNonEmitting(next_tokid);
			ret2 = ProcessEmitting(next_tokid);
			_num_word--;
			if(0 == ret1 && 0 == ret2)
			{
				DeleteToken();
			}
			else
			{
				ret = 1;
			}
		}
		else
			break;
	}
	return ret;
}
*/
int ParseDeepDecode::AddToken(int nodeid, 
		float path_score, int prev, int wordid, int onet , bool final)
{
	int size = _stack_toks.size();
	Token tok ;
	tok._nodeid = nodeid;
	tok._path_score = path_score;
	tok._wordid = wordid;
	tok._onet = onet;
	tok._final = final;
	tok._prev = prev;
	_stack_toks.push_back(tok);
	/*
	int size = _stack_toks.size();
	_stack_toks.resize(size+1);
	_stack_toks[size]._nodeid = nodeid;
	_stack_toks[size]._path_score = path_score;
	_stack_toks[size]._wordid = wordid;
	_stack_toks[size]._onet = onet;
	_stack_toks[size]._final = final;
	_stack_toks[size]._prev = prev;
	*/
	return size;
}

void ParseDeepDecode::DeleteToken()
{
	_stack_toks.pop_back();
}

void ParseDeepDecode::Reset()
{
	_stack_netjump.clear();
	_stack_toks.clear();

	_num_word = 0;
	_tot_scale = 1.0;
//	_words_arr.clear();
//	_wordid_arr.clear();

	_res_tok_list.clear();
	_best_score = 0.0;
	_best_tok = -1;
}

int ParseDeepDecode::Init(BaseGraph *graph)
{
	if(graph != NULL)
		_graph = graph;
	if(_graph == NULL)
	{
		LOGERR("graph is NULL,it's error.\n");
		return ERROR;
	}
	int start = _graph->_net_startid;
	return AddToken(start, 0, -1, 0, 0, false);
}

void ParseDeepDecode::GetOneBest()
{
	if(_best_tok == -1)
		return ;
	for(int tokid = _best_tok ; _stack_toks[tokid]._prev >= 0;
			tokid = _stack_toks[tokid]._prev)
	{
		if(_stack_toks[tokid]._wordid != 0 || 
				_stack_toks[tokid]._onet != 0 ||
				_stack_toks[tokid]._final != 0)
			_onebest.push_back(tokid);
	}
}

void ParseDeepDecode::PrintOneBest()
{
	unsigned i = 0;
	for(i=0;i<_onebest.size();++i)
	{
		Token &tok = _stack_toks[_onebest[i]];
		tok.PrintTok();
	}
	for(i=0;i<_onebest.size();++i)
	{
		Token &tok = _stack_toks[_onebest[i]];
		PrintToken(tok);
	}
}
void ParseDeepDecode::PrintToken(Token &tok)
{
	const char *out = NULL;
	if(tok._onet < 0)
		out = _graph->_frames[-1 * tok._onet].c_str();
	else
		out = _graph->_net_index[tok._onet].c_str();
	printf("%3d %5.2f %10s %10s %d\n",
			tok._nodeid,tok._path_score,_graph->_dict_index[tok._wordid].c_str(),
			out,tok._final==true?1:0);
}


void ParseDeepDecode::ExchangeResFormatXML(string &res)
{
	string prefix = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
	// add slots
	res += static_cast<string>(prefix);
//	res += static_cast<string>("<slots>\n\t\t\t");

	string slots;
	string focus;
	string operation;
	int big_s = 0;
	int small_s = 0;
	int tabnums = 2;
	int slotdeep = 0;
	vector<string> slot_stack;
	bool slot_in = false;
	while(!_onebest.empty())
	{
		int tokid = _onebest.back();
		_onebest.pop_back();
		Token &tok = _stack_toks[tokid];
		if(tok._onet < 0)
		{
			if(!focus.empty())
			{
				LOGERR("it's error format for focus.\n");
				continue;
			}

			focus = _graph->_frames[-1 * tok._onet];
		}
		else if(tok._onet > 0)
		{
			const char *tmp = _graph->_net_index[tok._onet].c_str();
			const char *cnet = tmp;
			if(tmp[0] == '[' && tmp[1] == '_')
			{// this operation
				if(!operation.empty())
				{
					LOGERR("it's error format for operation.\n");
					continue;
				}
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				const char *e = strchr(cnet,'@');
				memcpy(option,s+1,e-s-1);

				operation = string(option);
			}
			else if(tmp[0] == '[' && isupper((int)tmp[1]))
			{// slot
				big_s ++;
				slotdeep ++;
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				const char *e = strchr(cnet,']');
				memcpy(option,s+1,e-s-1);

				slot_stack.push_back(string(option));
				if(slot_in == true)
					slots += string("\n");
				for(int i=0;i<tabnums+slotdeep;++i)
				{
					slots += string("\t");
				}
				slots += "<"+string(option)+">";
				slot_in = true;
			}
			else if(tmp[0] == '[' && islower((int)tmp[1]) && strchr(tmp,'_') == NULL)
			{
				small_s ++;
			}
		}
		else if(tok._wordid != 0)
		{
			if(big_s == 0 || slot_in == false)
				continue;
			slots += _graph->_dict_index[tok._wordid];
		}
		if(tok._final)
		{
			if(small_s > 0)
				small_s--;
			else if(big_s > 0)
			{
				big_s--;
				if(false == slot_in)
				{
					///slots += "\n";
					for(int i=0;i<tabnums+slotdeep;++i)
					{
						slots += string("\t");
					}
				}
				slots += "</"+slot_stack.back()+">";
				slots += "\n";
				slot_stack.pop_back();
				slot_in = false;
				slotdeep--;
			}
		}
	}
//	std::cout << slots << std::endl;
	res = prefix + "\t<hubo>\n" + "\t\t<slots>\n" + slots + "\t\t</slots>\n";
	string rawtext ;
	for(unsigned i=0;i<_words_arr.size();++i)
	{
		rawtext += _words_arr[i];
	}
	res += "\t\t<rawtext>" + rawtext + "</rawtext>\n";
	res += "\t\t<focus>" + focus + "</focus>\n";
	res += "\t\t<operation>" + operation + "</operation>\n";
	res += "\t\t<message>ok</message>\n";
	res += "\t\t<vesion>0.1</vesion>\n";
	res += "\t\t<status>0</status>\n";
	res += "\t</hubo>\n";

	std::cout << res << std::endl;
	return ;
}

void ParseDeepDecode::ExchangeResFormat(string &res)
{
	res.clear();
	vector<string> stack_action;
//	int __flag = 0;
	string prev_line,cur_line;
	unsigned head3 = 0;
	for(head3 = 0;!_onebest.empty() && head3 < 3;++head3)
	{
		int tokid = _onebest.back();
		_onebest.pop_back();
		Token &tok = _stack_toks[tokid];
		if(tok._onet < 0)
			stack_action.push_back(_graph->_frames[-1 * tok._onet]);
		else if(tok._onet > 0)
		{
			stack_action.push_back(_graph->_net_index[tok._onet]);
		}
	}
	if(stack_action.size() != head3)
		LOGERR("it's error format.\n");
	for(unsigned i = 0;i<stack_action.size();++i)
	{
		string &outline = cur_line;
		if(i == 0)
			outline = stack_action[i] + ":";
		else 
		{//i == 2
			const char *cnet = stack_action[i].c_str();
			if(cnet[0] == '[' && cnet[1] == '_')
			{
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				const char *e = strchr(cnet,']');
				memcpy(option,s+1,e-s-1);
				outline += string(option) + " ";
			}
		}
	}//end for
	res += cur_line;
	stack_action.clear();
	// the first three
	
//	return ;
	int big_s = 0;
	int small_s = 0;
	while(!_onebest.empty())
	{
		cur_line.clear();
		string &outline = cur_line;
		int tokid = _onebest.back();
		_onebest.pop_back();
		Token &tok = _stack_toks[tokid];
		if(tok._onet > 0)
		{
			const char *tmp = _graph->_net_index[tok._onet].c_str();
			if(tmp[0] == '[' && isupper((int)tmp[1]))
			{
				stack_action.push_back(_graph->_net_index[tok._onet]);
				//res += _graph->_net_index[tok._onet] + ".";
				big_s++;
			}
			else if(tmp[0] == '[' && tmp[1] == '_')
			{
				if(big_s != 1)
				{
					LOGERR("it's error format.\n");
					continue;
				}
				big_s ++;
				outline = stack_action.back() + ".";
				const char *cnet = tmp;
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				const char *e = strchr(cnet,']');
				memcpy(option,s+1,e-s-1);
				outline += string(option) + "\n";
				res += outline;
				while(!_onebest.empty())
				{
					int wordtokid = _onebest.back();
					_onebest.pop_back();
					Token &wordtok = _stack_toks[wordtokid];
					if(wordtok._onet > 0)
						small_s++;
					if(wordtok._final)
					{
						if(small_s > 0)
							small_s--;
						else
							big_s--;
					}
					if(big_s == 0)
						break;
				}
				stack_action.pop_back();
			}
			else
				small_s++;
		}
		else if(tok._wordid != 0)
		{
			if(big_s != 1)
				continue;
			outline = stack_action.back() + ".";
			outline += _graph->_dict_index[tok._wordid] + "\n";
			res += outline;
		}
		if(tok._final)
		{
			if(small_s > 0)
				small_s--;
			else
				big_s--;
		}
	}
	return ;
}

#define NONE_MATCH(sent) \
	char nospace[1024] ;memset(nospace,0x00,sizeof(nospace));\
	int i = 0,j=0;while(sent[i] != '\0'){ \
		if(!isspace(sent[i])){nospace[j++] = sent[i]; }i++;	\
	}nospace[j] = '\0'; \
	const char *prefix = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<hubo>\n\t\t<rawtext>"; \
	const char * suffix = "</rawtext>\n\t\t<message> none match </message>\n\t\t<vesion> 0.1 </vesion>\n\t\t<status> 1 </status>\n\t\t<focus> OTHER </focus>\n\t</hubo>"; \
	strcat(result,prefix); \
	strcat(result,nospace); \
	strcat(result,suffix);

float ParseDeepDecode::Decoder(const char *sent, char *const result)
{
	unsigned word_num = SendSentence(sent,_graph->_dict);
	if( 0 == word_num)
	{
		NONE_MATCH(sent);
		return 0.0;
	}
	// clear no find word;
	Reset();
	int start_tokid = Init();
	for(unsigned i=0; i < _wordid_arr.size() ; ++i)
	{
		if(_wordid_arr[i] == 0)
			continue;
		ExpendPath(start_tokid);
		_num_word++;
	}

	GetOneBest();
	if(_best_tok == -1)
	{
		NONE_MATCH(sent);
		return 0.0;
	}
#ifdef DEBUG
	PrintOneBest();
#endif
	string outline;
	ExchangeResFormatXMLOld(outline);
//	ExchangeResFormat(outline);
#ifdef DEBUG
	printf("%s",outline.c_str());
#endif

	memcpy(result,outline.c_str(),sizeof(char)*outline.size());
	result[outline.size()] = '\0';
//	convert2xml((char*)sent,(char*)outline.c_str(),(char*)xml_res_p,NULL);
#ifdef DEBUG
	printf("%s\n",result);
#endif
	float score = _best_score/_wordid_arr.size();
	return score > 1 ? 1 : score;
}
void ParseDeepDecode::Decoder(const char *sent)
{
	unsigned word_num = SendSentence(sent,_graph->_dict);
	if( 0 == word_num)
		return ;
	Reset();
	int start_tokid = Init();
	//int start = _net_startid;
	for(unsigned i=0; i < _wordid_arr.size() ; ++i)
	{
		if(_wordid_arr[i] == 0)
			continue;
		ExpendPath(start_tokid);
		_num_word++;
	}
	GetOneBest();
	PrintOneBest();

	string outline;
	ExchangeResFormat(outline);

	printf("%s",outline.c_str());

	char xml_result[4096];
	char *xml_res_p = xml_result;
	memset(xml_result,0x00,sizeof(xml_result));
	convert2xml((char*)sent,(char*)outline.c_str(),xml_res_p,NULL);
	printf("%s\n",xml_result);

}

void ParseDeepDecode::ExchangeResFormatXMLOld(string &res)
{
	string prefix = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
	// add slots
	res += static_cast<string>(prefix);
//	res += static_cast<string>("<slots>\n\t\t\t");

	string slots;
	string focus;
	string operation;
	int big_s = 0;
	int small_s = 0;
	int _s = 0;
	int tabnums = 2;
	vector<string> slot_stack;
	vector<int> stack_type; // 0 is [_*@*], 1 is [A*_*] ,2 is [a*]
	bool slot_in = false;
	while(!_onebest.empty())
	{
		int tokid = _onebest.back();
		_onebest.pop_back();
		Token &tok = _stack_toks[tokid];
		if(tok._onet < 0)
		{
			if(!focus.empty())
			{
				LOGERR("it's error format for focus.\n");
				continue;
			}

			focus = _graph->_frames[-1 * tok._onet];
		}
		else if(tok._onet > 0)
		{
			const char *tmp = _graph->_net_index[tok._onet].c_str();
			const char *cnet = tmp;
			if(tmp[0] == '[' && tmp[1] == '_')
			{// this operation
				_s ++;
				stack_type.push_back(0);
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				const char *e = strchr(cnet,'@');
				memcpy(option,s+1,e-s-1);

				if(_s != 1)
				{
					slots += string(option);
				}
				else
				{
					operation = string(option);
				}
			}
			else if(tmp[0] == '[' && isupper((int)tmp[1]))
			{// slot
				stack_type.push_back(1);
				big_s ++;
				char option[128];
				memset(option,0x00,sizeof(option));
				const char *s = strchr(cnet,'_');
				if(s == NULL)
					s = cnet;
				const char *e = strchr(cnet,']');
				memcpy(option,s+1,e-s-1);

				slot_stack.push_back(string(option));
				if(slot_in == true)
					slots += string("\n");
				for(int i=0;i<tabnums+big_s;++i)
				{
					slots += string("\t");
				}
				slots += "<"+string(option)+">";
				slot_in = true;
			}
			else if(tmp[0] == '[' && islower((int)tmp[1]))// && strchr(tmp,'_') == NULL)
			{
				stack_type.push_back(2);
				small_s ++;
			}
		}
		else if(tok._wordid != 0)
		{
			if(big_s == 0 || slot_in == false || _s > 1)
				continue;
			slots += _graph->_dict_index[tok._wordid];
		}
		if(tok._final)
		{
			int ltype = stack_type.back();
			stack_type.pop_back();

			if(ltype == 0 )
				_s--;
			else if(ltype == 2)
				small_s--;
			else if(ltype == 1)
			{
				if(false == slot_in)
				{
					///slots += "\n";
					for(int i=0;i<tabnums+big_s;++i)
					{
						slots += string("\t");
					}
				}
				slots += "</"+slot_stack.back()+">";
				slots += "\n";
				slot_stack.pop_back();
				slot_in = false;
				big_s--;
			}
		}
	}
//	std::cout << slots << std::endl;

	if(slots.empty())
		res = prefix + "\t<hubo>\n";
	else
		res = prefix + "\t<hubo>\n" + "\t\t<slots>\n" + slots + "\t\t</slots>\n";
	string rawtext ;
	for(unsigned i=0;i<_words_arr.size();++i)
	{
		rawtext += _words_arr[i];
	}
	res += "\t\t<rawtext>" + rawtext + "</rawtext>\n";
	res += "\t\t<focus>" + focus + "</focus>\n";
	res += "\t\t<operation>" + operation + "</operation>\n";
	res += "\t\t<message>ok</message>\n";
	res += "\t\t<vesion>0.1</vesion>\n";
	res += "\t\t<status>0</status>\n";
	res += "\t</hubo>\n";

//	std::cout << res << std::endl;
	return ;
}

