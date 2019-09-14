#include "../Fly3D.h"

#define FLY_OCTREE_MINFACES 16

// flyOcTreeNode

flyOcTreeNode::flyOcTreeNode()
{
	for( int i=0;i<8;i++ )
		nodes[i]=0;
}

flyOcTreeNode::~flyOcTreeNode()
{
	for( int i=0;i<8;i++ )
		delete nodes[i];
}

flyOcTreeNode::flyOcTreeNode(flyOcTreeNode& in) :
	bbox(in.bbox),faces(in.faces)
{
	for( int i=0;i<8;i++ )
		if (in.nodes[i]==0)
			nodes[i]=0;
		else
			nodes[i]=new flyOcTreeNode(*in.nodes[i]);
}

void flyOcTreeNode::build_node(int *triverts,flyVertex *verts)
{
	if (faces.num<=FLY_OCTREE_MINFACES)
		return;

	int i,j;
	flyVector center=(bbox.min+bbox.max)*0.5f;

	for( i=0;i<8;i++ )
		nodes[i]=new flyOcTreeNode;
	
	nodes[0]->bbox.min.vec(bbox.min.x,bbox.min.y,bbox.min.z);
	nodes[0]->bbox.max.vec(center.x,center.y,center.z);

	nodes[1]->bbox.min.vec(center.x,center.y,bbox.min.z);
	nodes[1]->bbox.max.vec(bbox.max.x,bbox.max.y,center.z);

	nodes[2]->bbox.min.vec(center.x,bbox.min.y,bbox.min.z);
	nodes[2]->bbox.max.vec(bbox.max.x,center.y,center.z);
	
	nodes[3]->bbox.min.vec(bbox.min.x,center.y,bbox.min.z);
	nodes[3]->bbox.max.vec(center.x,bbox.max.y,center.z);

	nodes[4]->bbox.min.vec(bbox.min.x,bbox.min.y,center.z);
	nodes[4]->bbox.max.vec(center.x,center.y,bbox.max.z);

	nodes[5]->bbox.min.vec(center.x,center.y,center.z);
	nodes[5]->bbox.max.vec(bbox.max.x,bbox.max.y,bbox.max.z);

	nodes[6]->bbox.min.vec(center.x,bbox.min.y,center.z);
	nodes[6]->bbox.max.vec(bbox.max.x,center.y,bbox.max.z);
	
	nodes[7]->bbox.min.vec(bbox.min.x,center.y,center.z);
	nodes[7]->bbox.max.vec(center.x,bbox.max.y,bbox.max.z);

	int v;
	flyBoundBox bb;
	for( i=0;i<faces.num;i++ )
	{
		v=faces[i]*3;
		bb.reset();
		bb.add_point(verts[triverts[v]]);
		bb.add_point(verts[triverts[v+1]]);
		bb.add_point(verts[triverts[v+2]]);
		for( j=0;j<8;j++ )
			if (nodes[j]->bbox.clip_bbox(bb.min,bb.max))
				nodes[j]->faces.add(faces[i]);
	}
	for( i=0;i<8;i++ )
		if (nodes[i]->faces.num>faces.num*3/5)
			break;

	if (i<8)
	{
		for( i=0;i<8;i++ )
		{
			delete nodes[i];
			nodes[i]=0;
		}
	}
	else
	{
		faces.free();

		for( i=0;i<8;i++ )
			if (nodes[i]->faces.num==0)
			{
				delete nodes[i];
				nodes[i]=0;
			}
			else
				if (nodes[i]->faces.num>FLY_OCTREE_MINFACES)
					nodes[i]->build_node(triverts,verts);
	}
}

// flyOcTree

flyOcTree::flyOcTree()
{
	root=0;
	face=0;
}

flyOcTree::~flyOcTree()
{
	reset();
}

flyOcTree::flyOcTree(flyOcTree& in)
{
	if (in.root==0)
		root=0;
	else
		root=new flyOcTreeNode(*root);
	face=in.face;
}

void flyOcTree::operator=(flyOcTree& in)
{
	if (in.root==0)
		root=0;
	else
		root=new flyOcTreeNode(*root);
	face=in.face;
}

void flyOcTree::reset()
{
	delete root;
	root=0;
	face=0;
}

void flyOcTree::build_tree(flyFace *f)
{
	face=f;
	root=new flyOcTreeNode;

	for( int i=0;i<f->ntriface;i++ )
		root->faces.add(i);
	root->bbox=f->bbox;
	
	root->build_node(f->trivert,f->vert);
}

int flyOcTree::ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const
{
	static flyOcTreeNode *stack[64];
	static float f1,f2;
	if (root->bbox.ray_intersect(ro,rd,f1,f2)==-1)
		return 0;
	
	flyOcTreeNode *n;
	int nstack=1,i;
	stack[0]=root;

	while(nstack)
	{
		n=stack[--nstack];
		if (n->faces.num==0)
		{
			for( i=0;i<8;i++ )
				if (n->nodes[i] && 
					n->nodes[i]->bbox.ray_intersect(ro,rd,f1,f2)!=-1)
					stack[nstack++]=n->nodes[i];
		}
		else
			if (face->ray_intersect_tri_test(n->faces.buf,n->faces.num,ro,rd,dist))
				return 1;
	}
	
	return 0;
}

int flyOcTree::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const
{
	static flyOcTreeNode *stack[64];
	static float f1,f2;
	if (root->bbox.ray_intersect(ro,rd,f1,f2)==-1)
		return -1;
	
	flyOcTreeNode *n;
	int nstack=1;
	stack[0]=root;
	
	int i,min_face=-1;
	float min_dist=FLY_BIG;
	flyVector min_ip;

	while(nstack)
	{
		n=stack[--nstack];
		if (n->faces.num==0)
		{
			for( i=0;i<8;i++ )
				if (n->nodes[i] && 
					n->nodes[i]->bbox.ray_intersect(ro,rd,f1,f2)!=-1)
					stack[nstack++]=n->nodes[i];
		}
		else
		{
			i=face->ray_intersect_tri(n->faces.buf,n->faces.num,ro,rd,ip,dist);
			if (i!=-1 && dist<min_dist)
			{
				min_dist=dist;
				min_ip=ip;
				min_face=n->faces[i];
			}
		}
	}
	
	ip=min_ip;
	dist=min_dist;
	return min_face;
}

void flyOcTree::clip_bbox(const flyBoundBox& bbox,flyArray<int>& faces) const
{
	static flyOcTreeNode *stack[64];

	faces.clear();

	if (bbox.clip_bbox(root->bbox.min,root->bbox.max)==0)
		return;

	flyOcTreeNode *n;
	int nstack=1,i;
	stack[0]=root;

	while(nstack)
	{
		n=stack[--nstack];
		if (n->faces.num==0)
		{
			for( i=0;i<8;i++ )
				if (n->nodes[i] && 
					bbox.clip_bbox(n->nodes[i]->bbox.min,n->nodes[i]->bbox.max))
					stack[nstack++]=n->nodes[i];
		}
		else
			faces+=n->faces;
	}
}

void flyOcTree::draw()
{
	static flyOcTreeNode *stack[64];

	flyOcTreeNode *n;
	int nstack=1,i;
	stack[0]=root;
	
	root->bbox.draw();

	while(nstack)
	{
		n=stack[--nstack];
		if (n->faces.num==0)
		{
			for( i=0;i<8;i++ )
				if (n->nodes[i])
					stack[nstack++]=n->nodes[i];
		}
		else
			n->bbox.draw();
	}
}