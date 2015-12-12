/************************************************************************************************
*
*		MTC Project - Client Main Project - ClientGlobalVariables
*
*
*		@desc The global variables for the client.
*		@author igarciad
*
************************************************************************************************/
//based on http://www.yolinux.com/TUTORIALS/C++Singleton.html

#ifndef MTC_CLIENT_MAIN_GLOBAL_VARIABLES_H
#define MTC_CLIENT_MAIN_GLOBAL_VARIABLES_H

#include <QGLWidget>
#include <string>
#include <QVector3D>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace LC {
	namespace misctools {


		enum t_land_use {
			kLUNone = 0,
			kLUResS,
			kLUResM,
			kLUResL,
			kLUOfficeS,
			kLUOfficeM,
			kLUOfficeL,
			kLURetail,
			kLUIndS,
			kLUIndM,
			kLUIndL,
			kLUPking,
			kLUInstit,
			kLUPark,
			kLUWater
		};

		class Global{
		public:

			bool figureMode;
			// textures
			std::vector<GLuint> sidewalkTextures;
			std::vector<QVector3D> sidewalkTexturesScale;

			std::vector<GLuint> asfaltTextures;
			std::vector<GLuint> roofTextures;

			std::vector<GLuint> modelTextures;

			static Global* global();

			//living cities
			bool selectedParametricBuildings;
			bool render_lighting;
			//layer
			bool render_layer;
			int render_layer_number;
			int render_layer_radius_brush;
			int render_layer_value_brush;
			//simulation
			bool simulation_stop;
			int simulation_step;
			// CUDA
			float cuda_arterial_edges_speed_ms;
			int cuda_arterial_numLanes;
			int cuda_road_numLanes;
			float cuda_cell_size;
			float cuda_delta_time;
			float roadLaneWidth;

			// CUDA
			int cuda_carInfoRendering_type;

			int cuda_render_displaylist_staticRoadsBuildings;

			float arterial_edges_width;//R
			float arterial_edges_speed;;//R
			
			int render_building_with_shading;

			//selection
			int mouseState;
			static const int M_NORMAL=0;
			static const int M_AREA_SELECT_CENTER=1;
			static const int M_RADIUS_AREA_SELECT_CENTER=2;
			static const int M_PTYPE_CENTER_EDITION=30;
			static const int M_PTYPE_EXT_HANDLER_EDITION=31;
			static const int M_PTYPE_EXT_HANDLER2_EDITION=32;
			static const int M_PTYPE_UDIST_HANDLER_EDITION=33;
			static const int M_PTYPE_VDIST_HANDLER_EDITION=34;

			static const int M_BOUNDING_POLYGON_EDITION=7;

			
			bool view_color_with_place_types;

			bool view_place_types_centept_render;
			/*bool view_place_types_adjacencies_render;
			bool view_place_types_adj_measured_render;
			bool view_place_types_adj_probability_render;

			bool view_terrain_render;
			*/
			//bool view_arterials_render;	
			bool view_arterials_vertices_render;
			bool view_arterials_edges_render;
			bool view_arterials_arrows_render;

			bool view_blocks_render;
			/*
			bool view_parcels_render;
			bool view_parcels_buildable_area;
			*/
			bool view_buildings_render;
			bool view_trees_render;
			/*float view_buildings_transparency;
			*/
			//roads
			//float arterial_orientation;
			float arterial_edges_count;
			
			

			//float arterial_edges_lengthU;
			//float arterial_edges_lengthV;
			//float arterial_edges_irregularity;
			//float arterial_edges_curvature;
			//int   arterial_num_departing;

			//float building_height_mean2;
			//float building_height_deviation2;

			//float local_edges_lengthU;
			//float local_edges_lengthV;
			//float local_edges_irregularity;
			//float local_edges_curvature;

			int   num_place_types;

			//float parcel_area_mean;
			//float parcel_area_deviation;
			//float parcel_split_deviation;


			//float parcel_setback_front;
			//float parcel_setback_sides;
			
			//float land_use_park_percentage;
			
			//building
			//float building_height_mean;
			//float building_height_deviation;
			int render_parcelTextures;
			int cuda_current_time_slider;

			/*
			int building_style;

			bool path_savePath;
			bool path_playPath;*/

			//Bounding polygon
			std::vector<QVector3D> boundingPolygon;
			int selectedBoundingPolygonVertexIdx;

			// Drawing
			int cuda_drawing_rad;
			int cuda_drawing_selected;

			int cuda_design_edge_color;

			// building heights density
			int cuda_bldg_use_density;
			int cuda_bldg_use_density_factor;
			cv::Mat* peopleDist;
			cv::Mat* jobDist;

			int cuda_OSM_mode;

		private:
			Global(){};  // Private so that it can  not be called
			Global(Global const&){};             // copy constructor is private
			Global& operator=(Global const&){};  // assignment operator is private
			static Global* m_pInstance;
		};

	}
}

#endif