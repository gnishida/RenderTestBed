#include "polygon_3D.h"

namespace LC {
	namespace misctools {

		//**** Polygon3D
		void Polygon3D::renderContour(void)
		{	
			glBegin(GL_LINE_LOOP);
			for(size_t i=0; i<contour.size(); ++i){
				glVertex3f(contour[i].x(),
					contour[i].y(),
					contour[i].z());
			}
			glEnd();	


		}


		void Polygon3D::render(void)
		{		
			glBegin(GL_POLYGON);
			for(size_t i=0; i<contour.size(); ++i){
				glVertex3f(contour[i].x(),
					contour[i].y(),
					contour[i].z());
			}
			glEnd();			
		}

		void Polygon3D::renderNonConvex(bool reComputeNormal,
			float nx, float ny, float nz)
		{
			QVector3D myNormal;
			if(reComputeNormal)
			{
				myNormal = this->getNormalVector();
			} else {
				myNormal.setX(nx);
				myNormal.setY(ny);
				myNormal.setZ(nz);
			}

			//Render inside fill			
			if(contour.size() == 3){
				glBegin(GL_TRIANGLES);	
				for(size_t i=0; i<contour.size(); i++){	
					glNormal3f(myNormal.x(), myNormal.y(), myNormal.z());
					glVertex3f(contour[i].x(), contour[i].y(), contour[i].z());			
				}
				glEnd();
			} else if(contour.size() == 4){
				glBegin(GL_QUADS);	
				for(int i=0; i<contour.size(); i++){	
					glNormal3f(myNormal.x(), myNormal.y(), myNormal.z());
					glVertex3f(contour[i].x(), contour[i].y(), contour[i].z());			
				}
				glEnd();
			} else {

				// create tessellator
				GLUtesselator *tess = gluNewTess();

				double *vtxData = new double[3*contour.size()];
				for(size_t i=0; i<contour.size(); i++){
					vtxData[3*i]=contour[i].x();
					vtxData[3*i+1]=contour[i].y();
					vtxData[3*i+2]=contour[i].z();
				}

				// register callback functions
				gluTessCallback(tess, GLU_TESS_BEGIN, 
					(void (__stdcall *)(void))glBegin);
				gluTessCallback(tess, GLU_TESS_VERTEX,
					(void (__stdcall *)(void))glVertex3dv);
				gluTessCallback(tess, GLU_TESS_END, glEnd);

				// describe non-convex polygon
				gluTessBeginPolygon(tess, NULL);


				// contour
				gluTessBeginContour(tess);

				for(size_t i=0; i<contour.size(); i++){
					//HACK
					glNormal3f(myNormal.x(), myNormal.y(), fabs(myNormal.z()));
					gluTessVertex(tess, &vtxData[3*i], &vtxData[3*i]);
				}
				gluTessEndContour(tess);

				gluTessEndPolygon(tess);

				// delete tessellator after processing
				gluDeleteTess(tess);

				delete [] vtxData;
			}

			//Render contour wire
			/*glColor3f(0.5f, 0.5f, 1.0f);			
			glBegin(GL_LINES);
			for(int i=0; i<contour.size()-1; ++i){
			if(i==contour.size()-2)//last
			glColor3f(0.5f, 1.0f, 1.0f);
			else
			glColor3f(0.5f, 0.5f, 1.0f);
			if(LC::misctools::ClientGlobalVariable::gV()->render_adaptGeometry==false){
			glVertex3f(contour[i].x(),contour[i].y(),contour[i].z());
			glVertex3f(contour[i+1].x(),contour[i+1].y(),contour[i+1].z());
			}
			else{
			glVertex3f(contour[i].x(),contour[i].y(),0);
			glVertex3f(contour[i+1].x(),contour[i+1].y(),0);
			}
			}*/

			/*
			glColor3f(0.5f, 0.5f, 1.0f);
			glBegin(GL_LINE_LOOP);
			for(size_t i=0; i<contour.size(); ++i){

			if(LC::misctools::ClientGlobalVariable::gV()->render_adaptGeometry==false)
			glVertex3f(contour[i].x(),contour[i].y(),0.0f);
			else
			glVertex3f(contour[i].x(),contour[i].y(),contour[i].z());
			}
			glEnd();
			*/
		}

		QVector3D calculateNormal(QVector3D& p0,QVector3D& p1,QVector3D& p2);

		QVector3D Polygon3D::getLoopNormalVector(Loop3D &pin)
		{
			if(pin.size() >= 3){
				return (calculateNormal(pin[0], pin[1], pin[2]));
			}
			return ( QVector3D(0, 0, 0) );
		}

		QVector3D Polygon3D::getNormalVector()
		{	
			if(this->normalVec.isNull())
			{
				normalVec = getLoopNormalVector(this->contour);
			}
			return normalVec;
		}

		bool segmentSegmentIntersectXY(QVector2D &a, QVector2D &b, QVector2D &c, QVector2D &d,
			float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint);

		//Only works for polygons with no holes in them
		bool Polygon3D::splitMeWithPolyline(std::vector<QVector3D> &pline, Loop3D &pgon1, Loop3D &pgon2)
		{
			bool polylineIntersectsPolygon = false;

			int plineSz = pline.size();
			int contourSz = this->contour.size();

			if(plineSz < 2 || contourSz < 3){
				//std::cout << "ERROR: Cannot split if polygon has fewer than three vertices of if polyline has fewer than two points\n.";
				return false;
			}

			QVector2D tmpIntPt;
			QVector2D firstIntPt;
			QVector2D secondIntPt;
			float tPline, tPgon;
			int firstIntPlineIdx    = -1;
			int secondIntPlineIdx   = -1;
			int firstIntContourIdx  = -1;
			int secondIntContourIdx = -1;
			int intCount = 0;


			//iterate along polyline
			for(int i=0; i<plineSz-1; ++i){
				int iNext = i+1;

				for(int j=0; j<contourSz; ++j){
					int jNext = (j+1)%contourSz;

					if (LC::misctools::segmentSegmentIntersectXY( QVector2D(pline[i]), QVector2D(pline[iNext]),
						QVector2D(contour[j]), QVector2D(contour[jNext]),
						&tPline, &tPgon, true, tmpIntPt) ) 
					{
						polylineIntersectsPolygon = true;

						//first intersection
						if(intCount == 0){
							firstIntPlineIdx = i;
							firstIntContourIdx = j;
							firstIntPt = tmpIntPt;
						} else if(intCount == 1) {
							secondIntPlineIdx = i;
							secondIntContourIdx = j;
							secondIntPt = tmpIntPt;
						} else {
							//std::cout << "Cannot split - Polyline intersects polygon at more than two points.\n";
							return false;
						}
						intCount++;
					}
				}
			}

			if(intCount != 2){
				//std::cout << "Cannot split - Polyline intersects polygon at " << intCount <<" points\n";
				return false;
			}

			//Once we have intersection points and indexes, we reconstruct the two polygons
			pgon1.clear();
			pgon2.clear();
			int pgonVtxIte;
			int plineVtxIte;

			//If first polygon segment intersected has an index greater
			//	than second segment, modify indexes for correct reconstruction
			if(firstIntContourIdx > secondIntContourIdx){
				secondIntContourIdx += contourSz;
			}

			//==== Reconstruct first polygon
			//-- append polygon contour
			pgon1.push_back(firstIntPt);
			pgonVtxIte = firstIntContourIdx;
			while( pgonVtxIte < secondIntContourIdx){
				pgon1.push_back(contour[(pgonVtxIte+1)%contourSz]);
				pgonVtxIte++;
			}
			pgon1.push_back(secondIntPt);
			//-- append polyline points
			plineVtxIte = secondIntPlineIdx;
			while(plineVtxIte > firstIntPlineIdx){
				pgon1.push_back(pline[(plineVtxIte)]);
				plineVtxIte--;
			}

			//==== Reconstruct second polygon
			//-- append polygon contour
			pgon2.push_back(secondIntPt);
			pgonVtxIte = secondIntContourIdx;
			while( pgonVtxIte < firstIntContourIdx + contourSz){
				pgon2.push_back(contour[(pgonVtxIte+1)%contourSz]);
				pgonVtxIte++;
			}
			pgon2.push_back(firstIntPt);
			//-- append polyline points
			plineVtxIte = firstIntPlineIdx;
			while(plineVtxIte < secondIntPlineIdx){
				pgon2.push_back(pline[(plineVtxIte + 1)]);
				plineVtxIte++;
			}


			//verify that two polygons are created after the split. If not, return false
			/////
			if(pgon1.size() < 3 || pgon2.size() < 3){
				//std::cout << "Invalid split - Resulting polygons have fewer than three vertices.\n";
				return false;
			}

			return polylineIntersectsPolygon;
		}

		/**
		* @brief: Reorient polygon faces so that they are CCW
		* @in: If only check is true, the polygon is not modified
		* @out: True if polygon had to be reoriented
		**/
		bool Polygon3D::reorientFace(Loop3D &pface, bool onlyCheck)
		{
			int pfaceSz = pface.size();
			int next;
			float tmpSum = 0.0f;

			for(int i=0; i<pfaceSz; ++i){
				next = (i+1)%pfaceSz;
				tmpSum = tmpSum + ( pface[next].x() - pface[i].x() )*( pface[next].y() + pface[i].y() );
			}			

			if(tmpSum > 0.0f)
			{				
				if(!onlyCheck){
					std::reverse(pface.begin(), pface.end());
				}
				return true;
			}
			return false;
		}


		/**
		* @brief: Given a polygon, this function computes the polygon's inwards offset. The offset distance
		* is not assumed to be constant and must be specified in the vector offsetDistances. The size of this
		* vector must be equal to the number of vertices of the polygon.
		* Note that the i-th polygon segment is defined by vertices i and i+1.
		* The polygon vertices are assumed to be oriented clockwise
		* @param[in] offsetDistances: Perpendicular distance from offset segment i to polygon segment i.
		* @param[out] pgonInset: The vertices of the polygon inset
		* @return insetArea: Returns the area of the polygon inset		
		**/
		float Polygon3D::computeInset(float offsetDistance, Loop3D &pgonInset, bool computeArea)
		{
			if(contour.size() < 3) return 0.0f;				
			std::vector<float> offsetDistances(contour.size(), offsetDistance);

			return computeInset(offsetDistances, pgonInset, computeArea);
		}

	}
}