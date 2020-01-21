#ifndef OPTTREE_H
#define OPTTREE_H

#include "opperations.h"

typedef struct unity unity ;
typedef struct query query ;
typedef struct statistic statistic ;
typedef struct TableStorage TableStorage ;

typedef struct rs{ 
	int* ids;
	int id_num;
	int* col_num;
	statistic** stats;
}rs;

typedef struct node{
	rs* rss;
	int rs_num;
	unity* joins;
	int join_num;
	struct node** next;
	int next_num;
}node;


typedef struct tree{
	node* start;
	int depth;
}tree;



void print_un(unity un);
void un_copy(unity* to, unity from);
int un_comp(unity first,unity second);
void delete_tree(node* nod);
unity* find_not_used_join(unity* from,int fnum, unity* outof,int onum);
uint64_t calc_total_tuples(node* nod);
node* find_best_child(node* nod);
void copy_node(node* nod, node* copy);
void exec_pred(node* nod,unity join,int* rel_nums,TableStorage* store);
void expand_node(node* nod,unity* joins,int join_num,TableStorage* store,int* rel_nums);
query* opt_query(TableStorage* store,char* tq);
void different_matrix(node* nod,int place1,int rel1,int col1,int place2, int rel2, int col2);
void same_matrix(node* nod,int place, int rel1, int col1,int rel2,int col2);
void selfrel(node* nod,int place,int col,int rel);
void lower_than_s(node* nod,int place,uint64_t value,int col);
void bigger_than_s(node* nod,int place,uint64_t value,int col);
void equal_to_s(node* nod,int place,uint64_t value,int col,uint64_t* ar,uint64_t colnum);
int add_stat(node* nod,int name,uint64_t col,statistic* stats);
node* node_init();
int find_place_n(node* nod,int rel);

#endif
