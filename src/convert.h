#ifndef _CONVERT2JSON_
#define _CONVERT2JSON_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
//#include <tr1/unordered_map>
#include <unordered_map>
using namespace std; 
//using namespace __gnu_cxx;
//using namespace std::tr1;


void convert(char* buff,char* result);

//text: sentence to be parserd
//sen : parserd result of Phoenix
//json_buff: output json format parser result
int convert2json(char *text, char* sen,char *json_buff, char *history=NULL);
//text: sentence to be parserd
//sen : parserd result of Phoenix
//xml_buff: output json format parser result
int convert2xml(char *text,char* sen,char *xml_buff,char *history=NULL);

void parse_failed_json(char* result,char* text);

void parse_failed_xml(char* result,char* text);

size_t utf8_to_chars(const std::string &input, std::vector<string> &output);

#endif
