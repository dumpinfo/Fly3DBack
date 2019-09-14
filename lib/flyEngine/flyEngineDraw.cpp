#include "..\Fly3D.h"

int flyEngine::draw_frame_subpic(flyPicture& p,int x,int y,int sizex,int sizey,flyBspObject *camera,float camangle,float aspect)
{
	if (g_flyrender==0 || 
		g_flytexcache==0 || 
		g_flytexcache->ntex==0 ||
		p.buf==0 ||
		p.sx>flyRender::s_screensizex ||
		p.sy>flyRender::s_screensizey)
		return 0;

	picrender=1;

	flyBspObject *last_cam=cam;
	float last_aspect=flyRender::s_aspect;
	float last_camangle=flyRender::s_camangle;

	cam=camera;
	flyRender::s_camangle=camangle;
	flyRender::s_aspect=aspect;

	glViewport(0,0,p.sx,p.sy);
	glScissor(x,y,sizex,sizey);
	glEnable(GL_SCISSOR_TEST);

	cur_frame_base=++cur_frame;

	g_flyrender->begin_draw();

	if (cam && player && bsp)
		dll.send_message(FLY_MESSAGE_DRAWSCENE,0,0);

	glReadPixels(x,y,sizex,sizey,GL_RGB,GL_UNSIGNED_BYTE,p.buf);

	cam=last_cam;
	flyRender::s_camangle=last_camangle;
	flyRender::s_aspect=last_aspect;

	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);
	picrender=0;

	return 1;
}

int flyEngine::draw_frame_pic(flyPicture& p,flyBspObject *camera,float camangle,float aspect)
{
	if (g_flyrender==0 || 
		g_flytexcache==0 || 
		g_flytexcache->ntex==0 ||
		p.buf==0 ||
		p.sx>flyRender::s_screensizex ||
		p.sy>flyRender::s_screensizey)
		return 0;

	picrender=1;

	flyBspObject *last_cam=cam;
	float last_aspect=flyRender::s_aspect;
	float last_camangle=flyRender::s_camangle;

	cam=camera;
	flyRender::s_camangle=camangle;
	flyRender::s_aspect=aspect;

	glViewport(0,0,p.sx,p.sy);

	cur_frame_base=++cur_frame;

	g_flyrender->begin_draw();

	if (cam && player && bsp)
		dll.send_message(FLY_MESSAGE_DRAWSCENE,0,0);

	glReadPixels(0,0,p.sx,p.sy,GL_RGB,GL_UNSIGNED_BYTE,p.buf);

	cam=last_cam;
	flyRender::s_camangle=last_camangle;
	flyRender::s_aspect=last_aspect;

	glViewport(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);
	picrender=0;

	return 1;
}

int flyEngine::draw_frame_subtex(int tex,int x,int y,int sizex,int sizey,flyBspObject *camera,float camangle,float aspect)
{
	if (g_flyrender==0 || 
		g_flytexcache==0 || 
		g_flytexcache->ntex==0 ||
		tex==-1 ||
		g_flytexcache->texinfo[tex].sx>flyRender::s_screensizex ||
		g_flytexcache->texinfo[tex].sy>flyRender::s_screensizey)
		return 0;

	picrender=2;

	flyBspObject *last_cam=cam;
	float last_aspect=flyRender::s_aspect;
	float last_camangle=flyRender::s_camangle;

	cam=camera;
	flyRender::s_camangle=camangle;
	flyRender::s_aspect=aspect;

	glViewport(0,0,g_flytexcache->texinfo[tex].sx,g_flytexcache->texinfo[tex].sy);
	glScissor(x,y,sizex,sizey);
	glEnable(GL_SCISSOR_TEST);

	cur_frame_base=++cur_frame;

	g_flyrender->begin_draw();

	if (cam && player && bsp)
		dll.send_message(FLY_MESSAGE_DRAWSCENE,0,0);

	g_flytexcache->sel_tex(tex);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,x,y,x,y,sizex,sizey);

	cam=last_cam;
	flyRender::s_camangle=last_camangle;
	flyRender::s_aspect=last_aspect;

	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);
	picrender=0;

	return 1;
}

int flyEngine::draw_frame_tex(int tex,flyBspObject *camera,float camangle,float aspect)
{
	if (g_flyrender==0 || 
		g_flytexcache==0 || 
		g_flytexcache->ntex==0 ||
		tex==-1 ||
		g_flytexcache->texinfo[tex].sx>flyRender::s_screensizex ||
		g_flytexcache->texinfo[tex].sy>flyRender::s_screensizey)
		return 0;

	picrender=2;

	flyBspObject *last_cam=cam;
	float last_aspect=flyRender::s_aspect;
	float last_camangle=flyRender::s_camangle;

	cam=camera;
	flyRender::s_camangle=camangle;
	flyRender::s_aspect=aspect;

	glViewport(0,0,g_flytexcache->texinfo[tex].sx,g_flytexcache->texinfo[tex].sy);

	cur_frame_base=++cur_frame;

	g_flyrender->begin_draw();

	if (cam && player && bsp)
		dll.send_message(FLY_MESSAGE_DRAWSCENE,0,0);

	g_flytexcache->sel_tex(tex);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,g_flytexcache->texinfo[tex].sx,g_flytexcache->texinfo[tex].sy);

	cam=last_cam;
	flyRender::s_camangle=last_camangle;
	flyRender::s_aspect=last_aspect;

	glViewport(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);
	picrender=0;

	return 1;
}

void flyEngine::draw_frame()
{
	if (g_flyrender==0 || 
		g_flytexcache==0 || 
		g_flytexcache->ntex==0)
		return ;

	cur_frame_base=++cur_frame;

	g_flyrender->begin_draw();

	if (cam && player && bsp)
		dll.send_message(FLY_MESSAGE_DRAWSCENE,0,0);

	g_flyrender->begin_draw2d();

	if (crosshairpic!=-1)//&& cam==player)
	{
		int i,j;
		i=flyRender::s_screensizex>>1;
		j=flyRender::s_screensizey>>1;
		g_flytexcache->sel_tex(crosshairpic);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex2i(i-crosshairsize,j-crosshairsize);
			glTexCoord2f(1,0);
			glVertex2i(i+crosshairsize,j-crosshairsize);
			glTexCoord2f(1,1);
			glVertex2i(i+crosshairsize,j+crosshairsize);
			glTexCoord2f(0,1);
			glVertex2i(i-crosshairsize,j+crosshairsize);
		glEnd();
	}

	if (flyRender::s_stencil)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL,0,~0);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);

		glBlendFunc(GL_DST_COLOR,0);
		glColor3fv(&shadowcolor.x);
		g_flytexcache->sel_tex(-1);
		glRecti(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);

		glDisable(GL_STENCIL_TEST);
	}
	
	if (status)
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(255,255,255,255);

		if (cam && player && bsp)
		{
			if (cur_time - status_msg_time < 2000)
				g_flyrender->draw_text_center(flyRender::s_screensizex/2, 0, status_msg );

			dll.send_message(FLY_MESSAGE_DRAWTEXT,0,0);
		}
		
		glColor4ub(255,255,255,255);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		if (FLY_FPBITS(filter.x)!=0 || FLY_FPBITS(filter.y)!=0 || FLY_FPBITS(filter.z)!=0)
		{
			g_flytexcache->sel_tex(-1);
			glBlendFunc(GL_ONE, GL_ONE);
			glColor3fv(&filter.x);
			glRecti(0, 0, flyRender::s_screensizex, flyRender::s_screensizey);
		}
	}
	
	glColor4ub(255,255,255,255);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ub(255,255,255,255);
	if (con.mode)
		con.draw();

	g_flyrender->end_draw2d();

	g_flyrender->end_draw();

	if (debug)
	{
	int i;
	while(i=glGetError())
		console_printf("glError: %s",gluErrorString(i));
	}
}

void draw_negative_nodes(flyBspNode *node,int side,flyBspNode *parentnode)
{
	if (node->child[0])
		draw_negative_nodes(node->child[0],0,node);
	if (node->child[1])
		draw_negative_nodes(node->child[1],1,node);
	int i,j;
	for( i=0;i<node->elem.num;i++ )
		if (node->elem[i]->type==FLY_TYPE_STATICMESH)
			break;
	if (i<node->elem.num && side==1)
	{
		flyStaticMesh *m=(flyStaticMesh *)node->elem[i];
		glColor4f(1,0,0,1);
		glDisable(GL_DEPTH_TEST);
		glPointSize(4);
//		glDepthFunc(GL_EQUAL);
		m->objmesh->draw_plain();
		glColor4f(0,1,0,1);
		glBegin(GL_POINTS);
		for( i=0;i<m->objmesh->nf;i++ )
			for( j=0;j<m->objmesh->faces[i]->nvert;j++ )
				if (parentnode->distance(m->objmesh->faces[i]->vert[j])<-0.1f)
					glVertex3fv(&m->objmesh->faces[i]->vert[j].x);
		glEnd();
		glPointSize(1);
//		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
	}
}

void flyEngine::draw_bsp()
{
	int i,j,n,e,ne;

	for( i=0;i<FLY_MAX_OBJDRAWLAYERS;i++ )
		objdraw[i].clear();
	nnodedraw=0;
	
	facedraw.clear();
	facedrawtransp.clear();

	flyMesh *m;
	flyShader *s;
	flyBspObject *elem;

	if (nodeonly)
	{
		selnodes.clear();
		if (cam->clipnodes.num)
			selnodes.add(cam->clipnodes[0]);
	}
	else 
		recurse_bsp(view.verts,5,-1,cam->clipnodes.num?cam->clipnodes[0]->leaf:-1);
	for( n=0;n<selnodes.num;n++ )
	{
		ne=selnodes[n]->elem.num;
		for( e=0;e<ne;e++ )
		{
			elem=selnodes[n]->elem[e];
			if (elem->type==FLY_TYPE_STATICMESH)
			{
				m=((flyStaticMesh *)elem)->objmesh;
				m->lastdraw=cur_frame;
				register flyFace *f;
				for( i=0;i<m->nf;i++ )
				{
					f=m->faces[i];
					if (f->lastbsprecurse!=cur_bsprecurse)
					{
						if ((shaders[f->sh]->flags&FLY_SHADER_SKY)==0)
							{
							s=shaders[f->sh];
							if (f->facetype==FLY_FACETYPE_LARGE_POLYGON)
								if((s->flags&FLY_SHADER_NOCULL)==0 &&
									f->distance(cam->pos)<0)
									continue;
//								else ;
//							else
								if (view.clip_bbox(f->bbox)==0)
									continue;

							if (f->facetype==FLY_FACETYPE_BEZIER_PATCH)
								f->patch->set_detail(f->pivot,cam->pos);
								
							if ((s->flags&FLY_SHADER_TRANSPARENT)==0)
								facedraw.add(f);
							else 
								facedrawtransp.add(f);
							}
						f->lastbsprecurse=cur_bsprecurse;
					}
				}
			}
			else 
			if (elem->lastbsprecurse!=cur_bsprecurse)
				{
				elem->lastbsprecurse=cur_bsprecurse;
				if (view.clip_bbox(flyBoundBox(elem->pos+elem->bbox.min,elem->pos+elem->bbox.max)))
					{
					elem->lastdraw=cur_frame;
					objdraw[elem->latedraw&7].add(elem);
					}
				}
		}
	}

	nnodedraw=selnodes.num;
	ntotfacedraw=facedraw.num+facedrawtransp.num;
	ntotelemdraw=0;

	if (facedraw.num)
		draw_faces(facedraw.num,facedraw.buf,vert);

	for( i=0;i<FLY_MAX_OBJDRAWLAYERS;i++ )
	{
		ntotelemdraw+=objdraw[i].num;
		for( j=0;j<objdraw[i].num;j++ )
			objdraw[i][j]->draw();
	}

	if (facedrawtransp.num)
		draw_faces(facedrawtransp.num,facedrawtransp.buf,vert);

	if (debug)
	{
	if (debug&1)
		{
		glColor4ub(192,192,192,255);
		for( i=0;i<FLY_MAX_OBJDRAWLAYERS;i++ )
			for( j=0;j<objdraw[i].num;j++ )
			{
				glPushMatrix();
				glTranslatef(objdraw[i][j]->pos.x,objdraw[i][j]->pos.y,objdraw[i][j]->pos.z);
				objdraw[i][j]->bbox.draw();
				glPopMatrix();
			}
		}
	if (debug&2)
		{
		glColor4ub(128,128,128,255);
		for( i=0;i<facedraw.num;i++ )
			if (facedraw[i]->facetype!=FLY_FACETYPE_LARGE_POLYGON)
				if (facedraw[i]->octree&&g_flyengine->octree)
					facedraw[i]->octree->draw();
				else
					facedraw[i]->bbox.draw();
		}

	if (debug&4)
		if(cam->clipnodes.num)
		{
		g_flytexcache->sel_tex(-1);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		for(i=0;i<cam->clipnodes[0]->portals.num;i++)
			{
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glPolygonMode(GL_FRONT,GL_LINE);
			cam->clipnodes[0]->portals[i].draw();
			glColor4f(1.0f,1.0f,1.0f,0.25f);
			glPolygonMode(GL_FRONT,GL_FILL);
			cam->clipnodes[0]->portals[i].draw();
			}
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glDepthMask(GL_TRUE);
		}

	if (debug&8)
		draw_negative_nodes(bsp,0,0);
	}

	if (flyRender::s_stencil)
	{
	g_flytexcache->sel_tex(-1);
	glColor4ub(255,255,255,255);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,~0);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	for( i=0;i<FLY_MAX_OBJDRAWLAYERS;i++ )
		{
		ntotelemdraw+=objdraw[i].num;
		for( j=0;j<objdraw[i].num;j++ )
			objdraw[i][j]->draw_shadow();
		}

	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	glDisable(GL_STENCIL_TEST);
	glCullFace(GL_BACK);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthMask(GL_TRUE);
	}
}

void flyEngine::sort_faces(int nfd,flyFace **fd)
{
	int s1,s2;
	int p1,p2;
	int i;
	flyFace *f;

	p1=0;
	p2=nfd-1;
	while(p1<p2)
	{
		s1=fd[p1]->sortkey&0xffff0000;
		s2=fd[p2]->sortkey&0xffff0000;
		if (s1==s2)
			s2=-2;
		for( i=p1+1;i<p2;i++ )
			if ((unsigned)s1==(fd[i]->sortkey&0xffff0000))
			{
				f=fd[i];
				fd[i]=fd[++p1];
				fd[p1]=f;
			}
			else
			if ((unsigned)s2==(fd[i]->sortkey&0xffff0000))
			{
				f=fd[i];
				fd[i--]=fd[--p2];
				fd[p2]=f;
			}
		p1++;
		if (s2!=-2)
			p2--;
	}

	p1=0;
	p2=nfd-1;
	while(p1<p2)
	{
		s1=fd[p1]->sortkey;
		s2=fd[p2]->sortkey;
		if (s1==s2)
			s2=-2;
		for( i=p1+1;i<p2;i++ )
			if (s1==fd[i]->sortkey)
			{
				f=fd[i];
				fd[i]=fd[++p1];
				fd[p1]=f;
			}
			else
			if (s2==fd[i]->sortkey)
			{
				f=fd[i];
				fd[i--]=fd[--p2];
				fd[p2]=f;
			}
		p1++;
		if (s2!=-2)
			p2--;
	}
}

void flyEngine::draw_faces(int nfd,flyFace **fd,flyVertex *v,int sort)
{
	int i,j,t;
	flyShader *s;
	flyFace *f;
	int s1,p1,p2;

	if (sort)
		sort_faces(nfd,fd);

	glVertexPointer(3,GL_FLOAT,sizeof(flyVertex),&v->x);
	glNormalPointer(GL_FLOAT,sizeof(flyVertex),&v->normal.x);
	glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(flyVertex),&v->color);
	glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.x);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	g_flytexcache->clear_tex_state();
	for( i=g_flyrender->m_numtextureunits-1;i>=0;i-- )
	{
		g_flytexcache->sel_unit(i);
		glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.x);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	p1=0;
	while( p1<nfd )
	{
		s1=fd[p1]->sh;
		s=shaders[s1];
		j=0;
		while( j<s->npass )
			{
			p2=p1;
			t=s->set_state(j);
			if (t==-1)
				break;
			if (t==0)
				while(p2<nfd && s1==fd[p2]->sh)
				{
					f=fd[p2++];
					switch(f->facetype)
					{
					case FLY_FACETYPE_LARGE_POLYGON:
						glDrawArrays(GL_POLYGON,f->vertindx,f->nvert);
						break;
					case FLY_FACETYPE_BEZIER_PATCH:
						f->patch->draw(t);
						break;
					case FLY_FACETYPE_TRIANGLE_MESH:
						glDrawElements(GL_TRIANGLES,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
						break;
					case FLY_FACETYPE_TRIANGLE_STRIP:
						glDrawElements(GL_TRIANGLE_STRIP,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
						break;
					case FLY_FACETYPE_TRIANGLE_FAN:
						glDrawElements(GL_TRIANGLE_FAN,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
						break;
					}
				}
			else
				{
				glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.z);
				while(p2<nfd && s1==fd[p2]->sh)
					{
					f=fd[p2++];
					g_flytexcache->sel_tex(f->lm!=-1?lm[f->lm]->pic+lmbase:-1);
					switch(f->facetype)
						{
						case FLY_FACETYPE_LARGE_POLYGON:
							glDrawArrays(GL_POLYGON,f->vertindx,f->nvert);
							break;
						case FLY_FACETYPE_BEZIER_PATCH:
							f->patch->draw(t);
							break;
						case FLY_FACETYPE_TRIANGLE_MESH:
							glDrawElements(GL_TRIANGLES,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						case FLY_FACETYPE_TRIANGLE_STRIP:
							glDrawElements(GL_TRIANGLE_STRIP,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						case FLY_FACETYPE_TRIANGLE_FAN:
							glDrawElements(GL_TRIANGLE_FAN,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						}
					}
				glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.x);
				}
			j+=s->restore_state();
			}

		if (fogmap && fm.num)
			{
			glDepthMask(GL_FALSE);
			glDepthFunc(GL_EQUAL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.z);
			glColor4ub(255,255,255,255);
			p2=p1;
			while(p2<nfd && s1==fd[p2]->sh)
				{
				f=fd[p2++];
				if (f->lm!=-1 && fm[f->lm]->lastupdate>=cur_step_base)
					{
					g_flytexcache->sel_tex(fm[f->lm]->pic+fmbase);
					switch(f->facetype)
						{
						case FLY_FACETYPE_LARGE_POLYGON:
							glDrawArrays(GL_POLYGON,f->vertindx,f->nvert);
							break;
						case FLY_FACETYPE_BEZIER_PATCH:
							f->patch->draw(t);
							break;
						case FLY_FACETYPE_TRIANGLE_MESH:
							glDrawElements(GL_TRIANGLES,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						case FLY_FACETYPE_TRIANGLE_STRIP:
							glDrawElements(GL_TRIANGLE_STRIP,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						case FLY_FACETYPE_TRIANGLE_FAN:
							glDrawElements(GL_TRIANGLE_FAN,f->ntrivert,GL_UNSIGNED_INT,f->trivert);
							break;
						}
					}
				}
			glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->texcoord.x);
			}
			
		if (s->npass==0 || j<s->npass)
			while(p1<nfd && s1==fd[p1]->sh)
				p1++;
		else 
			p1=p2;
	}

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	g_flytexcache->clear_tex_state();
	glMatrixMode(GL_MODELVIEW);
}
