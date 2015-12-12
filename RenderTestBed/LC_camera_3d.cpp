#include "LC_camera_3d.h"
#include "qfile.h"

namespace LC {

	Camera3D::Camera3D(){
		sensitivity = 0.002f;
		walk_speed = 15.0f;

		this->resetCamera();	
	}//



	// Compute the 8 corner points of the current view frustum
	void Camera3D::updateFrustumPoints(frustum &f, QVector3D &center, QVector3D &view_dir){
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


	void Camera3D::resizeCam(int halfW,int halfH){
		half_height = halfW;
		half_width = halfH;

		for(int i=0; i<MAX_SPLITS; i++)
		{
			f[i].fov = 45.0/57.2957795 + 0.2f;
			f[i].ratio = (double)halfW/(double)halfH;
		}
	}//

	void Camera3D::resetCamera(void){
		cam_pos=QVector3D(750.0, 500.0f, 900);
		cam_view=QVector3D(-0.5f, -0.5f, -0.7f);

		light_dir=QVector4D(-0.5f,0.4f,-0.77f,0.0f);
		light_dir=QVector4D(-0.38f,0.86f,-0.34f,0.00f);
		light_dir=QVector4D(-0.40f,0.81f,-0.51f,0.00f);
		fovy = 45.0f;
	}

	void Camera3D::rotate_view(QVector3D& view, float angle, float x, float y, float z){
		float new_x;
		float new_y;
		float new_z;

		float c = cos(angle);
		float s = sin(angle);

		new_x  = (x*x*(1-c) + c)	* view.x();
		new_x += (x*y*(1-c) - z*s)	* view.y();
		new_x += (x*z*(1-c) + y*s)	* view.z();

		new_y  = (y*x*(1-c) + z*s)	* view.x();
		new_y += (y*y*(1-c) + c)	* view.y();
		new_y += (y*z*(1-c) - x*s)	* view.z();

		new_z  = (x*z*(1-c) - y*s)	* view.x();
		new_z += (y*z*(1-c) + x*s)	* view.y();
		new_z += (z*z*(1-c) + c)	* view.z();

		view=QVector3D(new_x,new_y,new_z);
		view.normalize();
	}//

	void Camera3D::motion(int dx, int dy,bool moveLight){

		float rot_x, rot_y;
		QVector3D rot_axis;

		rot_x = -dx *sensitivity;//- to invert
		rot_y = dy *sensitivity;

		if(moveLight==true)	{
			printf("1Move light %f %f %f %f\n",light_dir.x(),light_dir.y(),light_dir.z());
			light_dir.setY(light_dir.y()+ rot_y);
			if(light_dir.y() < 0.2f)
				light_dir.setY(0.2f);
			light_dir.normalize();
			QVector3D light_dir3D=light_dir.toVector3D();
			rotate_view(light_dir3D, -rot_x, cam_view.x(), cam_view.y(), cam_view.z());
			light_dir=QVector4D(light_dir3D.x(),light_dir3D.y(),light_dir3D.z(),light_dir.w());
			printf("2Move light %f %f %f %f\n",light_dir.x(),light_dir.y(),light_dir.z());
		}else{
			rotate_view(cam_view, rot_x, 0.0f, 1.0f, 0.0f);
			rot_axis=QVector3D(-cam_view.z(),0,cam_view.x());
			rot_axis.normalize();
			rotate_view(cam_view, rot_y, rot_axis.x(), rot_axis.y(), rot_axis.z());
		}
	}

	void Camera3D::camLook(){
		mvMatrix.setToIdentity();
		QVector3D dirV=cam_pos+ cam_view;
		mvMatrix.lookAt(QVector3D(cam_pos.x(),cam_pos.z(),cam_pos.y()),
			//cam_pos+ cam_view,
			QVector3D(dirV.x(),dirV.z(),dirV.y()),
			QVector3D(0.0f, 0.0f, 1.0f));
	}//

	void Camera3D::cameraInverse(float dst[16], float src[16]){
		dst[0] = src[0];
		dst[1] = src[4];
		dst[2] = src[8];
		dst[3] = 0.0f;
		dst[4] = src[1];
		dst[5] = src[5];
		dst[6]  = src[9];
		dst[7] = 0.0f;
		dst[8] = src[2];
		dst[9] = src[6];
		dst[10] = src[10];
		dst[11] = 0.0f;
		dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
		dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
		dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
		dst[15] = 1.0f;
	}//

	void Camera3D::moveKey(int typeMode){
		if(typeMode==0){//GetAsyncKeyState('W')){
			cam_pos.setX(cam_pos.x()+ cam_view.x() * walk_speed);
			cam_pos.setY(cam_pos.y()+ cam_view.y() * walk_speed);
			cam_pos.setZ(cam_pos.z()+ cam_view.z() * walk_speed);
		}
		if(typeMode==1){//if(GetAsyncKeyState('S')){
			cam_pos.setX(cam_pos.x() - cam_view.x() * walk_speed);
			cam_pos.setY(cam_pos.y() - cam_view.y() * walk_speed);
			cam_pos.setZ(cam_pos.z() - cam_view.z() * walk_speed);
		}
		if(typeMode==2){//if(GetAsyncKeyState('A')){
			cam_pos.setX(cam_pos.x()+cam_view.z() * walk_speed);
			cam_pos.setZ(cam_pos.z()-cam_view.x() * walk_speed);		
		}
		if(typeMode==3){//if(GetAsyncKeyState('D')){
			cam_pos.setX(cam_pos.x()-cam_view.z() * walk_speed);
			cam_pos.setZ(cam_pos.z()+cam_view.x() * walk_speed);
		}

		if(typeMode==4){//if(GetAsyncKeyState(VK_SPACE)){
			cam_pos.setY( cam_pos.y()+ walk_speed);
		}
	}//

	void Camera3D::printCamera(){
		printf("cam_pos  %f %f %f cam_view %f %f %f fovy %f\n",cam_pos.x(),cam_pos.y(),cam_pos.z(),cam_view.x(),cam_view.y(),cam_view.z(),fovy);
	}

	void Camera3D::saveCameraPose(int numCam)
	{
		QFile camFile("data/camPose"+QString::number(numCam)+".cam");
		if (!camFile.open(QIODevice::WriteOnly | QIODevice::Text)){
			printf("ERROR: Cannot open the file cam.txt for writing\n");
			return;
		}
		QTextStream stream( &camFile );	

		stream << this->cam_pos.x() << " " << this->cam_pos.y() << " " << this->cam_pos.z() << " " <<
			this->cam_view.x() << " " << this->cam_view.y() << " " << " " << this->cam_view.z() << " " << this->fovy;		

		camFile.close();
	}//

	void Camera3D::loadCameraPose(int numCam)
	{
		QFile camFile("data/camPose"+QString::number(numCam)+".cam");

		if (!camFile.open(QIODevice::ReadOnly | QIODevice::Text)) // Open the file
		{
			printf("Can't open file camPose%d.cam\n",numCam);
		}

		else {
			QTextStream stream( &camFile); // Set the stream to read from myFile
			float x,y,z,dx,dy,dz,fov;
			stream >> x;
			stream >> y;
			stream >> z;
			stream >> dx;
			stream >> dy;
			stream >> dz;
			stream >> fov;
			cam_pos=QVector3D(x,y,z);
			cam_view=QVector3D(dx,dy,dz);
			fovy=fov;
		}
	}//

}