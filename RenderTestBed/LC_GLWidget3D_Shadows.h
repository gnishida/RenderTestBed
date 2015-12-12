#pragma once

#include "misctools/misctools.h"
#include "LC_camera_3d.h"

namespace LC {

	class LCGLWidget3D;

	class GLWidgetSimpleShadow
	{
	public:
		GLWidgetSimpleShadow();
		~GLWidgetSimpleShadow();	

		void makeShadowMap(LCGLWidget3D* glWidget3D);
		void initShadow(int _programId,LCGLWidget3D* glWidget3D);
		void shadowRenderScene(int program);

		void updateShadowMatrix(LCGLWidget3D* glWidget3D);
		
		// show depth textures
		void showDepthTex();
		bool displayDepthTex;
		bool displayDepthTexInit;
		uint depthTexProgram;
		uint depthTexVBO;
		uint fustrumVBO;
		//std::vector<float> quadDepthTex;
		float applyCropMatrix(frustum &f,LCGLWidget3D* glWidget3D);

		int programId;
		//QVector3D light_position;

		QMatrix4x4 light_biasMatrix;
		QMatrix4x4 light_pMatrix;
		QMatrix4x4 light_mvMatrix;
		QMatrix4x4 light_mvpMatrix;

		void setEnableShadowMap(bool enableDisable);
	};


}
