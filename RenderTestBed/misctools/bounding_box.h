#pragma once
/************************************************************************************************
*
*		Bounding Box
*
*
*		@desc Main
*		@author igarciad
*
************************************************************************************************/
#ifndef MTC_MISCTOOLS_BBOX_H
#define MTC_MISCTOOLS_BBOX_H

#include "common.h"

namespace LC {
	namespace misctools {

		class BBox{
		public:

			BBox(){
				this->resetMe();	
			}

			~BBox(){
			}

			BBox(const BBox &ref){	
				minPt = ref.minPt;
				maxPt = ref.maxPt;
			}

			inline BBox &operator=(const BBox &ref){				
				minPt = ref.minPt;
				maxPt = ref.maxPt;				
				return (*this);
			}

			inline void resetMe(void){
				minPt.setX(FLT_MAX);
				minPt.setY(FLT_MAX);
				minPt.setZ(FLT_MAX);
				maxPt.setX(-FLT_MAX);
				maxPt.setY(-FLT_MAX);
				maxPt.setZ(-FLT_MAX);
			}

			inline bool overlapsWithBBox(BBox &other){
				return  
					( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
					( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) ) &&
					( (this->minPt.z() <= other.maxPt.z()) && (this->maxPt.z() >= other.minPt.z()) );
			}

			void combineWithBBox(BBox &other);

			void addPoint(QVector3D &newPt);

			inline bool overlapsWithBBoxXY(BBox &other){
				return  
					( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
					( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) );					
			}

			inline QVector3D midPt(void){
				return (0.5*(minPt + maxPt));
			}

			QVector3D minPt;
			QVector3D maxPt;
		};
	}
}

#endif