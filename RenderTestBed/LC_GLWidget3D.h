#pragma once

#include "glew.h"
#include <QGLWidget>
#include "VBORenderManager.h"
//#include "LC_camera_3d.h"
#include "Camera.h"

#include "misctools/misctools.h"
//#include "LC_GLWidget3D_Shadows.h"


namespace LC {

//class UrbanMain;
class LCUrbanMain;

class LCGLWidget3D : public QGLWidget {
//	Q_OBJECT

protected:
	LCUrbanMain* urbanMain;
	//Camera3D *myCam;
	Camera camera;
	//ucore::TextureManager* textureManager;
	bool shiftPressed;
	bool controlPressed;
	bool altPressed;
	bool keyMPressed;
	bool keyLPressed;
	QPoint lastPos;
	float farPlaneToSpaceRadiusFactor;
	float spaceRadius;
	float rotationSensitivity;
	float zoomSensitivity;

	QMatrix4x4 mvpMatrix;
	QMatrix4x4 pMatrix;
	QMatrix3x3 normalMatrix;

	
	//GLWidgetSimpleShadow shadow;
	//bool shadowEnabled;
public:
	VBORenderManager vboRenderManager;

	LCGLWidget3D(QWidget *parent = 0);
	~LCGLWidget3D();	

	//Camera3D* getCamera() { return myCam; }

	void updateMe();
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

	void resetMyCam();

	void setLightPosition(double altitude, double azimuth);
	void setLightPosition(GLfloat x, GLfloat y, GLfloat z);

	//bool mouseTo3D(int x, int y, QVector3D* result);
	bool mouseMoved;

	void drawScene(int drawMode);


signals:	

protected:
	void initializeGL();

	void resizeGL(int width, int height);
	void paintGL();    
		
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void changeXRotation(float angle);
	void changeYRotation(float angle);
	void changeZRotation(float angle);
	void changeXYZTranslation(float dx, float dy, float dz);
};

} // namespace ucore