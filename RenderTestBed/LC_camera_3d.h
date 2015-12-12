#pragma once

#define MAX_SPLITS 4

#include "qvector3d.h"
#include "qvector4d.h"
#include "qmatrix4x4.h"
#include "qtextstream.h"


namespace LC {
	struct frustum
	{
		float neard;
		float fard;
		float fov;
		float ratio;
		QVector3D point[8];
	};


	class Camera3D
	{  
	public:

		// fustrum
		frustum f[MAX_SPLITS];
		void updateFrustumPoints(frustum &f, QVector3D &center, QVector3D &view_dir);

		// CAMERA

		QVector3D cam_pos; 
		QVector3D cam_view;
		float fovy;

		QVector4D light_dir;

		QMatrix4x4 mvMatrix;

		float sensitivity;
		float walk_speed;

		int old_x, old_y;
		int half_width;
		int half_height;

		Camera3D();

		~Camera3D()
		{
		}
		//
		void resetCamera(void);

		void saveCameraPose(int numCam);
		void loadCameraPose(int numCam);

		void rotate_view(QVector3D& view, float angle, float x, float y, float z);

		void motion(int dx, int dy,bool moveLight=false);

		void camLook();

		void cameraInverse(float dst[16], float src[16]);

		void moveKey(int typeMode);

		void printCamera();

		void resizeCam(int halfW,int halfH);

	};
}
