#include "bounding_box.h"

//**** BBox
namespace LC {
	namespace misctools {

		void BBox::combineWithBBox(BBox &other)
		{	
			minPt.setX(qMin(minPt.x(), other.minPt.x()));
			minPt.setY(qMin(minPt.y(), other.minPt.y()));
			minPt.setZ(qMin(minPt.z(), other.minPt.z()));

			maxPt.setX(qMax(maxPt.x(), other.maxPt.x()));
			maxPt.setY(qMax(maxPt.y(), other.maxPt.y()));
			maxPt.setZ(qMax(maxPt.z(), other.maxPt.z()));

			return;
		}

		void BBox::addPoint(QVector3D &newPt)
		{
			minPt.setX(qMin(minPt.x(), newPt.x()));
			minPt.setY(qMin(minPt.y(), newPt.y()));
			minPt.setZ(qMin(minPt.z(), newPt.z()));

			maxPt.setX(qMax(maxPt.x(), newPt.x()));
			maxPt.setY(qMax(maxPt.y(), newPt.y()));
			maxPt.setZ(qMax(maxPt.z(), newPt.z()));
		}		

	}
}