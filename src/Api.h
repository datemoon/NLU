#ifndef __API_H__
#define __API_H__ 1

#ifdef   __cplusplus
extern   "C"
{
#endif
/*
 * make graph and return source point.
 * */
void *mkgraph_source(char *path ,char *gram, char * frames);

/*
 * read source.
 * */
void *read_source(char *gram,char *dict,char *netlist,char *outframes);

/*
 * save fst format source
 * */
int write_source(void *source, char *gram,char *dict,char *netlist,char *outframes);

/*
 * init parse handle.
 * */
void *init_hubo_parse(void *source);

/*
 * analysis sentence.
 * */
int hubo_parse(void *parse, char *sent, char *result, float *score);

/*
 * destory parse .
 * */
void destory_hubo_parse(void *parse);

/*
 * destory source .
 * */
void destory_source(void *source);

char *get_history();
#ifdef   __cplusplus
}
#endif

#endif
