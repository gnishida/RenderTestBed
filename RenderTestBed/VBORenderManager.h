#pragma once

#include "glew.h"
#include "Shader.h"
#include "VBOUtil.h"
#include "qmap.h"
#include "Vertex.h"
#include <QHash>

#ifndef Q_MOC_RUN
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/multi/multi.hpp>
#include <boost/polygon/polygon.hpp>
#endif

namespace LC {


	const int mode_AdaptTerrain=0x0100;
	const int mode_Lighting=0x0200;
	const int mode_TexArray=0x0400;
	//0x0100 --> adapt terrain
	//0x0200 --> lighting

	struct RenderSt{
		uint texNum;//0 means use color
		//int gridIndex;
		GLuint vbo;
		GLuint vao;
		int numVertex;//defines if the vbo has been created
		std::vector<Vertex> vertices;

		GLenum geometryType;
		int shaderMode;

		RenderSt(uint _texNum,std::vector<Vertex> _vertices,GLenum geoT,int shModer){
			texNum=_texNum;
			vertices=_vertices;
			geometryType=geoT;
			shaderMode=shModer;
			numVertex=-1;
		}
		RenderSt(){
			numVertex=-1;
		}
	};

	typedef QHash<uint,RenderSt> renderGrid;

	/////////////////////////////////////
	// VBORenderManager

	class VBORenderManager {
	
	public:

		// POLYGON
		typedef boost::polygon::point_data<double> pointP;
		typedef boost::polygon::polygon_set_data<double> polygon_setP;
		typedef boost::polygon::polygon_with_holes_data<double> polygonP;
		typedef std::pair<pointP, pointP> edgeP;
		typedef std::vector<boost::polygon::polygon_data<double> > PolygonSetP;


		QMap<QString,int> geoNameToGeoNum;
		GLuint program_pass1;
		GLuint program_pass2;
		GLuint program_pass3;

		Shader shader;

		int currentIndexGeo;

		VBORenderManager();
		~VBORenderManager();

		void init();
				
		/// water
		//VBOWater vboWater;
		//void renderWater();

		/// GUI
		//VBOGUI vboGUI;

		///////////////
		// DEFERRED SHADING
		std::vector<QString> fragDataNamesP1;//Multi target fragmebuffer names P1
		std::vector<GLuint> fragDataTex;
		GLuint fragNoiseTex;
		GLuint fragDepthTex;
		GLuint fragDataFB;
		GLuint fragAOTex;
		GLuint fragDataFB_AO;
		GLuint fragDepthTex_AO;
		void resizeFragTex(int width,int height);
		// second pass
		GLuint secondPassVBO;
		GLuint secondPassVAO;
		//void renderSecondPass();
		// ssao
		float uRadius;
		float uPower;
		float uKernelSize;
		std::vector<float> uKernelOffsets;
		void resizeSsaoKernel();

		// textures
		QHash<QString,GLuint> nameToTexId;
		GLuint loadTexture(const QString fileName,bool mirrored=false);

		//static
		bool addStaticGeometry(QString geoName,std::vector<Vertex>& vert,QString textureName,GLenum geometryType,int shaderMode);
		bool addStaticGeometry2(QString geoName,std::vector<glm::vec3>& pos,float zShift,bool inverseLoop,QString textureName,GLenum geometryType,int shaderMode,glm::vec2 texScale,glm::vec4 color);
		//bool addStaticConvexPoly(QString geoName,std::vector<QVector3D>& pos,float zShift,bool inverseLoop,QString textureName,int shaderMode,QVector3D texScale,bool tesselate=true,QVector3D* color=0);
		bool removeStaticGeometry(QString geoName);
		void renderStaticGeometry(QString geoName);


		//grid
		bool addGridGeometry(QString geoName,std::vector<Vertex>& vert,QString textureName);
		bool removeGridGeometry(QString geoName);
		void renderGridGeometry(QString geoName);

		//models
		/*QHash<QString,std::vector<ModelSpec>> nameToVectorModels;
		bool initializedStreetElements;
		void addStreetElementModel(QString name,ModelSpec mSpec);
		void renderAllStreetElementName(QString name);
		void removeAllStreetElementName(QString name);*/

		
		void renderAll(bool cleanVertex);
	private:

		QHash<QString,QHash<int,renderGrid>> geoName2RenderGrids;
		QHash<QString,renderGrid> geoName2StaticRender;

		void renderVAO(RenderSt& renderSt,bool cleanVertex);
		bool createVAO(std::vector<Vertex>& vert,GLuint& vbo,GLuint& vao,int& numVertex);
		void cleanVAO(GLuint vbo,GLuint vao);
		
	};

} // namespace LC