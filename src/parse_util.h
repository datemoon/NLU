#ifndef __PARSE_UTIL_H__
#define __PARSE_UTIL_H__

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

using namespace std;
using std::unordered_map;

class Sentence
{
public:
	Sentence(){}
	
	~Sentence(){}

	unsigned SendSentence(const char *sent,unordered_map<string,int> &dict)
	{
		if(sent == NULL)
			return 0;
		_wordid_arr.clear();
		_words_arr.clear();
		char word[128];
		const char *s = sent;
		int i=0;
		memset(word,0x00,sizeof(word));
		while(*s != '\0')
		{
			if( !isspace((int)(*s)))
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
					int wordid = -1;//no this word in dict
					if(dict.find(word) != dict.end())
						wordid = dict[word];
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
			int wordid = -1;//no this word in dict
			if(dict.find(word) != dict.end())
			{
				wordid = dict[word];
			}
			_wordid_arr.push_back(wordid);
		}
		return _wordid_arr.size();
	}
protected:
	vector<string> _words_arr;
 	vector<int> _wordid_arr;
};


#endif
