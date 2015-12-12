#include "LC_UrbanMain.h"
#include "LC_GLWidget3D.h"

namespace LC {

	LCUrbanMain::LCUrbanMain(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
		glWidget3D = new LCGLWidget3D(this);
		ui.setupUi(this);
		this->setCentralWidget(glWidget3D);


		connect(ui.ssaoKernelSizeSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(ssaoSpinBox(double)));
		connect(ui.ssaoRadiusSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(ssaoSpinBox(double)));
		connect(ui.ssaoPowerSpinBox, SIGNAL(valueChanged(double)),
			this, SLOT(ssaoSpinBox(double)));

		connect(ui.ssaoEnabledCheckBox, SIGNAL(stateChanged(int)),
			this, SLOT(ssaoCheckBox(int)));
	}

	LCUrbanMain::~LCUrbanMain() {
	}


	/**
	 * Initialize geometry and generate the entire geometry objects.
	 */
	void LCUrbanMain::init() {

	}


	void LCUrbanMain::ssaoSpinBox(double) {
		printf("--> ssaoSpinBox\n");
		int uKernelSize = ui.ssaoKernelSizeSpinBox->value();
		if (uKernelSize != glWidget3D->vboRenderManager.uKernelSize) {

			glWidget3D->vboRenderManager.uKernelSize = uKernelSize;
			glWidget3D->vboRenderManager.resizeSsaoKernel();
		}
		glWidget3D->vboRenderManager.uRadius = ui.ssaoRadiusSpinBox->value();
		glWidget3D->vboRenderManager.uPower = ui.ssaoPowerSpinBox->value();
		glWidget3D->updateGL();

	}//

	void LCUrbanMain::ssaoCheckBox(int) {
		printf("--> ssaoCheckBox\n");
		glWidget3D->updateGL();
	}//


} // namespace ucore