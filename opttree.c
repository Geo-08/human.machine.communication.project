#include <math.h>
#include "opttree.h"
#include <unistd.h>

query* opt_query(TableStorage* store,char* tq){
	query* qu;
	query_init(&qu);
	read_query(qu,tq);
	int i,place;
	int relnum,colnum;
	node* snode;
	snode = node_init();
	for(i=0;i<qu->fnum;i++){
		relnum = qu->relation_numbers[qu->filters[i].rel.rel];
		colnum = qu->filters[i].rel.col;
		place = find_place_n(snode,qu->filters[i].rel.rel);
		if(place == -1){
			place = add_stat(snode,qu->filters[i].rel.rel,store->tables[relnum]->numColumns,store->tables[relnum]->stats);
		}
		if(qu->filters[i].op == '<')
			lower_than_s(snode,place,qu->filters[i].num,qu->filters[i].rel.col);
		if(qu->filters[i].op == '>')
			bigger_than_s(snode,place,qu->filters[i].num,qu->filters[i].rel.col);
		if(qu->filters[i].op == '=')
			equal_to_s(snode,place,qu->filters[i].num,qu->filters[i].rel.col,store->tables[relnum]->relations[colnum],store->tables[relnum]->numColumns);
	}
	node* tnode = snode;
	unity* tj;
	for(i=0;i<qu->unum;i++){
		tj = find_not_used_join(qu->unitys,qu->unum,tnode->joins,tnode->join_num);
		expand_node(tnode,tj,qu->unum-tnode->join_num,store,qu->relation_numbers);
		if(i != 0)
			free(tj);
		tnode = find_best_child(tnode);
	}
	for(i=0;i<qu->unum;i++){
		qu->unitys[i] = tnode->joins[i];
	}
	delete_tree(snode);
	return qu;
}

void delete_tree(node* nod){
	int i,j,g;
	for(i=0;i<nod->next_num;i++)
		delete_tree(nod->next[i]);
	for(i=0;i<nod->rs_num;i++){
		for(j=0;j<nod->rss[i].id_num;j++)
			free(nod->rss[i].stats[j]);
		free(nod->rss[i].stats);
		free(nod->rss[i].col_num);
		free(nod->rss[i].ids);
	}
	free(nod->rss);
	free(nod->joins);
	free(nod->next);
	free(nod);
}


unity* find_not_used_join(unity* from,int fnum, unity* outof,int onum){
	if(onum == 0)
		return from;
	int i,j,count = 0;
	unity* temp = malloc(sizeof(unity)*(fnum - onum));
	for(i=0;i<fnum;i++){
		for(j=0;j<onum;j++){
			if(un_comp(from[i],outof[j])==1)
				break;
			if(j == onum-1){
				temp[count] = from[i];
				count++;
			}	
		}
	}
	return temp;
}

uint64_t calc_total_tuples(node* nod){
	int i;
	uint64_t count=0;
	for(i=0;i<nod->rs_num;i++)
		count = count + nod->rss[i].stats[0][0].f;
	return count;
}

node* find_best_child(node* nod){
	int i,place=0;
	uint64_t count,tcount;
	count = calc_total_tuples(nod->next[0]);
	for(i=1;i<nod->next_num;i++){
		tcount = calc_total_tuples(nod->next[i]);
		if(tcount < count){
			place = i;
			count = tcount;
		}
	}
	return nod->next[place];
}

void copy_node(node* nod, node* copy){
	copy->rs_num = nod->rs_num;
	copy->rss = malloc(sizeof(rs)*copy->rs_num);
	copy->join_num = nod->join_num;
	if(copy->join_num !=0)
		copy->joins = malloc(sizeof(unity)*copy->join_num);
	int i,j,g;
	for(i=0;i<copy->join_num;i++)
		copy->joins[i] = nod->joins[i];

	for(i=0;i<copy->rs_num;i++){
		copy->rss[i].id_num = nod->rss[i].id_num;
		copy->rss[i].ids = malloc(sizeof(int)*copy->rss[i].id_num);
		copy->rss[i].col_num = malloc(sizeof(int)*copy->rss[i].id_num);
		copy->rss[i].stats = malloc(sizeof(statistic*)*copy->rss[i].id_num);
		for(j=0;j<copy->rss[i].id_num;j++){
			copy->rss[i].ids[j] = nod->rss[i].ids[j];
			copy->rss[i].col_num[j] = nod->rss[i].col_num[j];
			copy->rss[i].stats[j] = malloc(sizeof(statistic)*copy->rss[i].col_num[j]);
			for(g=0;g<copy->rss[i].col_num[j];g++)
				copy->rss[i].stats[j][g] = nod->rss[i].stats[j][g];	
		}
		
	}
}

void exec_pred(node* nod,unity join,int* rel_nums,TableStorage* store){
	int i,place1, place2,rel1,rel2,col1,col2;
	printf("hello\n");
	rel1 = rel_nums[join.rel1.rel];
	col1 = join.rel1.col;
	place1 = find_place_n(nod,join.rel1.rel);
	if(place1 == -1){
			place1 = add_stat(nod,join.rel1.rel,store->tables[rel1]->numColumns,store->tables[rel1]->stats);	
	}
	rel2 = rel_nums[join.rel2.rel];
	col2 = join.rel2.col;
	place2 = find_place_n(nod,join.rel2.rel);
	if(place2 == -1){
			place2 = add_stat(nod,join.rel2.rel,store->tables[rel2]->numColumns,store->tables[rel2]->stats);	
	}
	for(i=0;i<nod->rss[place1].id_num;i++){
		if(nod->rss[place1].ids[i] == join.rel1.rel){
			rel1 = i;
			break;
		}
	}
	for(i=0;i<nod->rss[place2].id_num;i++){
		if(nod->rss[place2].ids[i] == join.rel2.rel){
			rel2 = i;
			break;
		}
	}
	printf("hi\n");
	if(place1 == place2){
		if(rel1 == rel2 && col1 == col2){
			selfrel(nod,place1,col1,rel1);
		}
		else{
			same_matrix(nod,place1,rel1,col1,rel2,col2);
		}
	}
	else{
		different_matrix(nod,place1,rel1,col1,place2,rel2,col2);
	}
}

void expand_node(node* nod,unity* joins,int join_num,TableStorage* store,int* rel_nums){
	node* cnode;
	if(join_num != 0)
		nod->next = malloc(sizeof(node*)*join_num);
	nod->next_num = join_num;
	int i;
	for(i=0;i<join_num;i++){
		cnode = node_init();
		copy_node(nod,cnode);
		if(cnode->join_num ==0)
			cnode->joins = malloc(sizeof(unity));
		else
			cnode->joins = realloc(cnode->joins,sizeof(unity)*(cnode->join_num+1));
		cnode->joins[cnode->join_num] = joins[i];
		cnode->join_num++;
		exec_pred(cnode,joins[i],rel_nums,store);
		nod->next[i] = cnode;
	}
}

void different_matrix(node* nod,int place1,int rel1,int col1,int place2, int rel2, int col2){
	rs temp;
	double dt,dt2;
	uint64_t i,j,c,n,tempf1,tempf2;
	temp.id_num = (nod->rss[place1].id_num)+(nod->rss[place2].id_num);
	temp.ids = malloc(sizeof(int)*temp.id_num);
	temp.col_num = malloc(sizeof(int)*temp.id_num);
	temp.stats = malloc(sizeof(statistic*)*temp.id_num);
	temp.id_num = nod->rss[place1].id_num+nod->rss[place2].id_num;
	if(nod->rss[place1].stats[rel1][col1].u < nod->rss[place2].stats[rel2][col2].u)
		nod->rss[place2].stats[rel2][col2].u = nod->rss[place1].stats[rel1][col1].u;
	else
		nod->rss[place1].stats[rel1][col1].u = nod->rss[place2].stats[rel2][col2].u;
	if(nod->rss[place1].stats[rel1][col1].l > nod->rss[place2].stats[rel2][col2].l)
		nod->rss[place2].stats[rel2][col2].l = nod->rss[place1].stats[rel1][col1].l;
	else
		nod->rss[place1].stats[rel1][col1].l = nod->rss[place2].stats[rel2][col2].l;
	
	n = nod->rss[place1].stats[rel1][col1].u - nod->rss[place1].stats[rel1][col1].l - 1;
	tempf1 = nod->rss[place1].stats[rel1][col1].f;
	tempf2 = nod->rss[place2].stats[rel2][col2].f;
	nod->rss[place1].stats[rel1][col1].f = tempf1 * tempf2/n;
	nod->rss[place2].stats[rel2][col2].f = nod->rss[place1].stats[rel1][col1].f;
	nod->rss[place1].stats[rel1][col1].d = nod->rss[place1].stats[rel1][col1].d*nod->rss[place2].stats[rel2][col2].d/n;
	nod->rss[place2].stats[rel2][col2].d = nod->rss[place1].stats[rel1][col1].d;
	dt = (double)(nod->rss[place1].stats[rel1][col1].f)/tempf1;
	
	for(i=0;i<nod->rss[place1].id_num;i++){
		temp.ids[i] = nod->rss[place1].ids[i];
		temp.col_num[i] = nod->rss[place1].col_num[i];
		temp.stats[i] = malloc(sizeof(statistic)*nod->rss[place1].col_num[i]);
		
		for(j=0;j<nod->rss[place1].col_num[i];j++){
			temp.stats[i][j].l = nod->rss[place1].stats[i][j].l;
			temp.stats[i][j].u = nod->rss[place1].stats[i][j].u;
			
			if(i == rel1 && j == col1){
				temp.stats[i][j].d = nod->rss[place1].stats[rel1][col1].d;
				temp.stats[i][j].f = nod->rss[place1].stats[rel1][col1].f;
				continue;
			}
			dt2 = (double)(nod->rss[place1].stats[i][j].f)/nod->rss[place1].stats[i][j].d;
			//temp.stats[i][j].d = nod->rss[place1].stats[i][j].d * (1-pow(1-(nod->rss[place1].stats[rel1][col1].f/tempf1),nod->rss[place1].stats[i][j].f/nod->rss[place1].stats[i][j].d));
			temp.stats[i][j].d = nod->rss[place1].stats[i][j].d * (1-pow(1-dt,dt2));
			temp.stats[i][j].f = nod->rss[place1].stats[rel1][col1].f;
		}
	}

	dt = (double)(nod->rss[place1].stats[rel1][col1].f)/tempf2;
/*	for(i=nod->rss[place1].id_num;i<temp.id_num;i++){
		temp.ids[i] = nod->rss[place2].ids[i-nod->rss[place1].id_num];
		temp.col_num[i] = nod->rss[place2].col_num[i-nod->rss[place1].id_num];
		temp.stats[i] = malloc(sizeof(statistic)*nod->rss[place2].col_num[i-nod->rss[place1].id_num]);
		printf("check 2.5\n");
		for(j=0;j<nod->rss[place2].col_num[i-nod->rss[place1].id_num];j++){
			temp.stats[i][j].l = nod->rss[place2].stats[i-nod->rss[place1].id_num][j].l;
			temp.stats[i][j].u = nod->rss[place2].stats[i-nod->rss[place1].id_num][j].u;

			if(i == rel2 && j == col2){
				temp.stats[i][j].d = nod->rss[place2].stats[rel2][col2].d;
				temp.stats[i][j].f = nod->rss[place2].stats[rel2][col2].f;
				continue;
			}
			dt2 = (double)(nod->rss[place2].stats[i-nod->rss[place1].id_num][j].f)/nod->rss[place2].stats[i-nod->rss[place1].id_num][j].d;
			//temp.stats[i][j].d = nod->rss[place2].stats[i-nod->rss[place1].id_num][j].d * (1-pow(1-(nod->rss[place1].stats[rel1][col1].f/tempf2),nod->rss[place2].stats[i-nod->rss[place1].id_num][j].f/nod->rss[place2].stats[i-nod->rss[place1].id_num][j].d));
			temp.stats[i][j].d = nod->rss[place2].stats[i-nod->rss[place1].id_num][j].d * (1-pow(1-dt,dt2));
			temp.stats[i][j].f = nod->rss[place1].stats[rel1][col1].f;
		}
	}*/
	c=i;

	for(i=0;i<nod->rss[place2].id_num;i++){
		c=i+c;	
		
		temp.ids[c] = nod->rss[place2].ids[i];
		temp.col_num[c] = nod->rss[place2].col_num[i];
		temp.stats[c] = malloc(sizeof(statistic)*nod->rss[place2].col_num[i]);
		for(j=0;j<nod->rss[place2].col_num[i];j++){
			 
			temp.stats[c][j].l = nod->rss[place2].stats[i][j].l;
			temp.stats[c][j].u = nod->rss[place2].stats[i][j].u;
			if(i == rel2 && j == col2){
				temp.stats[c][j].d = nod->rss[place2].stats[rel2][col2].d;
				temp.stats[c][j].f = nod->rss[place2].stats[rel2][col2].f;
				continue;
			}
			dt2 = (double)(nod->rss[place2].stats[i][j].f)/nod->rss[place2].stats[i][j].d;
			temp.stats[c][j].d = nod->rss[place2].stats[i][j].d * (1-pow(1-dt,dt2));
			temp.stats[c][j].f = nod->rss[place1].stats[rel1][col1].f;
		}
	}
	nod->rs_num--;
	int flag = 0;
	rs* temps;
	temps = malloc(sizeof(rs)*nod->rs_num);
	for(i=0;i<nod->rss[place1].id_num;i++){
		//print_stats(nod->rss[place1].stats[i],nod->rss[place1].col_num[i]);
		//printf("%p\n",nod->rss[place1].stats[i]);
		free(nod->rss[place1].stats[i]);
	}
	for(i=0;i<nod->rss[place2].id_num;i++)
		free(nod->rss[place2].stats[i]);
	free(nod->rss[place1].stats);
	free(nod->rss[place2].stats);
	free(nod->rss[place1].ids);
	free(nod->rss[place2].ids);
	free(nod->rss[place1].col_num);
	free(nod->rss[place2].col_num);
	for(i=0;i<nod->rs_num;i++){
		if(flag == 2){
			temps[i] = nod->rss[i+1];
		}
		if(flag ==0){
			if(i == place1 || i == place2){
				temps[i] = temp;
				flag++;
			}
			else
				temps[i] = nod->rss[i];
		}
		else if(flag == 1){
			if(i == place1 || i == place2){
				temps[i] = nod->rss[i+1];
				flag++;
				continue;
			}
			else
				temps[i] = nod->rss[i];
		}
	}
	free(nod->rss);
	nod->rss = temps;
}

void same_matrix(node* nod,int place, int rel1, int col1,int rel2,int col2){
	uint64_t i,j,n,tempf;
	double dt,dt2;
	if(nod->rss[place].stats[rel1][col1].u < nod->rss[place].stats[rel2][col2].u)
		nod->rss[place].stats[rel2][col2].u = nod->rss[place].stats[rel1][col1].u;
	else
		nod->rss[place].stats[rel1][col1].u = nod->rss[place].stats[rel2][col2].u;

	if(nod->rss[place].stats[rel1][col1].l > nod->rss[place].stats[rel2][col2].l)
		nod->rss[place].stats[rel2][col2].l = nod->rss[place].stats[rel1][col1].l;
	else
		nod->rss[place].stats[rel1][col1].l = nod->rss[place].stats[rel2][col2].l;

	n = nod->rss[place].stats[rel1][col1].u - nod->rss[place].stats[rel1][col1].l - 1;
	tempf = nod->rss[place].stats[rel1][col1].f;
	nod->rss[place].stats[rel1][col1].f = nod->rss[place].stats[rel1][col1].f/n;
	nod->rss[place].stats[rel2][col2].f = nod->rss[place].stats[rel1][col1].f;
	dt = (double)(nod->rss[place].stats[rel1][col1].f)/tempf;
	dt2 = (double)(tempf)/nod->rss[place].stats[rel1][col1].d;
	//nod->rss[place].stats[rel1][col1].d = nod->rss[place].stats[rel1][col1].d * (1-pow(1-(nod->rss[place].stats[rel1][col1].f/tempf),tempf/nod->rss[place].stats[rel1][col1].d));
	nod->rss[place].stats[rel1][col1].d = nod->rss[place].stats[rel1][col1].d * (1-pow(1-dt,dt2));
	nod->rss[place].stats[rel2][col2].d = nod->rss[place].stats[rel1][col1].d;
	for(i=0;i<nod->rss[place].id_num;i++){
		for(j=0;j<nod->rss[place].col_num[i];j++){
			if((i == rel1 && j == col1) || (i == rel2 && j == col2))
				continue;
			dt2 = (double)(nod->rss[place].stats[i][j].f)/nod->rss[place].stats[i][j].d;
			//nod->rss[place].stats[i][j].d = nod->rss[place].stats[i][j].d * (1-pow(1-(nod->rss[place].stats[rel1][col1].f/tempf),nod->rss[place].stats[i][j].f/nod->rss[place].stats[i][j].d));
			nod->rss[place].stats[i][j].d = nod->rss[place].stats[i][j].d * (1-pow(1-dt,dt2));
			nod->rss[place].stats[i][j].f = nod->rss[place].stats[rel1][col1].f;
		}
	}
}


void selfrel(node* nod,int place,int col,int rel){
	uint64_t i,j,n;

	n = nod->rss[place].stats[rel][col].u - nod->rss[place].stats[rel][col].l - 1;
	nod->rss[place].stats[rel][col].f = nod->rss[place].stats[rel][col].f*nod->rss[place].stats[rel][col].f/n;
	for(i=0;i<nod->rss[place].id_num;i++)
		for(j=0;j<nod->rss[place].col_num[i];j++){
			if(i == rel && j == col)
				continue;
			nod->rss[place].stats[i][j].f = nod->rss[place].stats[rel][col].f;
	}

}

void lower_than_s(node* nod,int place,uint64_t value,int col){
	uint64_t tempu,tempd,tempf,i;
	if(nod->rss[place].stats[0][col].l >= value){
		for(i=0;i<nod->rss[place].col_num[0];i++){
			nod->rss[place].stats[0][i].d = 0;
			nod->rss[place].stats[0][i].f = 0;
		}
		return;
	}
	if(nod->rss[place].stats[0][col].u == nod->rss[place].stats[0][col].l)
		return;
	
	tempu = nod->rss[place].stats[0][col].u;
	tempd = nod->rss[place].stats[0][col].d;
	tempf = nod->rss[place].stats[0][col].f;
	if(nod->rss[place].stats[0][col].u > value)
		nod->rss[place].stats[0][col].u = value;

	double dt = ((double)(nod->rss[place].stats[0][col].u - nod->rss[place].stats[0][col].l)/(tempu - nod->rss[place].stats[0][col].l)) ;
	double dt2;
	nod->rss[place].stats[0][col].d = dt * nod->rss[place].stats[0][col].d;
	nod->rss[place].stats[0][col].f = dt * nod->rss[place].stats[0][col].f;
	dt = ((double)(nod->rss[place].stats[0][col].f))/tempf;
	for(i=0;i<nod->rss[place].col_num[0];i++){
		if(i == col)
			continue;
		
		dt2 = ((double)(nod->rss[place].stats[0][i].f))/nod->rss[place].stats[0][i].d;
		//nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow(1-(nod->rss[place].stats[0][col].f/tempf),nod->rss[place].stats[0][i].f/nod->rss[place].stats[0][i].d));
		nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow(1-dt,dt2));
		nod->rss[place].stats[0][i].f = nod->rss[place].stats[0][col].f;
	}
	
}

void bigger_than_s(node* nod,int place,uint64_t value,int col){
	uint64_t templ,tempd,tempf,i;
	if(nod->rss[place].stats[0][col].u <= value){
		for(i=0;i<nod->rss[place].col_num[0];i++){
			nod->rss[place].stats[0][i].d = 0;
			nod->rss[place].stats[0][i].f = 0;
		}
		return;
	}
	if(nod->rss[place].stats[0][col].u == nod->rss[place].stats[0][col].l)
		return;
	
	templ = nod->rss[place].stats[0][col].l;
	tempd = nod->rss[place].stats[0][col].d;
	tempf = nod->rss[place].stats[0][col].f;
	if(nod->rss[place].stats[0][col].l < value)
		nod->rss[place].stats[0][col].l = value;
	double dt = ((double)(nod->rss[place].stats[0][col].u - nod->rss[place].stats[0][col].l)/(nod->rss[place].stats[0][col].u - templ)) ;
	double dt2;
	nod->rss[place].stats[0][col].d = dt * nod->rss[place].stats[0][col].d;
	nod->rss[place].stats[0][col].f = dt * nod->rss[place].stats[0][col].f;
	dt = ((double)(nod->rss[place].stats[0][col].f))/tempf;
	for(i=0;i<nod->rss[place].col_num[0];i++){
		if(i == col)
			continue;
		dt2 = ((double)(nod->rss[place].stats[0][i].f))/nod->rss[place].stats[0][i].d;
		//nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow((double)(1-(nod->rss[place].stats[0][col].f/tempf)),nod->rss[place].stats[0][i].f/nod->rss[place].stats[0][i].d));
		nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow(1-dt,dt2));
		nod->rss[place].stats[0][i].f = nod->rss[place].stats[0][col].f;
	}
}
	

void equal_to_s(node* nod,int place,uint64_t value,int col,uint64_t* ar,uint64_t colnum){
	uint64_t tempf,tempd;
	bool flag = false;
	tempd = nod->rss[place].stats[0][col].d;
	tempf = nod->rss[place].stats[0][col].f;
	nod->rss[place].stats[0][col].d = 0;
	nod->rss[place].stats[0][col].f = 0;
	if(value > nod->rss[place].stats[0][col].u || value < nod->rss[place].stats[0][col].l){
		flag = true;
	}
	int i;
	if(flag == false){
		for(i=0;i<colnum;i++){
			if(value == ar[i]){
				nod->rss[place].stats[0][col].d = 1;
				nod->rss[place].stats[0][col].f = tempf/tempd;
				break;
			}
		}
	}
	nod->rss[place].stats[0][col].l = value;
	nod->rss[place].stats[0][col].u = value;
	double dt,dt2;
	dt = ((double)(nod->rss[place].stats[0][col].f))/tempf;
	for(i=0;i<nod->rss[place].col_num[0];i++){
		if(i == col)
			continue;
		
		dt2 = ((double)(nod->rss[place].stats[0][i].f))/nod->rss[place].stats[0][i].d;
		//nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-(1-pow((nod->rss[place].stats[0][col].f/tempf),nod->rss[place].stats[0][i].f/nod->rss[place].stats[0][i].d)));
		//nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow(1-(nod->rss[place].stats[0][col].f/tempf),nod->rss[place].stats[0][i].f/nod->rss[place].stats[0][i].d));
		nod->rss[place].stats[0][i].d = nod->rss[place].stats[0][i].d * (1-pow(1-dt,dt2));
		nod->rss[place].stats[0][i].f = nod->rss[place].stats[0][col].f;
	}
}

/*node* create_copy(node* nod){
	node* no;
	no = malloc(sizeof(node));
	
}*/

/*void find_children(node* nod,unity* unitys,int u_num){
	while
}

void expand_at(node* nod,unity* unitys,int u_num,int target){
	node*
}*/

int add_stat(node* nod,int name,uint64_t col,statistic* stats){
	if(nod->rs_num ==0)
		nod->rss = malloc(sizeof(rs));
	else
		nod->rss = realloc(nod->rss,sizeof(rs)*(nod->rs_num+1));
	nod->rss[nod->rs_num].ids = malloc(sizeof(int));
	nod->rss[nod->rs_num].ids[0] = name;
	nod->rss[nod->rs_num].id_num = 1;
	nod->rss[nod->rs_num].col_num = malloc(sizeof(int));
	nod->rss[nod->rs_num].col_num[0] = col;
	nod->rss[nod->rs_num].stats = malloc(sizeof(statistic*));
	nod->rss[nod->rs_num].stats[0] = malloc(sizeof(statistic)*col);
	uint64_t i;
	for(i=0;i<col;i++){
		nod->rss[nod->rs_num].stats[0][i].l = stats[i].l;
		nod->rss[nod->rs_num].stats[0][i].u = stats[i].u;
		nod->rss[nod->rs_num].stats[0][i].d = stats[i].d;
		nod->rss[nod->rs_num].stats[0][i].f = stats[i].f;
	}
	nod->rs_num++;
	return (nod->rs_num-1);
}

node* node_init(){

	node* n;
	n = malloc(sizeof(node));
	n->rs_num =0;
	n->next_num =0;
	n->next = NULL;
	n->rss = NULL;
	n->joins = NULL;
	n->join_num = 0;
	return n;
}


int find_place_n(node* nod,int rel){
	int i,j;
	for(i=0;i<nod->rs_num;i++){
		for(j=0;j<nod->rss[i].id_num;j++){
			if(rel == nod->rss[i].ids[j])
				return i;
		}
	}
	return -1;
}

int un_comp(unity first,unity second){
	if(first.rel1.rel == second.rel1.rel && first.rel1.col == second.rel1.col)
		return 1;
	return 0;
}
