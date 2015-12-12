#pragma once

#include "glew.h"
#include <QtGui/QMainWindow>

#include "GeneratedFiles\ui_LC_UrbanMain.h"

namespace LC {

class LCGLWidget3D;
class UrbanModule;
class TextureManager;

class LCUrbanMain : public QMainWindow {
	Q_OBJECT

protected:
	LCGLWidget3D* glWidget3D;

public:
	LCUrbanMain(QWidget *parent = 0, Qt::WFlags flags = 0);
	~LCUrbanMain();

	Ui_LCUrbanMain ui;

	void init();
	void render(TextureManager* textureManager);

	//void keyPressEvent(QKeyEvent* e);
	//void keyReleaseEvent(QKeyEvent* e);
	//void updateGL();

public slots:
	void ssaoSpinBox(double);
	void ssaoCheckBox(int);
private:

};

} // namespace LC