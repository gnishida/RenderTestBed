/************************************************************************************************
*
*		@desc Shadow code
*
************************************************************************************************/

#include "LC_GLWidget3D_Shadows.h"
#include "LC_GLWidget3D.h"
#include "VBOUtil.h"

#define FAR_DIST 3000.0f//2500.0f

namespace LC {

	int shadowWidth=4096;//2048;
	int shadowHeight=4096;//2048;

	GLuint light_mvpMatrixLoc,light_positionLoc;
	GLuint light_biasMatrixLoc;

	uint FBO;
	uint shadowMap[MAX_SPLITS];

	#define MAX_SPLITS 4
	float split_weight = 0.75f;

	int cur_num_splits=1;

	QMatrix4x4 shad_cpm[MAX_SPLITS];

	GLWidgetSimpleShadow::GLWidgetSimpleShadow(){
		displayDepthTex=false;
		displayDepthTexInit=false;
		fustrumVBO=INT_MAX;
	}//

	GLWidgetSimpleShadow::~GLWidgetSimpleShadow(){
	}//

	// Compute the 8 corner points of the current view frustum
	void updateFrustumPoints(frustum &f, QVector3D &center, QVector3D &view_dir)
	{
		QVector3D up(0.0f, 1.0f, 0.0f);
		QVector3D right = QVector3D::crossProduct(view_dir,up);

		QVector3D fc = center + view_dir*f.fard;
		QVector3D nc = center + view_dir*f.neard;

		right.normalize();
		up = QVector3D::crossProduct(right, view_dir).normalized();

		// these heights and widths are half the heights and widths of
		// the near and far plane rectangles
		float near_height = tan(f.fov/2.0f) * f.neard;
		float near_width = near_height * f.ratio;
		float far_height = tan(f.fov/2.0f) * f.fard;
		float far_width = far_height * f.ratio;

		f.point[0] = nc - up*near_height - right*near_width;
		f.point[1] = nc + up*near_height - right*near_width;
		f.point[2] = nc + up*near_height + right*near_width;
		f.point[3] = nc - up*near_height + right*near_width;

		f.point[4] = fc - up*far_height - right*far_width;
		f.point[5] = fc + up*far_height - right*far_width;
		f.point[6] = fc + up*far_height + right*far_width;
		f.point[7] = fc - up*far_height + right*far_width;
	}//
	
	// updateSplitDist computes the near and far distances for every frustum slice
	// in camera eye space - that is, at what distance does a slice start and end
	void updateSplitDist(frustum f[MAX_SPLITS], float nd, float fd){
		//printf("nd %f fd %f\n",nd,fd);
		float lambda = split_weight;
		float ratio = fd/nd;
		f[0].neard = nd;

		for(int i=1; i<cur_num_splits; i++){
			float si = i / (float)cur_num_splits;

			f[i].neard = lambda*(nd*powf(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
			f[i-1].fard = f[i].neard * 1.005f;
		}
		f[cur_num_splits-1].fard = fd;

		/*for(int i=0; i<cur_num_splits; i++){
			printf("[%d] nd %f fd %f\n",i,f[i].neard,f[i].fard);
		}*/
	}//

	// this function builds a projection matrix for rendering from the shadow's POV.
	// First, it computes the appropriate z-range and sets an orthogonal projection.
	// Then, it translates and scales it, so that it exactly captures the bounding box
	// of the current frustum slice
	float GLWidgetSimpleShadow::applyCropMatrix(frustum &f,LCGLWidget3D* glWidget3D)
	{
		double shad_modelview[16];
		double shad_proj[16];
		double shad_crop[16];
		double shad_mvp[16];
		double maxX = -10000.0f;//1k
		double maxY = -10000.0f;
		double maxZ;
		double minX =  10000.0f;
		double minY =  10000.0f;
		double minZ;

		QVector4D transf;	

		// find the z-range of the current frustum as seen from the light
		// in order to increase precision
		//glGetDoublev(GL_MODELVIEW_MATRIX, shad_modelview);
		//nv_mvp.set_value(shad_modelview);
		//QVector3D light_position=clientMain->mGLWidget_3D->myCam->light_dir.toVector3D();
		light_mvMatrix.setToIdentity();
		//light_mvMatrix.lookAt(QVector3D(0,0,0),(-light_position).normalized(),QVector3D(0,0,1.0f));
		light_mvMatrix.lookAt(QVector3D(0,0,0),(-glWidget3D->getCamera()->light_dir.toVector3D()).normalized(),QVector3D(0,0,1.0f));
		

		// note that only the z-component is need and thus
		// the multiplication can be simplified
		// transf.z = shad_modelview[2] * f.point[0].x + shad_modelview[6] * f.point[0].y + shad_modelview[10] * f.point[0].z + shad_modelview[14];
		QVector4D frusPos=QVector4D(f.point[0].x(),f.point[0].z(),f.point[0].y(),1.0f);

		transf = light_mvMatrix*frusPos;
		minZ = transf.z();
		maxZ = transf.z();
		for(int i=1; i<8; i++){
			QVector4D frusPos=QVector4D(f.point[i].x(),f.point[i].z(),f.point[i].y(),1.0f);
			transf = light_mvMatrix*frusPos;
			if(transf.z() > maxZ) maxZ = transf.z();
			if(transf.z() < minZ) minZ = transf.z();
		}

		//make sure all the bounding poligon belongs to the z
		/*for(int i=0; i<clientMain->mGLWidget_3D->terrainVBO.boundingPolygon.size(); i++){
			transf = light_mvMatrix*clientMain->mGLWidget_3D->terrainVBO.boundingPolygon[i];
			if(transf.z() > maxZ) maxZ = transf.z() ;
		}*/

		printf("Z %f %f\n",minZ,maxZ);

		//glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();
		light_pMatrix.setToIdentity();
		// set the projection matrix with the new z-bounds
		// note the inversion because the light looks at the neg. z axis
		// gluPerspective(LIGHT_FOV, 1.0, maxZ, minZ); // for point lights
		//glOrtho(-1.0, 1.0, -1.0, 1.0, -maxZ, -minZ);
		light_pMatrix.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -maxZ, -minZ);
		//glGetDoublev(GL_PROJECTION_MATRIX, shad_proj);
		//glPushMatrix();
		//glMultMatrixd(shad_modelview);
		//glGetDoublev(GL_PROJECTION_MATRIX, shad_mvp);
		//glPopMatrix();

		// find the extends of the frustum slice as projected in light's homogeneous coordinates
		//nv_mvp.set_value(shad_mvp);
		light_mvpMatrix=light_pMatrix*light_mvMatrix;
		for(int i=0; i<8; i++){
			QVector4D frusPos=QVector4D(f.point[i].x(),f.point[i].z(),f.point[i].y(),1.0f);
			printf("P[%d]: %f %f %f\n",i,frusPos.x(),frusPos.y(),frusPos.z());
			transf = light_mvpMatrix*frusPos;

			transf.setX(transf.x()/transf.w());
			transf.setY( transf.y()/ transf.w());

			if(transf.x() > maxX) maxX = transf.x();
			if(transf.x() < minX) minX = transf.x();
			if(transf.y() > maxY) maxY = transf.y();
			if(transf.y() < minY) minY = transf.y();
		}
		printf("X %f %f Y %f %f\n",minX,maxX,minY,maxY);
		float scaleX = 2.0f/(maxX - minX);
		float scaleY = 2.0f/(maxY - minY);
		float offsetX = -0.5f*(maxX + minX)*scaleX;
		float offsetY = -0.5f*(maxY + minY)*scaleY;
		printf("Scale %f %f Offset %f %f\n",scaleX,scaleY,offsetX,offsetY);

		// apply a crop matrix to modify the projection matrix we got from glOrtho.
		//nv_mvp.make_identity();
		QMatrix4x4 mod_Matrix;
		mod_Matrix.setToIdentity();
		mod_Matrix.scale(scaleX,scaleY,0.0f);
		//light_mvpMatrix.data()[0,0]=scaleX;
		//light_mvpMatrix.data()[1,1]=scaleY;
		mod_Matrix.translate(offsetX,offsetY);
		//light_mvpMatrix.data()[0,3]=offsetX;
		//light_mvpMatrix.data()[1,3]=offsetY;
		//transpose(nv_mvp);
		mod_Matrix=light_mvpMatrix.transposed();
		//nv_mvp.get_value(shad_crop);
		//shad_crop=*mvp.data();
		light_pMatrix=light_pMatrix*mod_Matrix;//!!! which first!!
		//glLoadMatrixd(mvp.data());
		//glMultMatrixd(shad_proj);

		return minZ;
	}//

	

	void GLWidgetSimpleShadow::updateShadowMatrix(LCGLWidget3D* glWidget3D){

		QVector3D light_position=-glWidget3D->getCamera()->light_dir.toVector3D();
		//////////////////////////////////////////////////////
		float fov=glWidget3D->getCamera()->fovy; 
		float aspect=(float)shadowWidth/(float)shadowHeight;

		float zfar=3800.0f;//3300.0f;// 
		float znear=1500.0f;//1200.0f;// FIX

		float f = 1.0f / tan (fov * (M_PI / 360.0));
		double m[16]=
		{	f/aspect,	0,								0,									0,
		0,			f,								0,						 			0,
		0,			0,		(zfar+znear)/(znear-zfar),		(2.0f*zfar*znear)/(znear-zfar),
		0,			0,								-1,									0
		};

		light_pMatrix=QMatrix4x4(m);
		// BIAS MATRIX
		light_biasMatrix.setToIdentity();
		light_biasMatrix.scale(0.5f);
		light_biasMatrix.translate(0.5f,0.5f,0.5f);
		// UNIFORMS LOC
		light_mvpMatrixLoc= glGetUniformLocation(programId, "light_mvpMatrix");
		light_biasMatrixLoc= glGetUniformLocation(programId, "light_biasMatrix");
		light_positionLoc= glGetUniformLocation(programId, "lightPosition");
		//printf("LOC lights %d %d %d\n",light_mvpMatrixLoc,light_biasMatrixLoc,light_positionLoc);

		// UPDATE MATRIX
		light_mvMatrix.setToIdentity();
		light_mvMatrix.lookAt(light_position*2500,//QVector3D(1365.0,-1200.0f,1245.0),
			-light_position.normalized(),//QVector3D(-0.60f,0.55,-0.6),
			QVector3D(0.0f, 0.0f, 1.0f));
		light_mvpMatrix=light_pMatrix*light_mvMatrix;


		///////////////////////////////////////////////////////
		/*
		light_mvMatrix.setToIdentity();
		light_mvMatrix.lookAt(light_position.normalized(),QVector3D(0,0,0),QVector3D(0,0,1.0f));
		

		//light_mvpMatrix=lightBiasMatrix*light_pMatrix*light_mvMatrix;
		*/

		float light_mvpMatrixArray[16];
		float light_biasMatrixArray[16];
		//float light_mvMatrixArray[16];
		//float mvpMatrix[16];
		for(int i=0;i<16;i++){
			light_mvpMatrixArray[i]=light_mvpMatrix.data()[i];
			light_biasMatrixArray[i]=light_biasMatrix.data()[i];
			//light_mvMatrixArray[i]=light_mvMatrix.data()[i];
			//mvpMatrix[i]=clientMain->mGLWidget_3D->mvpMatrix.data()[i];
		}

		glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,light_mvpMatrixArray);
		//glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,mvpMatrix);///!!!!!! WRONG
		
		glUniformMatrix4fv(light_biasMatrixLoc,1,GL_FALSE,light_biasMatrixArray);
		//glUniformMatrix4fv(light_mvMatrixLoc,1,GL_FALSE,light_mvMatrixArray);
		glUniform3f(light_positionLoc,light_position.x(),light_position.y(),light_position.z());
	}//

	void CheckFramebufferStatus()
	{
		int status;
		status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			printf("Unsupported framebuffer format\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			printf("Framebuffer incomplete, missing attachment\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			printf("Framebuffer incomplete, incomplete attachment\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			printf("Framebuffer incomplete, attached images must have same dimensions\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			printf("Framebuffer incomplete, attached images must have same format\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			printf("Framebuffer incomplete, missing draw buffer\n");exit(0);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			printf("Framebuffer incomplete, missing read buffer\n");exit(0);
			break;
		default:
			exit(0);
		}

	}

	void GLWidgetSimpleShadow::initShadow(int _programId,LCGLWidget3D* glWidget3D){
		printf("---------- >> INIT SHADOW\n");
		
		programId=_programId;

		// PROJECTION MATRIX
		float fov=glWidget3D->getCamera()->fovy; 
		float aspect=(float)shadowWidth/(float)shadowHeight;
		//float zfar=FAR_DIST;//!!!!! clientMain->mGLWidget_3D->farPlaneToSpaceRadiusFactor*clientMain->mGLWidget_3D->spaceRadius;
		//float znear=5.0f;
		
		float zfar=3300.0f;// 
		float znear=1200.0f;// FIX

		float f = 1.0f / tan (fov * (M_PI / 360.0));
		double m[16]=
		{	f/aspect,	0,								0,									0,
			0,			f,								0,						 			0,
			0,			0,		(zfar+znear)/(znear-zfar),		(2.0f*zfar*znear)/(znear-zfar),
			0,			0,								-1,									0
		};

		

		light_pMatrix=QMatrix4x4(m);
		// BIAS MATRIX
		light_biasMatrix.setToIdentity();
		light_biasMatrix.scale(0.5f);
		light_biasMatrix.translate(0.5f,0.5f,0.5f);
		// UNIFORMS LOC
		light_mvpMatrixLoc= glGetUniformLocation(programId, "light_mvpMatrix");
		light_biasMatrixLoc= glGetUniformLocation(programId, "light_biasMatrix");
		light_positionLoc= glGetUniformLocation(programId, "lightPosition");
		printf("LOC lights %d %d %d\n",light_mvpMatrixLoc,light_biasMatrixLoc,light_positionLoc);
		
		// UPDATE MATRIX
		QVector3D light_position=glWidget3D->getCamera()->light_dir.toVector3D();
		light_mvMatrix.setToIdentity();
		////// light_mvMatrix.lookAt(-light_position,(QVector3D(0,0,0)-light_position).normalized(),QVector3D(0.0f,0,1.0f));
		
		// FIX
		/*light_mvMatrix.lookAt(QVector3D(1062.0,-900.0f,1158.0),
			QVector3D(-0.53,0.4675,-0.710),
			QVector3D(0.0f, 0.0f, 1.0f));*/
		light_mvMatrix.lookAt(QVector3D(1365.0,-1200.0f,1245.0),
			QVector3D(-0.60f,0.55,-0.6),
			QVector3D(0.0f, 0.0f, 1.0f));
		light_mvpMatrix=light_pMatrix*light_mvMatrix;
		
		updateShadowMatrix(glWidget3D);
		
		// INIT shadowMap
		// FBO
		glGenFramebuffers(1,&FBO);
		glBindFramebuffer(GL_FRAMEBUFFER,FBO);
		//glDrawBuffer(GL_NONE);//no color buffers
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthTexture,0);
		//glBindFramebuffer(GL_FRAMEBUFFER,0);
		//GLenum err;
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "*IS 0 initShader ERROR INIT: OpenGL-->" << err << endl;
		
		//shadowMap=createTexture(shadowWidth,shadowHeight,true);
		////////////////////
		glGenTextures(MAX_SPLITS, &shadowMap[0]);
		for(int i=0;i<1;i++){// !!!! NOTE 1
			//glActiveTexture(GL_TEXTURE1+i);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D,shadowMap[i]);

			glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32F, shadowWidth, shadowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		}

		glActiveTexture(GL_TEXTURE0);
		
		/*glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, depth_tex_ar);
		glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT24, depth_size, depth_size, MAX_SPLITS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/
		////
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "*IS 1 initShader ERROR INIT: OpenGL-->" << err << endl;
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "*IS 1 initShader ERROR INIT: OpenGL-->" << err << endl;
		VBOUtil::check_gl_error("Init ShadowMap");
		glUniform1i(glGetUniformLocation(programId,"shadowMap"), 6);//shadowMap in tex1
		//glUniform1i(glGetUniformLocation(programId,"shadowEnable"), 1);//shadowEnable enable
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		printf("---------- << INIT SHADOW\n");
	}//




	void GLWidgetSimpleShadow::makeShadowMap(LCGLWidget3D* glWidget3D){
	
		glDisable(GL_TEXTURE_2D);

		int origWidth=glWidget3D->width();
		int origHeigh=glWidget3D->height();
		

		// update camera fustrum (for overview)
		updateSplitDist(glWidget3D->getCamera()->f, 1.0f/*FAR_DIST/6*/, FAR_DIST);//FAR_DIST/6 to display
		
		// generate shadow map using drawScene(1)
		glBindFramebuffer(GL_FRAMEBUFFER,FBO);
		glViewport(0,0,shadowWidth,shadowHeight);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		glPolygonOffset( 1.0f, 4096.0f);
		//glPolygonOffset( 1.1f, 4.0f);
		//glPolygonOffset( 0.75f, 0.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		// draw all faces since our terrain is not closed.
		//glDisable(GL_CULL_FACE);
		for(int i=0; i<cur_num_splits; i++){//cur_num_splits
			//clientMain->mGLWidget_3D->myCam->updateFrustumPoints(clientMain->mGLWidget_3D->myCam->f[i], clientMain->mGLWidget_3D->myCam->cam_pos, clientMain->mGLWidget_3D->myCam->cam_view);
			
			//float minZ = applyCropMatrix(clientMain->mGLWidget_3D->myCam->f[i]);

			updateShadowMatrix(glWidget3D);
		
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap[i], 0);
			//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap[i], 0);
			glDrawBuffer(GL_NONE); // No color buffer is drawn to.
			CheckFramebufferStatus();
			glClear(GL_DEPTH_BUFFER_BIT);
			float light_mvpMatrixArray[16];
			for(int j=0;j<16;j++){
				light_mvpMatrixArray[j]=light_mvpMatrix.data()[j];
			}
			glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,light_mvpMatrixArray);
			//RENDER
			glWidget3D->drawScene(1);//1 light mode1
			
			// save camera for later
			shad_cpm[i]=light_mvpMatrix;
		}

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		// revert to normal back face culling as used for rendering
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glViewport(0,0,origWidth,origHeigh);

	}//



	void GLWidgetSimpleShadow::showDepthTex(){
		/*if(displayDepthTex==false){
			return;
		}
		//GLuint err;
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**1 renderOVerview: OpenGL-->" << err << endl;

		int origWidth=clientMain->mGLWidget_3D->width();
		int origHeigh=clientMain->mGLWidget_3D->height();
	
		GLuint mainProgram=clientMain->mGLWidget_3D->program;//save
		//////////////////////////////////////////////////////
		// TEXTURES
		//int vertex;
		glUseProgram(0);
		if(displayDepthTexInit==false){//initializate
			//////////
			// Init depth tex
			printf("** INIT showDepthTex\n");
			depthTexProgram=Shader::initShader(QString("shaders\\view_vertex.glsl"),QString("shaders\\view_fragment.glsl"));//clientMain->mGLWidget_3D->program;
			//glUseProgram(depthTexProgram);
			RendUtil::useProgram(depthTexProgram);
			float vertPosition[]={
				-1.0f, -1.0f, 0.0f,
				1.0f, -1.0f, 0.0f,
				1.0f,  1.0f, 0.0f,
				-1.0f,  1.0f, 0.0f,
			};
			glGenVertexArrays(1,&depthTexVBO);
			glBindBuffer(GL_ARRAY_BUFFER,depthTexVBO);
			glBufferData(GL_ARRAY_BUFFER,sizeof(vertPosition),vertPosition,GL_STATIC_DRAW);
			//////////
			displayDepthTexInit=true;
		}else{
			//glUseProgram(depthTexProgram);
			RendUtil::useProgram(depthTexProgram);
			glBindBuffer(GL_ARRAY_BUFFER,depthTexVBO);
		}
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**10 renderOVerview: OpenGL-->" << err << endl;
		printf("** showDepthTex Pr %u vbo %u\n",depthTexProgram,depthTexVBO);
		//glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT);
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**11 renderOVerview: OpenGL-->" << err << endl;
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**11 renderOVerview: OpenGL-->" << err << endl;
		//glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(depthTexProgram,"tex"), 1);//shadowMap in tex1
		//vertex=glGetAttribLocation(depthTexProgram,"vertex"); //0

		//glEnableVertexAttribArray(vertex);
		//glVertexAttribPointer(vertex,3,GL_FLOAT,GL_FALSE,sizeof(float)*3,0);

		//printf("Depth Tex %d %d\n",glGetUniformLocation(depthTexProgram,"tex"),vertex);

		glActiveTexture(GL_TEXTURE1);
		//int loc = glGetUniformLocation(depthTexProgram,"layer");
		for(int i=0; i<cur_num_splits; i++)
		{
			glViewport(194*i, 0, 192, 192);
			//glViewport(0,0,clientMain->mGLWidget_3D->width()/2.0f,clientMain->mGLWidget_3D->height()/2.0f);
			glBindTexture(GL_TEXTURE_2D, shadowMap[i]);
			//glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			//glUniform1f(loc, (float)i);
			glDrawArrays(GL_QUADS,0,4);//just one quad
		}
		//glDisableVertexAttribArray(vertex);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glActiveTexture(GL_TEXTURE0);
		//restore
		//glUseProgram(0);
		clientMain->mGLWidget_3D->program=mainProgram;
		//glUseProgram(mainProgram);
		RendUtil::useProgram(mainProgram);
		//////////////////////////////////////////////////////
		// LIGHT PERSPECTIVE RENDERING
		glDisable(GL_CULL_FACE);
		for(int i=0; i<cur_num_splits; i++)
		{
			//glViewport(194*i, 194, 192, 192);
			glViewport(194*i, 194, 192, 192);

			glClear(GL_DEPTH_BUFFER_BIT);
			//set matrix
			float light_mvpMatrixArray[16];
			for(int j=0;j<16;j++){
				light_mvpMatrixArray[j]=shad_cpm[i].data()[j];
			}

			glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,light_mvpMatrixArray);

			clientMain->mGLWidget_3D->drawScene(1);//1 light mode1
		}
		glEnable(GL_CULL_FACE);
		/////////////////////////////////////////
		// RENDER OVERVIEW
		//glViewport(clientMain->mGLWidget_3D->width()- 129, 0, 128, 128);
		//glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(mainProgram,"tex"), 0);
		glViewport(clientMain->mGLWidget_3D->width()- 193, 0, 192, 192);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		QMatrix4x4 mvpOverviewMatrix;
		mvpOverviewMatrix.setToIdentity();
		mvpOverviewMatrix.lookAt(QVector3D(0, 0, FAR_DIST/2.0f),QVector3D(0, 0, 0),QVector3D(0, 1.0f, 0.0));
		//mvpOverviewMatrix.scale(0.16);
		//mvpOverviewMatrix.scale(0.2f);
		mvpOverviewMatrix.scale(0.4f);
		mvpOverviewMatrix.rotate(20.0f,1.0f,0,0);
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**12 renderOVerview: OpenGL-->" << err << endl;
		mvpOverviewMatrix=clientMain->mGLWidget_3D->pMatrix*mvpOverviewMatrix;
		float mvpOverMatrixArray[16];
		for(int i=0;i<16;i++){
			mvpOverMatrixArray[i]=mvpOverviewMatrix.data()[i];
		}
		glUniformMatrix4fv(clientMain->mGLWidget_3D->mvpMatrixLoc,  1, false, mvpOverMatrixArray);
		//render terrain no lights
		//clientMain->mGLWidget_3D->terrainVBO.renderTerrain();
		//clientMain->mGLWidget_3D->roadGraphVBO.renderGroundVBO(mainProgram,3);
		bool currVegState=LC::misctools::Global::global()->view_trees_render;
		LC::misctools::Global::global()->view_trees_render=false;

		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**13 renderOVerview: OpenGL-->" << err << endl;
		clientMain->mGLWidget_3D->drawScene(2);//top

		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**14 renderOVerview: OpenGL-->" << err << endl;
		LC::misctools::Global::global()->view_trees_render=currVegState;//restore
		//LC::VMaker::v()->renderVBO(mainProgram,1);
		////////////////////////
		// create fustrum lines
		std::vector<Vertex> line_Vert;
		QVector3D col1(1.0f,0.0f,0.0f);
		//QVector3D col2(0.3f,0.0f,0.0f);
		QVector3D col2(0.0f,0.0f,0.0f);

		//clientMain->mGLWidget_3D->myCam->
		QVector3D pos1,pos2;
		for(int i=0; i<cur_num_splits; i++){
			//clientMain->mGLWidget_3D->myCam->updateFrustumPoints(clientMain->mGLWidget_3D->myCam->f[i], clientMain->mGLWidget_3D->myCam->cam_pos, clientMain->mGLWidget_3D->myCam->cam_view);
			//glBegin(GL_LINE_LOOP);
			for(int j=0; j<4; j++){
				//pos1=QVector3D
				int ind1=j;
				int ind2=(j+1)%4;
				pos1=clientMain->mGLWidget_3D->myCam->f[i].point[ind1];
				pos2=clientMain->mGLWidget_3D->myCam->f[i].point[ind2];
				//glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);

				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos1.x(),pos1.z(),pos1.y()),col1));
				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos2.x(),pos2.z(),pos2.y()),col1));
			}
			//glEnd();
			//glBegin(GL_LINE_LOOP);
			for(int j=4; j<8; j++){
				int ind1=j;
				int ind2=j+1;
				if(ind2>=8)ind2=ind2-4;
				printf("ind %d %d\n",ind1,ind2);
				pos1=clientMain->mGLWidget_3D->myCam->f[i].point[ind1];
				pos2=clientMain->mGLWidget_3D->myCam->f[i].point[ind2];
				//glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);
				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos1.x(),pos1.z(),pos1.y()),col2));
				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos2.x(),pos2.z(),pos2.y()),col2));
				//glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);
			}
			//glEnd();
		}
		for(int j=0; j<4; j++){
			//glBegin(GL_LINE_STRIP);
			pos1=clientMain->mGLWidget_3D->myCam->cam_pos;
			pos2=clientMain->mGLWidget_3D->myCam->f[0].point[j];
			line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos1.x(),pos1.z(),pos1.y()),col1));
			line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos2.x(),pos2.z(),pos2.y()),col1));
			//glVertex3fv(cam_pos);
			for(int i=0; i<cur_num_splits-1; i++){
				pos1=clientMain->mGLWidget_3D->myCam->f[i].point[j];
				pos2=clientMain->mGLWidget_3D->myCam->f[i+1].point[j];
				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos1.x(),pos1.z(),pos1.y()),col1));
				line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos2.x(),pos2.z(),pos2.y()),col1));
			}
			
			pos1=clientMain->mGLWidget_3D->myCam->f[cur_num_splits-1].point[j];
			pos2=clientMain->mGLWidget_3D->myCam->f[cur_num_splits-1].point[j+4];
			line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos1.x(),pos1.z(),pos1.y()),col1));
			line_Vert.push_back(RendUtil::makeLineVert(QVector3D(pos2.x(),pos2.z(),pos2.y()),col2));

			//glVertex3f(f[cur_num_splits-1].point[j+4].x, f[cur_num_splits-1].point[j+4].y, f[cur_num_splits-1].point[j+4].z);
			//glEnd();
		}

		

		//render fustrum lines
		glLineWidth(2.0f);

		//

		if(fustrumVBO!=INT_MAX){//it was already created--> remove
			glDeleteVertexArrays(1, &fustrumVBO);
		}
		glGenVertexArrays(1,&fustrumVBO);
		glBindBuffer(GL_ARRAY_BUFFER,fustrumVBO);
		glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*line_Vert.size(),line_Vert.data(),GL_STATIC_DRAW);



		//vertex=glGetAttribLocation(mainProgram,"vertex"); //0
		//int color=glGetAttribLocation(mainProgram,"color"); //1
		
		glUniform1i (glGetUniformLocation (mainProgram, "mode"), 1);//color

		//glEnableVertexAttribArray(vertex);
		//glVertexAttribPointer(vertex,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
		//glEnableVertexAttribArray(color);
		//glVertexAttribPointer(color,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));

		glDrawArrays(GL_LINES,0,line_Vert.size());

		//glDisableVertexAttribArray(vertex);
		//glDisableVertexAttribArray(color);

		glBindBuffer(GL_ARRAY_BUFFER,0);

		///
		glEnable(GL_CULL_FACE);

		glViewport(0,0,origWidth,origHeigh);

		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**2 renderOVerview: OpenGL-->" << err << endl;
		RendUtil::check_gl_error("ShowShadowMap");*/
	}//
	

	void GLWidgetSimpleShadow::shadowRenderScene(int program){
		GLuint mainProgram=program;//save
		//glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,&shadowMatrix[0][0]);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, shadowMap[0]);//groundTex);//1);
		glUniform1i(glGetUniformLocation(mainProgram,"shadowMap"), 6);//shadowMap in tex1
		printf("------** showMap %d\n",glGetUniformLocation(mainProgram,"shadowMap"));
		glActiveTexture(GL_TEXTURE0);
		
		// L
		float light_mvpMatrixArray[16];
		for(int j=0;j<16;j++){
			light_mvpMatrixArray[j]=shad_cpm[0].data()[j];
		}

		glUniformMatrix4fv(light_mvpMatrixLoc,1,GL_FALSE,light_mvpMatrixArray);

	}//

	/*void GLWidgetSimpleShadow::setEnableShadowMap(bool enableDisable){
		if(enableDisable)
			glUniform1i(glGetUniformLocation(programId,"shadowEnable"), 1);//shadowEnable enable
		else
			glUniform1i(glGetUniformLocation(programId,"shadowEnable"), 0);//shadowMap out
	}//*/
	

	
}//