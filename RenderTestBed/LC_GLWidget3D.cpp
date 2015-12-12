#pragma once

#include "LC_GLWidget3D.h"
#include "LC_UrbanMain.h"
#include <QMouseEvent>
#include <QDebug>
#include <QKeyEvent>
#include "qmath.h"
//#include "Util.h"

#define FAR_DIST 2500.0f

namespace LC {

	LCGLWidget3D::LCGLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
		urbanMain = (LCUrbanMain*)parent;

		//myCam = new Camera3D();
		spaceRadius = 5000.0;
		farPlaneToSpaceRadiusFactor = 5.0f;//N 5.0f

		rotationSensitivity = 0.4f;
		zoomSensitivity = 0.6f;

		controlPressed = false;
		shiftPressed = false;
		altPressed = false;
		keyMPressed = false;
		keyLPressed = false;

		//shadowEnabled=true;

	}

	LCGLWidget3D::~LCGLWidget3D() {
		//delete myCam;
	}

	QSize LCGLWidget3D::minimumSizeHint() const {
		return QSize(200, 200);
	}

	QSize LCGLWidget3D::sizeHint() const {
		return QSize(400, 400);
	}

	void LCGLWidget3D::resetMyCam() {
		//myCam->resetCamera();
		//cameraresetCamera();
		updateGL();
	}

	void LCGLWidget3D::mousePressEvent(QMouseEvent *event) {
		//printf(">>0mousePressEvent\n");
		this->setFocus();// force to recover focus from sliders
		mouseMoved = false;
		//printf(">>1mousePressEvent\n");
		QVector3D mouse3DPos;
		//mouseTo3D(event->x(), event->y(), &mouse3DPos);
		//printf("Pos2D %d %d --> Pos3D %f %f %f\n", event->x(), event->y(), mouse3DPos.x(), mouse3DPos.y(), mouse3DPos.z());
		//mouse3DPos.setZ(0);
		//printf(">>2mousePressEvent\n");
		lastPos = event->pos();
		camera.mousePress(event->x(), event->y());
		//printf("<<+mousePressEvent\n");
	}

	void LCGLWidget3D::mouseReleaseEvent(QMouseEvent *event) {
		event->ignore();


		setCursor(Qt::ArrowCursor);
		updateGL();

		return;
	}

	void LCGLWidget3D::mouseMoveEvent(QMouseEvent *event) {
		//printf(">>+mouseMoveEvent\n");
		//printf("moseState %d\n",LC::misctools::Global::global()->mouseState);
		mouseMoved = true;

		///////////////////////////////////////////////////////
		// 
		QVector3D mouse3DPos;
		/*mouseTo3D(event->x(), event->y(), &mouse3DPos);
		float dx = (float)(event->x() - lastPos.x());
		float dy = (float)(event->y() - lastPos.y());
		lastPos = event->pos();
		if (keyMPressed == true) {// update renderManager
			vboRenderManager.mousePos3D = mouse3DPos;
			printf("----------------\n----------\n");
			return;
		}
		*/
		camera.rotate(event->x(), event->y());
		//myCam->motion(dx, dy, keyLPressed);//if M pressed--> light Otherwise-->Move camera
		//if(keyLPressed==true)//update shadow map
		//	shadow.makeShadowMap(this);
		updateGL();
		//printf("<<+mouseMoveEvent\n");
	}

	void LCGLWidget3D::initializeGL() {
		////////////////////////////////////////
		//---- GLEW extensions ----
		GLenum err = glewInit();
		if (GLEW_OK != err) {// Problem: glewInit failed, something is seriously wrong.
			qDebug() << "Error: " << glewGetErrorString(err);
		}
		qDebug() << "Status: Using GLEW " << glewGetString(GLEW_VERSION);
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**4ERROR INIT: OpenGL-->" << err << endl;
		if (glewIsSupported("GL_VERSION_4_2"))
			printf("Ready for OpenGL 4.2\n");
		else {
			printf("OpenGL 4.2 not supported\n");
			//exit(1);
		}
		const GLubyte* text =
			glGetString(GL_VERSION);
		printf("VERSION: %s\n", text);
		//while ((err = glGetError()) != GL_NO_ERROR) qDebug() << "**3ERROR INIT: OpenGL-->" << err << endl;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glDisable(GL_TEXTURE_2D);

		glEnable(GL_TEXTURE_3D);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glDisable(GL_TEXTURE_3D);

		glEnable(GL_TEXTURE_2D_ARRAY);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glDisable(GL_TEXTURE_2D_ARRAY);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		////////////////////////////////
		vboRenderManager.init();
		vboRenderManager.resizeFragTex(this->width(), this->height());

		//// INIT SIMPLE GEOMETRY

		/*std::vector<QVector3D> vertBB;
		vertBB.push_back(QVector3D(-1000,-1000,0));
		vertBB.push_back(QVector3D(1000,-1000,0));
		vertBB.push_back(QVector3D(1000,1000,0));
		vertBB.push_back(QVector3D(-1000,1000,0));
		vboRenderManager.addStaticGeometry2("simple_geo1",vertBB,0,false,"data/textures/grass.jpg",GL_QUADS,2|LC::mode_Lighting,QVector3D(1.0f/(500.0f),1.0f/(500.0f),1.0f),QVector3D(0,0,0));
		*/
		std::vector<Vertex> terrVert;
		float r = 0, g = 92.0f / 255.0f, b = 9.0f / 255.0f;
		terrVert.push_back(Vertex(QVector3D(-1000, -1000, 0.0f), QVector3D(r, g, b), QVector3D(0, 0, 1), QVector3D(0, 0, 0)));
		terrVert.push_back(Vertex(QVector3D(1000, -1000, 0.0f), QVector3D(r, g, b), QVector3D(0, 0, 1), QVector3D(1, 0, 0)));
		terrVert.push_back(Vertex(QVector3D(1000, 1000, 0.0f), QVector3D(r, g, b), QVector3D(0, 0, 1), QVector3D(1, 1, 0)));
		terrVert.push_back(Vertex(QVector3D(-1000, 1000, 0.0f), QVector3D(r, g, b), QVector3D(0, 0, 1), QVector3D(0, 1, 0)));
		vboRenderManager.addStaticGeometry("simple_geo", terrVert, "", GL_QUADS, 1 | LC::mode_Lighting);//|LC::mode_AdaptTerrain|LC::mode_Lighting);
		//////////////////////////////////////////
		std::vector<Vertex> sideVertT;
		int nextN;
		QVector3D normal;
		QVector3D colorW(1, 1, 1);
		float height = 50;

		{// BOX GROUND
			float distance = 15.0f;
			std::vector<QVector3D> contB;
			contB.push_back(QVector3D(100 + distance, -100, 0));
			contB.push_back(QVector3D(300 + distance, -100, 0));
			contB.push_back(QVector3D(300 + distance, 100, 0));
			contB.push_back(QVector3D(100 + distance, 100, 0));

			for (int curN = 0; curN < contB.size(); curN++) {
				nextN = (curN + 1) % contB.size();
				QVector3D dir = contB[nextN] - contB[curN];
				float leng = dir.length();
				dir /= leng;
				normal = QVector3D::crossProduct(dir, QVector3D(0, 0, 1)).normalized();
				printf("nom %d: %f %f %f\n", curN, normal.x(), normal.y(), normal.z());
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 0.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 0.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 0.0f + height), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 0.0f + height), colorW, normal, QVector3D()));

			}
			//data/textures/LC/hatch/h2.png
			vboRenderManager.addStaticGeometry("simple_geo1", sideVertT, "", GL_QUADS, 1 | LC::mode_Lighting);//|LC::mode_AdaptTerrain|LC::mode_Lighting);
			//roof
			vboRenderManager.addStaticGeometry2("simple_geo1", contB, height, false, "", GL_QUADS, 1 | LC::mode_Lighting, QVector3D(1, 1, 1), colorW);
		}
		{//SIDE BOX GROUND
			std::vector<QVector3D> contB;
			contB.push_back(QVector3D(-100, -100, 0));
			contB.push_back(QVector3D(100, -100, 0));
			contB.push_back(QVector3D(100, 100, 0));
			contB.push_back(QVector3D(-100, 100, 0));

			for (int curN = 0; curN < contB.size(); curN++) {
				nextN = (curN + 1) % contB.size();
				QVector3D dir = contB[nextN] - contB[curN];
				float leng = dir.length();
				dir /= leng;
				normal = QVector3D::crossProduct(dir, QVector3D(0, 0, 1)).normalized();
				printf("nom %d: %f %f %f\n", curN, normal.x(), normal.y(), normal.z());
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 0.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 0.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 0.0f + height), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 0.0f + height), colorW, normal, QVector3D()));

			}
			//data/textures/LC/hatch/h2.png
			vboRenderManager.addStaticGeometry("simple_geo1", sideVertT, "", GL_QUADS, 1 | LC::mode_Lighting);//|LC::mode_AdaptTerrain|LC::mode_Lighting);
			//roof
			vboRenderManager.addStaticGeometry2("simple_geo1", contB, height, false, "", GL_QUADS, 1 | LC::mode_Lighting, QVector3D(1, 1, 1), colorW);
		}
		{ // TOP BOX
			std::vector<QVector3D> contB;
			contB.push_back(QVector3D(-50, -50, 0));
			contB.push_back(QVector3D(50, -50, 0));
			contB.push_back(QVector3D(50, 50, 0));
			contB.push_back(QVector3D(-50, 50, 0));

			for (int curN = 0; curN < contB.size(); curN++) {
				nextN = (curN + 1) % contB.size();
				QVector3D dir = contB[nextN] - contB[curN];
				float leng = dir.length();
				dir /= leng;
				normal = QVector3D::crossProduct(dir, QVector3D(0, 0, 1)).normalized();
				printf("nom %d: %f %f %f\n", curN, normal.x(), normal.y(), normal.z());
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 50.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 50.0f), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[nextN].x(), contB[nextN].y(), 50.0f + height), colorW, normal, QVector3D()));
				sideVertT.push_back(Vertex(QVector3D(contB[curN].x(), contB[curN].y(), 50.0f + height), colorW, normal, QVector3D()));

			}
			//data/textures/LC/hatch/h2.png
			vboRenderManager.addStaticGeometry("simple_geo1", sideVertT, "", GL_QUADS, 1 | LC::mode_Lighting);//|LC::mode_AdaptTerrain|LC::mode_Lighting);
			//roof
			vboRenderManager.addStaticGeometry2("simple_geo1", contB, 50+height, false, "", GL_QUADS, 1 | LC::mode_Lighting, QVector3D(1, 1, 1), colorW);
		}
		/////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//shadow.initShadow(vboRenderManager.program,this);

		////////////////////////////////
		// INIT FRAMEBUFFER TEXTURES
		/*GLuint FramebufferName = 0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		GLuint renderedTexture;
		glGenTextures(1, &renderedTexture);

		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, this->width(), this->height(), 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		*/
		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, "tex0"), 0);//tex0: 0
	}

	void LCGLWidget3D::resizeGL(int width, int height) {
		height = height ? height : 1;

		printf("Resize w %d h %d\n", width, height);

		//myCam->resizeCam(width / 2.0f, height / 2.0f);
		camera.updatePMatrix(width, height);

		glViewport(0, 0, (GLint)width, (GLint)height);

		//////////////////////////
		// PROJECTION MATRIX

		/*float fov = myCam->fovy;

		float aspect = (float)width / (float)height;
		float zfar = 4.0f*FAR_DIST;//4* to allow see the sky
		float znear = 5.0f;//also change in cameraTo3D

		float f = 1.0f / tan(fov * (M_PI / 360.0));

		double m[16] =
		{ f / aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (zfar + znear) / (znear - zfar), (2.0f*zfar*znear) / (znear - zfar),
		0, 0, -1, 0

		};
		pMatrix = QMatrix4x4(m);
		*/
		// resize buffers
		vboRenderManager.resizeFragTex(this->width(), this->height());
		updateGL();
	}

	void LCGLWidget3D::paintGL() {
		//VBOUtil::disaplay_memory_usage();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PASS 1: Render to texture
		glUseProgram(vboRenderManager.program_pass1);
		{
			glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program_pass1, "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);//mvpMatrixArray);
			glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program_pass1, "mvMatrix"), 1, false, &camera.mvMatrix[0][0]);//mvMatrixArray);
			//glUniformMatrix3fv(glGetUniformLocation(vboRenderManager.program_pass1, "normalMatrix"), 1, false, normMatrixArray);
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, vboRenderManager.fragDataFB);
		qglClearColor(QColor(0x00, 0xFF, 0xFF));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vboRenderManager.fragDataTex[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, vboRenderManager.fragDataTex[1], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, vboRenderManager.fragDataTex[2], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vboRenderManager.fragDepthTex, 0);
		// Set the list of draw buffers.
		GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, DrawBuffers); // "3" is the size of DrawBuffers
		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("+ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		drawScene(0);



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PASS 2: Create AO
		int enabledSSAO = urbanMain->ui.ssaoEnabledCheckBox->isChecked() ? 1:0;
		if (enabledSSAO == 1) {

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			qglClearColor(QColor(0xFF, 0xFF, 0xFF));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if(true){
				glBindFramebuffer(GL_FRAMEBUFFER, vboRenderManager.fragDataFB_AO);
			
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vboRenderManager.fragAOTex, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vboRenderManager.fragDepthTex_AO, 0);
				GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0};
				glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
			
			} 
		
			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				printf("++ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
				exit(0);
			}

		
			glDisable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);

			glUseProgram(vboRenderManager.program_pass2);
			glUniform2f(glGetUniformLocation(vboRenderManager.program_pass2, "pixelSize"), 2.0f / this->width(), 2.0f / this->height());
			//printf("pixelSize loc %d\n", glGetUniformLocation(vboRenderManager.program_pass2, "pixelSize"));

			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("tex0")), 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[0]);

			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("tex1")), 2);
			glActiveTexture(GL_TEXTURE2);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[1]);

			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("tex2")), 3);
			glActiveTexture(GL_TEXTURE3);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[2]);

			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("depthTex")), 8);
			glActiveTexture(GL_TEXTURE8);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDepthTex);

			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("noiseTex")), 7);
			glActiveTexture(GL_TEXTURE7);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragNoiseTex);

			{
				glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program_pass2, "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);//mvpMatrixArray);
				glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program_pass2, "pMatrix"), 1, false, &camera.pMatrix[0][0]);//pMatrixArray);
			}


			glUniform1i(glGetUniformLocation(vboRenderManager.program_pass2, ("uKernelSize")), vboRenderManager.uKernelSize);
			glUniform3fv(glGetUniformLocation(vboRenderManager.program_pass2, ("uKernelOffsets")), vboRenderManager.uKernelOffsets.size(), (const GLfloat*)vboRenderManager.uKernelOffsets.data());
			//printf("** ukernel %d %d\n", glGetUniformLocation(vboRenderManager.program_pass2, ("uKernelSize")), glGetUniformLocation(vboRenderManager.program_pass2, ("uKernelOffsets")));

			glUniform1f(glGetUniformLocation(vboRenderManager.program_pass2, ("uPower")), vboRenderManager.uPower);
			glUniform1f(glGetUniformLocation(vboRenderManager.program_pass2, ("uRadius")), vboRenderManager.uRadius);

			glBindVertexArray(vboRenderManager.secondPassVAO);

			glDrawArrays(GL_QUADS, 0, 4);
			glBindVertexArray(0);
			glDepthFunc(GL_LEQUAL);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PASS 3: Final

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		qglClearColor(QColor(0xFF, 0xFF, 0xFF));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUseProgram(vboRenderManager.program_pass3);
		glUniform2f(glGetUniformLocation(vboRenderManager.program_pass3, "pixelSize"), 2.0f / this->width(), 2.0f / this->height());
		//printf("pixelSize loc %d\n", glGetUniformLocation(vboRenderManager.program_pass3, "pixelSize"));

		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass3, ("tex0")), 1);//COLOR
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass3, ("tex1")), 2);//NORMAL
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[1]);

		/*glUniform1i(glGetUniformLocation(vboRenderManager.program_pass3, ("tex2")), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragDataTex[2]);*/

		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass3, ("tex3")), 4);//AO
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, vboRenderManager.fragAOTex);
		//printf(">> tex3 loc %d\n", glGetUniformLocation(vboRenderManager.program_pass3, "tex3"));

		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass3, ("enabledSSAO")), enabledSSAO);//AO

		glBindVertexArray(vboRenderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
		
		
		// REMOVE
		glActiveTexture(GL_TEXTURE0);


		//printf("<<\n");
		//VBOUtil::disaplay_memory_usage();
	}//

	void LCGLWidget3D::drawScene(int drawMode) {




		// NORMAL
		glDisable(GL_DEPTH_TEST);
		glDepthMask(false);
		glDepthFunc(GL_ALWAYS);
		//printf("RENDER NORMAL\n");
		glUniform1i(glGetUniformLocation(vboRenderManager.program_pass1, "shadowState"), 0);//SHADOW: Disable
		vboRenderManager.renderStaticGeometry(QString("sky"));

		//if(shadowEnabled)
		//	glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 1);//SHADOW: Render Normal with Shadows
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);
		// RENDER THINGS
		vboRenderManager.renderStaticGeometry("simple_geo");
		vboRenderManager.renderStaticGeometry("simple_geo1");


		//glDepthFunc(GL_ALWAYS);
		//glDisable(GL_TEXTURE_2D);

		glFlush();
	}

	void LCGLWidget3D::keyPressEvent(QKeyEvent *e) {
		//printf("k\n");
		float sensitivityFactor;

		shiftPressed = false;
		controlPressed = false;
		altPressed = false;
		keyMPressed = false;
		keyLPressed = false;

		switch (e->key()) {
		case Qt::Key_Escape:
			this->parentWidget()->close();
			break;
		case Qt::Key_Shift:
			shiftPressed = true;
			break;
		case Qt::Key_Control:
			controlPressed = true;
			break;
		case Qt::Key_Alt:
			altPressed = true;
			break;
		case Qt::Key_Z:
			break;
		default:
			;

		}
	}//

	void LCGLWidget3D::keyReleaseEvent(QKeyEvent* e) {
		if (e->isAutoRepeat()) {
			e->ignore();
			return;
		}
		switch (e->key()) {
		case Qt::Key_Escape:
			break;
		case Qt::Key_Shift:
			shiftPressed = false;
			break;
		case Qt::Key_Control:
			controlPressed = false;
			break;
		case Qt::Key_Alt:
			altPressed = false;
		default:
			;
		}
	}

	//from nehe.gamedev.net/data/articles/article.asp?article=13
	/*bool LCGLWidget3D::mouseTo3D(int x, int y, QVector3D *result) {
		//printf("mouseTo3D\n");
		GLint viewport[4];
		GLfloat winX, winY, winZ=0;
		GLdouble posX, posY, posZ;
		//printf("mouseTo3D 2\n");
		glGetIntegerv(GL_VIEWPORT, viewport);

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		//printf("mouseTo3D 21\n");
		//glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		//printf("mouseTo3D 3\n");
		myCam->camLook();
		gluUnProject(winX, winY, winZ, myCam->mvMatrix.data(), pMatrix.data(), viewport, &posX, &posY, &posZ);

		//if(posZ<-10000.0f)//if it -10000 means we are not in the scene <-- uncomment for real 3d position (and not plane)
		{
			// 2. ray test
			float zNear = 5.0f, zFar = 10.0f*spaceRadius;
			GLdouble posXFar, posYFar, posZFar;
			gluUnProject(winX, winY, zFar, myCam->mvMatrix.data(), pMatrix.data(), viewport, &posXFar, &posYFar, &posZFar);
			QVector3D rayStar(posX, posY, posZ);
			QVector3D rayEnd(posXFar, posYFar, posZFar);
			double t;
			QVector3D q1(0, 0, 1.0f);
			QVector3D q2(0, 0, 0);
			if (LC::misctools::planeIntersectWithLine(rayStar, rayEnd, q1, q2, t, *result) != 0)
				return true;
			return false;
		}
		//printf("mouseTo3D 4\n");
		result->setX(posX);
		result->setY(posY);
		result->setZ(posZ);
		//printf("mouseTo3D 5\n");
		return true;
	}//*/

} // namespace LC