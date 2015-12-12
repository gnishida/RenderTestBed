#include "VBORenderManager.h"

namespace LC {


	VBORenderManager::VBORenderManager(){
		//ssao
		uKernelSize = 16;
		uRadius = 17.0f;
		uPower = 2.0f;
	}

	VBORenderManager::~VBORenderManager() {
		Shader::cleanShaders();
		//delete
		glDeleteVertexArrays(1,&secondPassVBO);
		glDeleteVertexArrays(1,&secondPassVAO);
		// remove all geometry
		// TODO
	}

	void VBORenderManager::init(){
		// init program shader
		// PASS 1
		printf("PASS 1\n");
		fragDataNamesP1.push_back("def_diffuse");
		fragDataNamesP1.push_back("def_normal");
		fragDataNamesP1.push_back("def_originPos");
		program_pass1=Shader::initShader(QString("lc_vert_pass1.glsl"),QString("lc_frag_pass1.glsl"),fragDataNamesP1);
		// PASS 2
		printf("PASS 2\n");
		std::vector<QString> fragDataNamesP2;
		fragDataNamesP2.push_back("def_AO");
		program_pass2=Shader::initShader(QString("lc_vert_pass2.glsl"),QString("lc_frag_pass2.glsl"),fragDataNamesP2);
		// PASS 3
		printf("PASS 3\n");
		std::vector<QString> fragDataNamesP3;//default
		fragDataNamesP3.push_back("outputF");
		program_pass3 = Shader::initShader(QString("lc_vert_pass3.glsl"), QString("lc_frag_pass3.glsl"), fragDataNamesP3);

		glUseProgram(program_pass1);


		nameToTexId[""]=0;
		//////////////////////////////////////////////
		// INIT SECOND PASS
		// Quad
		std::vector<Vertex> vert(4);
		vert[0]=Vertex(glm::vec3(-1,-1,0),glm::vec3(0,0,0),glm::vec4(0,0,0,1), glm::vec2(0,0));
		vert[1]=Vertex(glm::vec3(1,-1,0),glm::vec3(0,0,0),glm::vec4(0,0,0,1), glm::vec2(1,0));
		vert[2]=Vertex(glm::vec3(1,1,0),glm::vec3(0,0,0),glm::vec4(0,0,0,1), glm::vec2(1,1));
		vert[3]=Vertex(glm::vec3(-1,1,0),glm::vec3(0,0,0),glm::vec4(0,0,0,1), glm::vec2(0,1));

		glGenVertexArrays(1,&secondPassVAO);
		glBindVertexArray(secondPassVAO);
		glGenBuffers(1, &secondPassVBO);
		glBindBuffer(GL_ARRAY_BUFFER, secondPassVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vert.size(), vert.data(), GL_STATIC_DRAW);
	
		// Configure the attributes in the VAO.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		/*
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));
		*/

		glBindVertexArray(0);
		// fragm
		fragDataFB=INT_MAX;
		
		printf("++++VBORenderManager %u\n",fragDataFB);
	}//

	void VBORenderManager::resizeFragTex(int winWidth,int winHeight){
		
		if(fragDataTex.size()>0){
			glDeleteTextures(fragDataTex.size(),&fragDataTex[0]);
			glDeleteTextures(1,&fragDepthTex);
			glDeleteTextures(1, &fragNoiseTex);
			fragDataTex.clear();
			glDeleteFramebuffers(1,&fragDataFB);
		}


		// The texture we're going to render to
		fragDataTex.resize(fragDataNamesP1.size());

		// TEX1: Diffuse Texture
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &fragDataTex[0]);
		glBindTexture(GL_TEXTURE_2D, fragDataTex[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, winWidth, winHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//NULL means reserve texture memory, but texels are undefined

		// TEX2: Normal Texture
		glActiveTexture (GL_TEXTURE2);
		glEnable( GL_TEXTURE_2D );
		glGenTextures(1, &fragDataTex[1]);
		glBindTexture(GL_TEXTURE_2D, fragDataTex[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, winWidth, winHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		// TEX3: Original Position Texture
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &fragDataTex[2]);
		glBindTexture(GL_TEXTURE_2D, fragDataTex[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, winWidth, winHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);


		// TEX4: AO Texture
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &fragAOTex);
		glBindTexture(GL_TEXTURE_2D, fragAOTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, winWidth, winHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		/////////////////////////////////////////

		// DEPTH
		glActiveTexture (GL_TEXTURE8);
		glEnable( GL_TEXTURE_2D );
		glGenTextures(1, &fragDepthTex);
		glBindTexture(GL_TEXTURE_2D, fragDepthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, winWidth, winHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glActiveTexture(GL_TEXTURE9);
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &fragDepthTex_AO);
			glBindTexture(GL_TEXTURE_2D, fragDepthTex_AO);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, winWidth, winHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		/////////////////////////////////////////

		// FRAME BUFFER
		fragDataFB = 0;
		glGenFramebuffers(1, &fragDataFB);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fragDataTex[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fragDataTex[1], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fragDataTex[2], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fragDepthTex, 0);
		// Set the list of draw buffers.
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, DrawBuffers); // "3" is the size of DrawBuffers

		// Always check that our framebuffer is ok
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			printf("+1ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// FRAME BUFFER AO
		fragDataFB_AO = 0;
		glGenFramebuffers(1, &fragDataFB_AO);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fragAOTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fragDepthTex, 0);
		// Set the list of draw buffers.
		GLenum DrawBuffers_AO[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers_AO); // "3" is the size of DrawBuffers

		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("+2ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//////////////////////////////////////////////////////////////////////////////////
		// Noise
		fragNoiseTex = 0;
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &fragNoiseTex);
		glBindTexture(GL_TEXTURE_2D, fragNoiseTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		qsrand(5648943215);
		
		int sizeX = winWidth;
		int sizeZ = winHeight;

		std::vector<GLfloat> data(sizeX*sizeZ*3);
		for (int c = 0; c < sizeX*sizeZ*3; c++) {
			if (c % 3 == 0 || c % 3 == 1)
				data[c] = (float(qrand()) / RAND_MAX)*2.0f - 1.0f;
			else
				data[c] = 0.0f;//0 in component z
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, sizeX, sizeZ, 0, GL_RGB, GL_FLOAT, &data[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		
		resizeSsaoKernel();
	}//

	template<typename T>
	T lerp(T v0, T v1, T t) {
		return (1 - t)*v0 + t*v1;
	}

	void VBORenderManager::resizeSsaoKernel() {
		uKernelOffsets.resize(uKernelSize*3);
		qsrand(123456);
		for (int i = 0; i < uKernelSize; ++i) {
			QVector3D kernel = QVector3D((float(qrand()) / RAND_MAX)*2.0f - 1.0f, (float(qrand()) / RAND_MAX)*2.0f - 1.0f, (float(qrand()) / RAND_MAX));
			kernel.normalize();
			float scale = float(i) / uKernelSize;
			kernel *= lerp<float>(0.1f, 1.0f, scale*scale);
			printf("[%d] %f %f %f\n",i,kernel.x(),kernel.y(),kernel.z());
			uKernelOffsets[i * 3 + 0] = kernel.x();
			uKernelOffsets[i * 3 + 1] = kernel.y();
			uKernelOffsets[i * 3 + 2] = kernel.z();
		}
	}//

	void VBORenderManager::renderSecondPass(){
		/*glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reffered
		glActiveTexture (GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_2D, fragDataTex[0]);
		glActiveTexture (GL_TEXTURE2);
		glBindTexture (GL_TEXTURE_2D, fragDataTex[1]);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, fragDataTex[2]);

		// noise
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, fragNoiseTex);
		

		glUniform1i(glGetUniformLocation(program_pass2, ("tex0")), 1);
		glUniform1i(glGetUniformLocation(program_pass2, ("tex1")), 2);
		glUniform1i(glGetUniformLocation(program_pass2, ("tex2")), 3);
		printf("** tex %d %d %d\n", glGetUniformLocation(program_pass2, ("tex0")), glGetUniformLocation(program_pass2, ("tex1")), glGetUniformLocation(program_pass2, ("tex2")));
		glUniform1i(glGetUniformLocation(program_pass2, "noiseTex"), 7);

		glUniform1i(glGetUniformLocation(program_pass2, ("uKernelSize")), uKernelSize);
		glUniform3fv(glGetUniformLocation(program_pass2, ("uKernelOffsets")), uKernelOffsets.size(), (const GLfloat*)uKernelOffsets.data());
		printf("** ukernel %d %d\n", glGetUniformLocation(program_pass2, ("uKernelSize")), glGetUniformLocation(program_pass2, ("uKernelOffsets")));

		glUniform1f(glGetUniformLocation(program_pass2, ("uPower")), uPower);
		glUniform1f(glGetUniformLocation(program_pass2, ("uRadius")), uRadius);




		glBindVertexArray(secondPassVAO);
		glDrawArrays(GL_QUADS,0,4);
		glBindVertexArray(0);*/
	}

	GLuint VBORenderManager::loadTexture(const QString fileName,bool mirrored){
		GLuint texId;
		if(nameToTexId.contains(fileName)){
			texId=nameToTexId[fileName];
		}else{
			texId=VBOUtil::loadImage(fileName,mirrored);
			nameToTexId[fileName]=texId;
		}
		return texId;
	}//

	// ATRIBUTES
	// 0 Vertex
	// 1 Color
	// 2 Normal
	// 3 UV

	// UNIFORMS
	// 0 mode
	// 1 tex0

	bool VBORenderManager::createVAO(std::vector<Vertex>& vert,GLuint& vbo,GLuint& vao,int& numVertex){
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);
		// Crete VBO
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vert.size(), vert.data(), GL_STATIC_DRAW);
	
		// Configure the attributes in the VAO.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

		/*
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));
		*/

		// Bind back to the default state.
		glBindVertexArray(0); 
		glBindBuffer(GL_ARRAY_BUFFER,0);
		
	
		// clean space
		numVertex=vert.size();
		//vert.clear();
		return true;
	}//

	void VBORenderManager::renderVAO(RenderSt& renderSt,bool cleanVertex){
		//printf("renderVAO numVert %d texNum %d vao %d numVertVert %d\n",renderSt.numVertex,renderSt.texNum,renderSt.vao,renderSt.vertices.size());
		// 1. Create if necessary
		if(renderSt.numVertex!=renderSt.vertices.size()&&renderSt.vertices.size()>0){
			if(renderSt.numVertex!=-1){
				cleanVAO(renderSt.vbo,renderSt.vao);
			}
			// generate vao/vbo
			createVAO(renderSt.vertices,renderSt.vbo,renderSt.vao,renderSt.numVertex);
			if(cleanVertex)
				renderSt.vertices.clear();
		}
		// 2. Render
		// 2.1 TEX
		int mode=renderSt.shaderMode;
		if((mode&mode_TexArray)==mode_TexArray){
			// MULTI TEX
			mode=mode&(~mode_TexArray);//remove tex array bit
			glActiveTexture(GL_TEXTURE8);

			glBindTexture(GL_TEXTURE_2D,0); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, renderSt.texNum);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i (glGetUniformLocation (program_pass1, "tex_3D"), 8);
		}else{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, renderSt.texNum);
		}
		// 2.2 mode
		//if(renderSt.texNum==0){
			//glUniform1i (glGetUniformLocation (program, "mode"), 1|(renderSt.shaderMode&0xFF00));//MODE: same modifiers but just color (renderSt.shaderMode&0xFF00)
		//}else{
			glUniform1i (glGetUniformLocation (program_pass1, "mode"), mode);
		//}

		glUniform1i (glGetUniformLocation (program_pass1, "tex0"), 0);//tex0: 0

		glBindVertexArray(renderSt.vao);
		glDrawArrays(renderSt.geometryType,0,renderSt.numVertex);
		glBindVertexArray(0);
	}

	void VBORenderManager::renderAll(bool cleanVertex){
		//vboTerrain.render(editionMode,mousePos3D);
		//vboWater.render(*this);
		
		////////////////////////
		// 1. STATIC
		QHash<QString,renderGrid>::iterator statcIt;
		// 1.1 iterate over geoNames
		for (statcIt = geoName2StaticRender.begin(); statcIt != geoName2StaticRender.end(); ++statcIt){
			printf("Render: %s\n",statcIt.key().toAscii().constData());
			// 1.1.1 iterate over textures
			renderGrid::iterator i;
			for (i = statcIt.value().begin(); i != statcIt.value().end(); ++i){
				renderVAO(i.value(),cleanVertex);
			}
		}

	}//

	

	void VBORenderManager::cleanVAO(GLuint vbo,GLuint vao){
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}//

	/*void VBORenderManager::renderWater(){
		vboWater.render(*this);
	}//

	void VBORenderManager::updateLayer(QVector3D mouse3D,float change){
		vboTerrain.updateTerrain(mouse3D,change);
	}//*/

	///////////////////////////////////////////////////////////////////
	// STATIC
	bool VBORenderManager::addStaticGeometry(QString geoName,std::vector<Vertex>& vert,QString texName,GLenum geometryType,int shaderMode){
		GLuint texId;
		if(nameToTexId.contains(texName)){
			texId=nameToTexId[texName];
		}else{
			printf("load img %s\n",texName.toAscii().constData());
			texId=VBOUtil::loadImage(texName);
			nameToTexId[texName]=texId;
		}
		if(geoName2StaticRender.contains(geoName)){
			// 1.1 If already in manager
			if(geoName2StaticRender[geoName].contains(texId)){
				if(geoName2StaticRender[geoName][texId].vertices.size()==0){
					//1.1.1 if also contains texture and the number of vertex=0--> vao created--> remove
					cleanVAO(geoName2StaticRender[geoName][texId].vbo,geoName2StaticRender[geoName][texId].vao);
					geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
				}else{
					//1.1.1 if also contains texture and the number of vertex!=0--> vao no created--> just append
					geoName2StaticRender[geoName][texId].vertices.insert(geoName2StaticRender[geoName][texId].vertices.end(),vert.begin(),vert.end());
				}
			}else{
				geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
			}
		}else{
			// 1.2 No yet in manager
			geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
		}
		return true;
	}//

	using namespace boost::polygon::operators;

	bool VBORenderManager::addStaticGeometry2(QString geoName,std::vector<glm::vec3>& pos,float zShift,bool inverseLoop,QString textureName,GLenum geometryType,int shaderMode,glm::vec2 texScale,glm::vec4 color){
		if(pos.size()<3){
			return false;
		}
		PolygonSetP polySet;
		polygonP tempPolyP;

		std::vector<pointP> vP;
		vP.resize(pos.size());
		float minX=FLT_MAX,minY=FLT_MAX;
		float maxX=-FLT_MAX,maxY=-FLT_MAX;

		for(int pN=0;pN<pos.size();pN++){
			vP[pN]=boost::polygon::construct<pointP>(pos[pN].x,pos[pN].y);
			minX=std::min<float>(minX,pos[pN].x);
			minY=std::min<float>(minY,pos[pN].y);
			maxX=std::max<float>(maxX,pos[pN].x);
			maxY=std::max<float>(maxY,pos[pN].y);
		}
		if(pos.back().x!=pos.front().x&&pos.back().y!=pos.front().y){
			vP.push_back(vP[0]);
		}

		boost::polygon::set_points(tempPolyP,vP.begin(),vP.end());
		polySet+=tempPolyP;
		std::vector<polygonP> allP;
		boost::polygon::get_trapezoids(allP,polySet);
		
		std::vector<Vertex> vert;

		for(int pN=0;pN<allP.size();pN++){
			//glColor3ub(qrand()%255,qrand()%255,qrand()%255);
			boost::polygon::polygon_with_holes_data<double>::iterator_type itPoly=allP[pN].begin();
			std::vector<glm::vec3> points;
			std::vector<glm::vec2> texP;
			while(itPoly!=allP[pN].end()){
				pointP cP=*itPoly;
				if(inverseLoop==false)
					points.push_back(glm::vec3(cP.x(),cP.y(),pos[0].z+zShift));
				else
					points.insert(points.begin(),glm::vec3(cP.x(),cP.y(),pos[0].z+zShift));

				//if(texZeroToOne==true){
					//texP.push_back(QVector3D((cP.x()-minX)/(maxX-minX),(cP.y()-minY)/(maxY-minY),0.0f));
				//}else{
					texP.push_back(glm::vec2((cP.x()-minX)*texScale.x,(cP.y()-minY)*texScale.y));
				//}
				itPoly++;
			}
			/*if(points.size()==4){//last vertex repited
				addTexTriang(texInd,points,texP,col,norm);
			}
			if(points.size()==5){
				addTexQuad(texInd,points,texP,col,norm);

			}
			if(points.size()==6){
				//addTexQuad(texInd,std::vector<QVector3D>(&points[0],&points[3]),std::vector<QVector3D>(&texP[0],&texP[3]),col,norm);

				addTexQuad(texInd,points,texP,col,norm);
				//addTexTriang(texInd,std::vector<QVector3D>(&points[3],&points[6]),std::vector<QVector3D>(&texP[3],&texP[6]),col,norm);
				//addTexTriang(texInd,std::vector<QVector3D>(&points[4],&points[6]),std::vector<QVector3D>(&texP[4],&texP[6]),col,norm);
			}*/
			vert.push_back(Vertex(points[0],glm::vec3(0,0,1),color,texP[0]));//texScale is a hack to define a color when it is not texture
			vert.push_back(Vertex(points[1],glm::vec3(0,0,1),color,texP[1]));
			vert.push_back(Vertex(points[2],glm::vec3(0,0,1),color,texP[2]));
			vert.push_back(Vertex(points[3],glm::vec3(0,0,1),color,texP[3]));
		}

		return addStaticGeometry(geoName,vert,textureName,geometryType,shaderMode);
	}//

	bool VBORenderManager::removeStaticGeometry(QString geoName){
		if(geoName2StaticRender.contains(geoName)){
			// iterate and remove
			renderGrid::iterator i;
			for (i = geoName2StaticRender[geoName].begin(); i != geoName2StaticRender[geoName].end(); ++i){
				cleanVAO(geoName2StaticRender[geoName][i.key()].vbo,geoName2StaticRender[geoName][i.key()].vao);
			}
			geoName2StaticRender[geoName].clear();
			geoName2StaticRender.remove(geoName);
		}else{
			printf("ERROR: Remove Geometry %s but it did not exist\n",geoName.toAscii().constData());
			return false;
		}

		return true;
	}//

	void VBORenderManager::renderStaticGeometry(QString geoName){

		if(geoName2StaticRender.contains(geoName)){
			// iterate and remove
			renderGrid::iterator i;
			for (i = geoName2StaticRender[geoName].begin(); i != geoName2StaticRender[geoName].end(); ++i){
				//printf("renderStaticGeometry %s\n",geoName.toAscii().constData());
				renderVAO(i.value(),false);
			}
		}else{
			printf("ERROR: Render Geometry %s but it did not exist\n",geoName.toAscii().constData());
			return;
		}
	}//

	///////////////////////////////////////////////////////////////////
	// GRID
	bool VBORenderManager::addGridGeometry(QString geoName,std::vector<Vertex>& vert,QString textureName){
		return false;
	}//
	bool VBORenderManager::removeGridGeometry(QString geoName){
		return false;
	}//
	
	void VBORenderManager::renderGridGeometry(QString geoName){
		
	}//
	/*///////////////////////////////////////////////////////////////////
	// MODEL
	//QHash<QString,std::vector<ModelSpec>> nameToVectorModels;
	void VBORenderManager::addStreetElementModel(QString name,ModelSpec mSpec){
		nameToVectorModels[name].push_back(mSpec);
	}//
	void VBORenderManager::renderAllStreetElementName(QString name){
		for(int i=0;i<nameToVectorModels[name].size();i++){
			VBOModel_StreetElements::renderOneStreetElement(program,nameToVectorModels[name][i]);
		}
		printf("name %s --> %d\n",name.toAscii().constData(),nameToVectorModels[name].size());
	}//
	void VBORenderManager::removeAllStreetElementName(QString name){
		nameToVectorModels[name].clear();
	}
	
	*/

} // namespace LC