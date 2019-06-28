/*zR
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年07月21日 16时55分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#define MODEL_PATH_NULL           -5001
#define MODEL_PATH_TOO_LONG       -5002
#define MALLOC_FAILED             -5003
#define GRAFILE_OPEN_FAILED       -5004
#define FORMSFILE_OPEN_FAILED     -5005
#define PERSONS_OPEN_FAILED       -5006
#define SONGS_OPEN_FAILED         -5007
#define ARTISTS_OPEN_FAILED       -5008
#define APPS_OPEN_FAILED          -5009
#define WORDS_OPEN_FAILED         -5010
#define GRA_FORMAT_ERROR          -5011
#define DICT_CREATE_FAILED        -5012
#define MEMORY_OVERFLOW_ERROR        -5013
#define NO_PARSE_RESULT        -5014
#define CONVERT_JSON_FAILED        -5015

#include "convert.h"
void parseAddress(char* address, char* province, char* city, char* area)
{
	char* p = address;
	char* q = strstr(p,"省");

	if(q){
		strncpy(province,p,q-p);
		province[q-p] = '\0';
		
		char* pos = strstr(p,"市");
		if(pos && (pos != (q+3)) ){
			strncpy(city,q+3,pos-q-3);
			city[pos-p-2] = '\0';
			strcpy(area,pos+3);
		}else{
			strcpy(city,"");
			if( *(q+3) != '\0' )
				strcpy(area,q+3);
			else
				strcpy(area,"");
		}

	}else{
		strcpy(province,"");
		char* pos = strstr(p,"市");
		if( pos && pos != address ){
			strncpy(city,p,pos-p);
			city[pos-p] = '\0';
		
			if( *(pos+3) != '\0' )	
				strcpy(area,pos+3);
			else
				strcpy(area,"");
		}else{
			strcpy(city,"");
			strcpy(area,address);
		}
	}
}

void convert(char* buff,char* result)
{
	//char *sen = "Schedule:[Sche_datetime].[Year].2 [Year].0 [Year].1 [Year].5 [Month].5 [Day].1 [Day].5 [Sche_TimeOrig].6 [Sche_TimeOrig].点 [Sche_TimeOrig].钟";

	if( buff[0] != '[' ){
		buff = strchr(buff,'[');
	}

	if( NULL == buff || NULL == result ){
		return;
	}


	strcpy(result,"");

	char *sen = buff;
	char *b = NULL;
	char *e = NULL;
	char *p = NULL;
	char *q = NULL;

	b = sen;
	e = sen;

	char key[1024] = "";
	char preKey[1024] = "";

	char value[1024] = "";
	char tmp[1024] = "";
	char service[1024] = "";
	char *pos = result;

	int cnt=0;

	while(*b){

		if( *b == ':' ){
			strncpy(service,e,b-e);
			service[b-e] = '\0';
			//printf("service: %s\n",service);

			e = b + 1;
			b++;
			continue;
		}

		if( *b == '[' ){
			if( b > e + 2 ){
				strncpy(value,e+2,b-e-2);
				value[b-e-2] = '\0';
				char* p = value;
				char* q = tmp;

				while(*p){
					if(*p != ' ')
						*q++ = *p;
					p++;
				}
				*q = '\0';

				strcpy(value,tmp);

				//printf("preKey %s key %s\n",preKey,key);
				if(  strcmp(preKey,key) ){//preKey != key
					if(preKey[0] != '\0'){
						//printf("\",\"%s\":\"%s",key,value);
						pos = result + strlen(result);
						sprintf(pos,"\",\"%s\":\"%s",key,value);
					}else{
						//printf("\"%s\":\"%s",key,value);
						pos = result + strlen(result);
						sprintf(pos,"\"%s\":\"%s",key,value);
					}

					strcpy(preKey,key);
				}else{
					//printf("%s",value);
					pos = result + strlen(result);
					sprintf(pos,"%s",value);
				}
			}else if (b == e + 2){
				//printf("%s:{",key);
				pos = result + strlen(result);
				sprintf(pos,"\"%s\":{",key);
				cnt++;
			}

			e = strchr(b,']');
			if(e){
				char oldKey[32];
				strncpy(oldKey,b+1,e-b-1);
				oldKey[e-b-1] = '\0';

				char *p = strchr(oldKey,'_');
				if(p){
					strcpy(key,p+1);
				}else{
					strcpy(key,oldKey);
				}
			}
			else{
				printf("Error: [ and ] not matched!");
				exit(-1);
			}

			b = e + 1;
			continue;
		}

		b++;

	}
	
	strcpy(value,e+2);

	p = value;
	q = tmp;
	while(*p){
		if(*p != ' ')
			*q++ = *p;
		p++;
	}
	*q = '\0';
	strcpy(value,tmp);

	if(  strcmp(preKey,key) ){//preKey != key
		if(preKey[0] != '\0' ){
			//printf("\",\"%s\":\"%s\"",key,value);
			pos = result + strlen(result);
			sprintf(pos,"\",\"%s\":\"%s\"",key,value);
		}else{
			//printf("\"%s\":\"%s\"",key,value);
			pos = result + strlen(result);
			sprintf(pos,"\"%s\":\"%s\"",key,value);
		}
	}else{
		//printf("%s\"",value);
		pos = result + strlen(result);
		sprintf(pos,"%s\"",value);
	}

	while(cnt--){
		//printf("}");
		pos = result + strlen(result);
		sprintf(pos,"}");
	}

	//printf("\n");
	//printf("%s\n",result);

}

int convert2json(char *text, char* sen,char *json_buff, char *history)
{
	if( NULL == sen || NULL == json_buff){
		return NO_PARSE_RESULT;
	}


	// sen = "Schedule:create [Sche_name].clock \nSchedule:[Sche_datetime].[Sche_TimeOrig].上 [Sche_TimeOrig].午 [Sche_TimeOrig].八 [Sche_TimeOrig].点 \nSchedule:[Sche_content].的 \nSchedule:[Sche_content].闹 \nSchedule:[Sche_content].铃";
//	char *sen = "Schedule:create [Sche_name].clock\nSchedule:[Sche_datetime].[Year].2 [Year].0 [Year].1 [Year].5 [Month].5 [Day].1 [Day].5 [Sche_TimeOrig].6 [Sche_TimeOrig].点 [Sche_TimeOrig].钟\nSchedule:[Sche_content].的 \nSchedule:[Sche_content].闹 \nSchedule:[Sche_content].钟";

	//printf("intput\n%s\n",sen);
	
	//rm space of text
	{
		char newText[2048] = "";
		char *pText = text, *pNewText = newText;
		while(*pText){
			if( *pText == ' '){
				pText++;
				continue;
			}
			*pNewText++ = *pText++;
		}
		*pNewText = '\0';
		strcpy(text,newText);
	}
	

	//printf("%s\n",text);

	strcpy(json_buff,"");
	char* pos = json_buff;

	char result[2048] = "";

	char service[32];
	char operation[32];

	//get service
	char* p = strchr(sen,':');
	if(p){
		strncpy(service,sen,p-sen);
		service[p-sen] = '\0';
	}else{
		//strcpy(json_buff,"No Parse!");
		return NO_PARSE_RESULT;
	}

	//get operation
	char* q = strchr(sen,' ');
	if(q){
		strncpy(operation,p+1,q-p-1);
		operation[q-p-1]='\0';
	}else{
		return CONVERT_JSON_FAILED;
	}

	char* tmp = strchr(operation,'@');
	if(tmp){
		*tmp = '\0';
	}
	
	

	q = strchr(sen,'[');
	if(!q)
	{
		q = strchr(sen,' ');
		if(q){
			strncpy(operation,p+1,q-p-1);
			operation[q-p-1]='\0';
		}else{
			printf("Error: format error!\n");
			return CONVERT_JSON_FAILED;
			//exit(-1);
		}

		char* tmp = strchr(operation,'@');
		if(tmp){
			*tmp = '\0';
		}

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"{\n");
		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"rawtext\":\"%s\",\n",text);

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"focus\":\"%s\",\n",service);

		if( NULL != history ){
			char* p = service;
			char* q = history;

			while(*p){
				*q = tolower(*p);
				q++; p++;
			}
			*q = '\0';
		}

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"operation\":\"%s\",\n",operation);

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"message\":\"ok\",\n");

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"vesion\":\"0.1\",\n");

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"\t\"status\":%d\n",0);

		pos = json_buff + strlen(json_buff);
		sprintf(pos,"}\n");


//		pos = json_buff + strlen(json_buff);
//		sprintf(pos,"\t\"text\":\"%s\",\n",text);
//
//		pos = json_buff + strlen(json_buff);
//		sprintf(pos,"\t\"service\":\"%s\",\n",service);
//
//		pos = json_buff + strlen(json_buff);
//		sprintf(pos,"\t\"operation\":\"%s\",\n",operation);
//
//		pos = json_buff + strlen(json_buff);
//		sprintf(pos,"\t\"rc\":%d\n",0);
//
//		pos = json_buff + strlen(json_buff);
//		sprintf(pos,"}\n");

		return 0;

	}


	//check 
	//printf("%s:%s\n",service,operation);
	
	unordered_map<string,string> jsonPair;

	char *b=q;
	char *e=b;

	char buff[1024] = "";
	char key[1024] = "";
	char value[1024] = "";

	while(*b){
		e = strchr(b,'\n');

		if( e==b ){
			b = e + 1;
			continue;
		}

		if(e){
			strncpy(buff,b,e-b);
			buff[e-b] = '\0';
			b = e + 1;

			char tmp[1024]="";
			char* pp = strchr(buff,':');
			if(pp){
				strncpy(tmp,buff,pp-buff);
				tmp[pp-buff]='\0';
			}else{
				strcpy(tmp,service);
			}

			if( ! strcmp(tmp,service) )
				convert(buff,result);
			else{
				strcpy(result,"");
			}

			//printf("result %s buff %s\n",result,buff);
			//解析 result => "name":"countdowm","TimeOrig":"三分钟"
			p = result;
			while(*p){
				q = strchr(p,',');
				//get key
				char* r = strchr(p,':');
				if(r){
					strncpy(key,p,r-p);
					key[r-p]='\0';

					//get value
					if(q){
						strncpy(value,r+1,q-r-1);
						value[q-r-1] = '\0';
					}else{
						strcpy(value,r+1);
					}

					char* tmp = strchr(value,'@');
					if(tmp){
						*tmp++ = '\"';
						*tmp='\0';
					}

				}else{
					printf("Error: convert2json failed\n");
					exit(-1);
				}

			
				if( jsonPair.count(key) == 0 ){
					jsonPair[key] = value;
				}else{
					char newValue[1024];
					strcpy(newValue,jsonPair[key].c_str());
					newValue[strlen(newValue)-1] = '\0';
					//printf("%s\n",key);
					//
					if( ! strcmp(key,"\"name\"")){
						strcat(newValue,",");
					}

					strcat(newValue,value+1);

					jsonPair[key] = newValue;
				}

			
				if( NULL == q ){
					break;
				}else{
					p = q + 1;
				}

			}

			//get key
//			p = strchr(result,':');
//			q = strchr(result,'_');
//			if(q){
//				strncpy(key+1,q+1,p-q-1);
//				key[0] = '\"';
//				key[p-q] = '\0';
//			}else{
//				strncpy(key,result,p-result);
//				key[p-result] = '\0';
//			}
//
//			//printf("key %s\n",key);
//			//get value
//			strcpy(value,p+1);



			//printf("%s\n",result);
		}else{
			strcpy(buff,b);
			convert(buff,result);

			//get key
			p = strchr(result,':');
			q = strchr(result,'_');
			if(q){
				strncpy(key+1,q+1,p-q-1);
				key[0] = '\"';
				key[p-q] = '\0';
			}else{
				strncpy(key,result,p-result);
				key[p-result] = '\0';
			}

			//get value
			strcpy(value,p+1);
			char* tmp = strchr(value,'@');
			if(tmp){
				*tmp++ = '\"';
				*tmp = '\0';
			}


			if( jsonPair.count(key) == 0 ){
				jsonPair[key] = value;
			}else{
				char newValue[1024];
				strcpy(newValue,jsonPair[key].c_str());
				newValue[strlen(newValue)-1] = '\0';
				strcat(newValue,value+1);

				jsonPair[key] = newValue;
			}

			//printf("%s\n",result);
			break;
		}
	}

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"{\n");

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"slots\":{\n");

	unordered_map<string,string>::iterator iter;
	int count=0;
	for(iter = jsonPair.begin(); iter != jsonPair.end(); ++iter)
	{
		pos = json_buff + strlen(json_buff);
		//printf("\t\t%s:%s,\n",(iter->first).c_str(), (iter->second).c_str());
		count++;

		if( (unsigned)count != jsonPair.size() )
			sprintf(pos, "\t\t%s:%s,\n",(iter->first).c_str(), (iter->second).c_str() );
		else
			sprintf(pos, "\t\t%s:%s\n",(iter->first).c_str(), (iter->second).c_str() );
	}


	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t},\n");


	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"rawtext\":\"%s\",\n",text);

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"focus\":\"%s\",\n",service);

	if( NULL != history ){
		char* p = service;
		char* q = history;

		while(*p){
			*q = tolower(*p);
			q++; p++;
		}
		*q = '\0';
	}

	p = strchr(operation,'_');
	pos = json_buff + strlen(json_buff);
	if(p){
		sprintf(pos,"\t\"operation\":\"%s\",\n",p+1);
	}else{
		sprintf(pos,"\t\"operation\":\"%s\",\n",operation);
	}
	
	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"message\":\"ok\",\n");

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"vesion\":\"0.1\",\n");

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"\t\"status\":%d\n",0);

	pos = json_buff + strlen(json_buff);
	sprintf(pos,"}\n");


	//printf("%s\n",sen);
	return 0;
}

int convert2xml(char *text, char* sen,char *xml_buff, char *history)
{
	if( NULL == sen || NULL == xml_buff){
		return NO_PARSE_RESULT;
	}
	
	//rm space of text
	{
		char newText[2048] = "";
		char *pText = text, *pNewText = newText;
		while(*pText){
			if( *pText == ' '){
				pText++;
				continue;
			}
			*pNewText++ = *pText++;
		}
		*pNewText = '\0';
		strcpy(text,newText);
	}
	

	//printf("%s\n",text);

	strcpy(xml_buff,"");
	char* pos = xml_buff;

	char result[2048] = "";

	char service[32];
	char operation[32];

	//get service
	char* p = strchr(sen,':');
	if(p){
		strncpy(service,sen,p-sen);
		service[p-sen] = '\0';
	}else{
		//strcpy(xml_buff,"No Parse!");
		return NO_PARSE_RESULT;
	}

	//get operation
	char* q = strchr(sen,' ');
	if(q){
		strncpy(operation,p+1,q-p-1);
		operation[q-p-1]='\0';
	}else{
		return CONVERT_JSON_FAILED;
	}

	char* tmp = strchr(operation,'@');
	if(tmp){
		*tmp = '\0';
	}
	
	

	q = strchr(sen,'[');
	if(!q)
	{
		q = strchr(sen,' ');
		if(q){
			strncpy(operation,p+1,q-p-1);
			operation[q-p-1]='\0';
		}else{
			printf("Error: format error!\n");
			return CONVERT_JSON_FAILED;
			//exit(-1);
		}

		char* tmp = strchr(operation,'@');
		if(tmp){
			*tmp = '\0';
		}
		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t<hubo>\n");
		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<rawtext>%s</rawtext>\n",text);

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<focus>%s</focus>\n",service);

		if( NULL != history ){
			char* p = service;
			char* q = history;

			while(*p){
				*q = tolower(*p);
				q++; p++;
			}
			*q = '\0';
		}

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<operation>%s</operation>\n",operation);

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<message>ok</message>\n");

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<vesion>0.1</vesion>\n");

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t\t<status>%d</status>\n",0);

		pos = xml_buff + strlen(xml_buff);
		sprintf(pos,"\t</hubo>");

		return 0;

	}


	//check 
	//printf("%s:%s\n",service,operation);
	
	unordered_map<string,string> xmlPair;

	char *b=q;
	char *e=b;

	char buff[1024] = "";
	char key[1024] = "";
	char value[1024] = "";

	while(*b){
		e = strchr(b,'\n');

		if( e==b ){
			b = e + 1;
			continue;
		}

		if(e){
			strncpy(buff,b,e-b);
			buff[e-b] = '\0';
			b = e + 1;

			char tmp[1024]="";
			char* pp = strchr(buff,':');
			if(pp){
				strncpy(tmp,buff,pp-buff);
				tmp[pp-buff]='\0';
			}else{
				strcpy(tmp,service);
			}

			if( ! strcmp(tmp,service) )
				convert(buff,result);
			else{
				strcpy(result,"");
			}

			p = result;
			while(*p){
				q = strchr(p,',');
				char* r = strchr(p,':');
				if(r){
					//get key
					strncpy(key,p,r-p);
					key[r-p]='\0';

					//get value
					if(q){
						strncpy(value,r+1,q-r-1);
						value[q-r-1] = '\0';
					}else{
						strcpy(value,r+1);
					}
					
					char* tmp = strchr(value,'@');
					if(tmp){
						//*tmp++ = '\"';
						*tmp='\0';
					}

				}else{
					printf("Error: convert2json failed\n");
					exit(-1);
				}

		
				if( xmlPair.count(key) == 0 ){
					xmlPair[key] = value;
				}else{
					char newValue[1024];
					strcpy(newValue,xmlPair[key].c_str());
					newValue[strlen(newValue)-1] = '\0';

					strcat(newValue,value+1);

					xmlPair[key] = newValue;
				}

				if( NULL == q ){
					break;
				}else{
					p = q + 1;
				}

			}
		}else{
			strcpy(buff,b);
			convert(buff,result);

			//get key
			p = strchr(result,':');
			q = strchr(result,'_');
			if(q){
				strncpy(key,q+1,p-q-1);
				//key[0] = '\"';
				key[p-q] = '\0';
			}else{
				strncpy(key,result,p-result);
				key[p-result] = '\0';
			}

			//get value
			strcpy(value,p+1);
			char* tmp = strchr(value,'@');
			if(tmp){
				//*tmp++ = '\"';
				*tmp = '\0';
			}


			if( xmlPair.count(key) == 0 ){
				xmlPair[key] = value;
			}else{
				char newValue[1024];
				strcpy(newValue,xmlPair[key].c_str());
				newValue[strlen(newValue)-1] = '\0';
				strcat(newValue,value+1);

				xmlPair[key] = newValue;
			}
			break;
		}
	}
	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t<hubo>\n");
	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<slots>\n");

	char province[32]="";
	char city[32]="";
	char area[128]="";

	unordered_map<string,string>::iterator iter;
	for(iter = xmlPair.begin(); iter != xmlPair.end(); ++iter)
	{
		pos = xml_buff + strlen(xml_buff);
		sscanf( (iter->first).c_str(),  "\"%[^\"]", key);
		sscanf( (iter->second).c_str(), "\"%[^\"]", value);
		if( ( ! strcmp(key,"destination") ) || ( ! strcmp(key,"origin") ) ){
		//if(0){

			sprintf(pos,"\t\t\t<%s>\n",key);
			//parseAddress(value,province,city,area);
			vector<string> output;
			//int len = utf8_to_chars(value,output);
			
			string sentence="ADDRESSPARSER";
			for(unsigned i=0; i<output.size(); i++){
				sentence += " ";
				sentence += output[i];
			}
	/*		if( 0 != address_parser((char*)sentence.c_str(), value, province,city, area) ){
				pos = xml_buff + strlen(xml_buff);
				sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","area",value,"area");
				
				//printf("value %s\n",value);
				pos = xml_buff + strlen(xml_buff);
				sprintf(pos,"\t\t\t</%s>\n",key);

				continue;
		
			}
	*/	
			bool flag = false;	
			if( strlen(province) != 0 ){
				pos = xml_buff + strlen(xml_buff);
				sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","province",province,"province");
				flag = true;
			}

			if( strlen(city) != 0 ){
				pos = xml_buff + strlen(xml_buff);
				sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","city",city,"city");
				flag = true;
			}
			//printf("area %s len %d\n",area,strlen(area));	
			if( strlen(area) != 0 ){
				pos = xml_buff + strlen(xml_buff);
				if( ! strcmp(area,"站") || !strcmp(area,"大学")){
					sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","area",value,"area");
				}else{
					sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","area",area,"area");
				}
				flag = true;
			}

			if( flag == false ){
				pos = xml_buff + strlen(xml_buff);
				sprintf(pos,"\t\t\t\t<%s>%s</%s>\n","area",value,"area");
			}

			pos = xml_buff + strlen(xml_buff);
			sprintf(pos,"\t\t\t</%s>\n",key);
			
		}else{
			sprintf(pos, "\t\t\t<%s>%s</%s>\n", key, value, key);
		}
	}


	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t</slots>\n");


	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<rawtext>%s</rawtext>\n",text);

	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<focus>%s</focus>\n",service);

	if( NULL != history ){
		char* p = service;
		char* q = history;

		while(*p){
			*q = tolower(*p);
			q++; p++;
		}
		*q = '\0';
	}

	p = strchr(operation,'_');
	pos = xml_buff + strlen(xml_buff);
	if(p){
		sprintf(pos,"\t\t<operation>%s</operation>\n",p+1);
	}else{
		sprintf(pos,"\t\t<operation>%s</operation>\n",operation);
	}
	
	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<message>ok</message>\n");

	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<vesion>0.1</vesion>\n");

	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t\t<status>%d</status>\n",0);

	pos = xml_buff + strlen(xml_buff);
	sprintf(pos,"\t</hubo>\n");

	//printf("%s\n",sen);
	return 0;
}

void parse_failed_json(char* result,char* text)
{

	//rm space of text
	{
		char newText[2048] = "";
		char *pText = text, *pNewText = newText;
		while(*pText){
			if( *pText == ' '){
				pText++;
				continue;
			}
			*pNewText++ = *pText++;
		}
		*pNewText = '\0';
		strcpy(text,newText);
	}

	char* pos;
	pos = result + strlen(result);
	sprintf(pos,"{\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\"vesion\":\"0.1\",\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\"rawtext\":\"%s\",\n",text);

	pos = result + strlen(result);
	sprintf(pos,"\t\"status\":%d,\n",1);

	pos = result + strlen(result);
	sprintf(pos,"\t\"message\":\"none_match\",\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\"focus\":\"%s\"\n","OTHER");


	pos = result + strlen(result);
	sprintf(pos,"}\n");

}

void parse_failed_xml(char* result,char* text)
{

	//rm space of text
	{
		char newText[2048] = "";
		char *pText = text, *pNewText = newText;
		while(*pText){
			if( *pText == ' '){
				pText++;
				continue;
			}
			*pNewText++ = *pText++;
		}
		*pNewText = '\0';
		strcpy(text,newText);
	}

	char* pos;
	pos = result + strlen(result);
	sprintf(pos,"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

	pos = result + strlen(result);
	sprintf(pos,"\t<hubo>\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\t<rawtext>%s</rawtext>\n",text);

	pos = result + strlen(result);
	sprintf(pos,"\t\t<message>none match</message>\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\t<vesion>0.1</vesion>\n");

	pos = result + strlen(result);
	sprintf(pos,"\t\t<status>%d</status>\n",1);
	pos = result + strlen(result);
	sprintf(pos,"\t\t<focus>OTHER</focus>\n");

	pos = result + strlen(result);
	sprintf(pos,"\t</hubo>");

}


size_t utf8_to_chars(const std::string &input, std::vector<string> &output)
{
	std::string ch;
	for(size_t i=0, len = 0; i != input.length(); i += len){
		unsigned char byte = (unsigned)input[i];
		if( byte >= 0xFC)
			len = 6;
		else if( byte >= 0xF8)
			len = 5;
		else if( byte >= 0xF0)
			len = 4;
		else if( byte >= 0xE0)
			len = 3;
		else if( byte >= 0xC0)
			len = 2;
		else
			len = 1;

		ch = input.substr(i,len);
		output.push_back(ch);
	}

	return output.size();
}


//test code
//int main(int argc,char** argv)
//{
//	char json_buff[2048] = "";
//	char *sen = "Schedule:create [Sche_name].clock \nSchedule:[Sche_datetime].[Sche_TimeOrig].上 [Sche_TimeOrig].午 [Sche_TimeOrig].八 [Sche_TimeOrig].点 \nSchedule:[Sche_content].的 \nSchedule:[Sche_content].闹 \nSchedule:[Sche_content].铃";
//	//char *sen = argv[1];
//	
//	char buff[2048];
//	strcpy(buff,sen);
//
//
//	printf("INPUT:%s\n",buff);
//
//	convert2json(buff,json_buff);
//
//	printf("%s\n",json_buff);
//	return 0;
//}
