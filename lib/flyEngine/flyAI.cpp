#include "../Fly3D.h"

int a_star(flyBspNode* source,flyBspNode* dest,flyArray<a_star_node*> nodes,flyArray<flyBspNode*>& path);
float get_cost(flyBspNode* a,flyBspNode* b);
int get_best_node(flyArray<a_star_node*> list, flyBspNode* dest,a_star_node* &node);
int is_in(flyArray<a_star_node*> list, a_star_node* node);

int flyEngine::find_path(const flyVector &source,const flyVector &sourcedir,float sourceradius,const flyVector &dest,flyBezierCurve &curve)
{
	flyArray<flyBspNode*> nodepath;
	flyArray<a_star_node*> a_star_nodes;
	flyArray<flyVector> path,newpath;

	int ret1,ret2;

	path.clear();

	a_star_nodes.reserve(nleaf);
	int i,j;
	for(i=0;i<nleaf;i++)
	{
		a_star_node *n=new a_star_node;
		n->leaf=-1;
		a_star_nodes.add(n);
	}

	flyBspNode *a=find_node(source);
	flyBspNode *b=find_node(dest);

	if(!a||!b)
		return 0;
	
	ret1=a_star(a,b,a_star_nodes,nodepath);

	for(i=0;i<a_star_nodes.num;i++)
		delete a_star_nodes[i];

	flyBspNode* n1;
	flyBspNode* n2;
	
	path.add(source);
	path.add(source+sourcedir*sourceradius);

	for(i=0;i<nodepath.num-1;i++)
	{
		n1=nodepath[(nodepath.num-1)-i];
		n2=nodepath[(nodepath.num-2)-i];

		for(j=0;j<n1->neighbors.num;j++)
			if(n1->neighbors[j]==n2)
				break;

		path.add(n1->centre+(n1->portals[j].get_center()-n1->centre)*0.5f);
		path.add(n1->portals[j].get_center());
		path.add(n2->centre+(n1->portals[j].get_center()-n2->centre)*0.5f);
	}

	if(nodepath.num>1)
	{
		n1=nodepath[1];
		n2=nodepath[0];

		for(j=0;j<n1->neighbors.num;j++)
			if(n1->neighbors[j]==n2)
				break;

		path.add(n2->centre+(n1->portals[j].get_center()-n2->centre)*0.5f);
	}
	else
		path.add(dest+(source-dest)*0.5f);

	path.add(dest);

	ret2=smooth_path(path,newpath);

	curve.reset();
	curve.set_dim(3);
	for(i=0;i<newpath.num;i++)
		curve.add_point(&newpath[i].x);

	return (ret1 && ret2);
}

int flyEngine::smooth_path(const flyArray<flyVector> &path,flyArray<flyVector> &newpath)
{
	int i,j,k;
	flyArray<int> flags;
	flags.reserve(path.num);

	newpath.clear();
	
	for(i=0;i<path.num;i++)
		flags.add(1);

	for(i=0;i<path.num-6;i+=3)
		for(j=i+6;j<path.num;j+=3)
		{
			if(!flags[i])
				break;
			if(!collision_test(path[i],path[j],FLY_TYPE_STATICMESH))
				for(k=i+2;k<j-1;k++)
					flags[k]=0;
		}

	for(i=0;i<flags.num;i++)
		if(flags[i])
			newpath.add(path[i]);

	return (newpath.num%3)==1;
}

float get_cost(flyBspNode* a,flyBspNode* b)
{
	return (a->centre-b->centre).length();
}

int get_best_node(flyArray<a_star_node*> list, flyBspNode* dest,a_star_node* &node)
{
	float f,mindist=FLY_BIG;
	int best=-1;
	
	for(int i=0;i<list.num;i++)
		if(list[i]->leaf!=-1)
		{
			f=list[i]->cost+list[i]->estimate;
			if(f<mindist)
			{
				mindist=f;
				best=i;
				node=list[i];
			}
		}
	
	return best;
}

int is_in(flyArray<a_star_node*> list, a_star_node* node)
{
	if(node->leaf==-1)
		return -1;
	
	for(int i=0;i<list.num;i++)
		if(list[i]->leaf==node->leaf)
			return i;

	return -1;
}

int a_star(flyBspNode* source,flyBspNode* dest,flyArray<a_star_node*> nodes,flyArray<flyBspNode*>& path)
{
	flyArray<a_star_node*> open,closed;
	a_star_node* node=0;
	int ind;
	
	nodes[source->leaf]->leaf=source->leaf;
	nodes[source->leaf]->cost=0;
	nodes[source->leaf]->estimate=get_cost(source,dest);
	nodes[source->leaf]->parent=0;
	open.add(nodes[source->leaf]);
	
	while(open.num)
	{
		ind=get_best_node(open,dest,node);
		open.remove(ind);

		if(g_flyengine->leaf[node->leaf]==dest)
		{
			path.add(g_flyengine->leaf[node->leaf]);
			while(node->parent)
			{
				node=node->parent;
				path.add(g_flyengine->leaf[node->leaf]);
			}

			return 1;
		}
		else
		{
			for(int i=0;i<g_flyengine->leaf[node->leaf]->neighbors.num;i++)
			{
				int neighbor_leaf=g_flyengine->leaf[node->leaf]->neighbors[i]->leaf;
				a_star_node* newnode=nodes[neighbor_leaf];
				
				if(newnode->leaf==-1)
				{
					newnode->leaf=neighbor_leaf;
					newnode->cost=get_cost(source,g_flyengine->leaf[neighbor_leaf]);
				}

				float newcost=node->cost+get_cost(g_flyengine->leaf[node->leaf],g_flyengine->leaf[newnode->leaf]);

				if(((is_in(open,newnode)!=-1)||(is_in(closed,newnode)!=-1))&&(newcost>=newnode->cost))
					continue;
				else
				{
					newnode->parent=nodes[node->leaf];
					newnode->cost=newcost;
					newnode->estimate=get_cost(g_flyengine->leaf[newnode->leaf],dest);

					int j=is_in(closed,newnode);
					if(j!=-1)
						closed.remove(j);
					j=is_in(open,newnode);
					if(j!=-1)
						open[j]=newnode;
					else
						open.add(newnode);
				}

				closed.add(node);
			}
		}
	}

	return 0;
}

int flyStateMachine::load_behaviour(const char *filename)
{
	int i,j;
	char str[255];
	
	FILE *fp=fopen(filename,"r+t");
	if(fp)
	{
		fscanf(fp,"%i",&nstates);
		for(i=0;i<nstates;i++)
			fscanf(fp,"%s",&str);

		fscanf(fp,"%i",&ninputs);
		for(i=0;i<ninputs;i++)
			fscanf(fp,"%s",&str);

		fscanf(fp,"%i",&nfactors);
		for(i=0;i<nfactors;i++)
			fscanf(fp,"%s",&str);

		rules.clear();

		int nrules;
		fscanf(fp,"%i",&nrules);
		for(i=0;i<nrules;i++)
		{
			for(j=0;j<5;j++)
				fscanf(fp,"%s",&str);
			flyStateMachineRule rule;
			fscanf(fp,"%i",&rule.output);
			fscanf(fp,"%i",&rule.state);
			fscanf(fp,"%i",&rule.positive_input);
			fscanf(fp,"%i",&rule.negative_input);
			fscanf(fp,"%i",&rule.factorindex);

			rules.add(rule);
		}

		fclose(fp);
		currentstate=0;
		return 1;
	}

	return 0;
}

int flyStateMachine::transition(int input,float *factors)
{
	if(currentstate==-1)
		return -1;
	
	int i;
	flyArray<flyStateMachineRule> selected;

	for(i=0;i<rules.num;i++)
	{
		if((rules[i].state==-1 || rules[i].state==currentstate))
		if((rules[i].positive_input==-1	|| rules[i].positive_input&input))
		if((rules[i].negative_input==-1	|| !(rules[i].negative_input&input)))
		if((rules[i].factorindex < nfactors))

				selected.add(rules[i]);
	}

	float f=0.0f;
	int flag=0;

	for(i=0;i<selected.num;i++)
	{
		if(flag)
		{
			if(selected[i].factorindex==-1)
				f+=1.0f;
			else
				selected.remove(i--);
		}
		else
		{
			if(selected[i].factorindex==-1)
			{
				flag=1;
				f=0.0f;
				i--;
			}
			else
				f+=factors[selected[i].factorindex];
		}
	}

	f*=FLY_FABSRAND;

	if(flag)
	{
		for(i=0;i<selected.num;i++)
			if(f<(float)(i+1))
			{
				currentstate=selected[i].output;
				break;
			}
	}
	else
		for(i=0;i<selected.num;i++)
			if(f>factors[selected[i].factorindex])
				f-=factors[selected[i].factorindex];
			else
			{
				currentstate=selected[i].output;
				break;
			}
	
	return currentstate;
}

flyStateMachine::flyStateMachine(const flyStateMachine& in) :
	nstates(in.nstates),
	ninputs(in.ninputs),
	nfactors(in.nfactors),
	currentstate(in.currentstate)
{
	rules=in.rules;
}
