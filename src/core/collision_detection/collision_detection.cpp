/************************************************************************************

	AstroMenace (Hardcore 3D space shooter with spaceship upgrade possibilities)
	Copyright (c) 2006-2018 Mikhail Kurinnoi, Viewizard


	AstroMenace is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AstroMenace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AstroMenace. If not, see <http://www.gnu.org/licenses/>.


	Web Site: http://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

#include "collision_detection.h"

/* TODO translate comments */

/*
 * Check, is point belong triangle.
 */
static bool PointInTriangle(const VECTOR3D &point, const VECTOR3D &pa,
			    const VECTOR3D &pb, const VECTOR3D &pc)
{
	float TotalAngle{0.0f};

	VECTOR3D V1{point.x - pa.x, point.y - pa.y, point.z - pa.z};
	VECTOR3D V2{point.x - pb.x, point.y - pb.y, point.z - pb.z};
	VECTOR3D V3{point.x - pc.x, point.y - pc.y, point.z - pc.z};

	V1.NormalizeHi();
	V2.NormalizeHi();
	V3.NormalizeHi();

	TotalAngle += acosf(V1*V2);
	TotalAngle += acosf(V2*V3);
	TotalAngle += acosf(V3*V1);

	if (fabsf(TotalAngle - 2*3.14159265f) <= 0.005f) return true;

	return false;
}

/*
 * AABB-AABB collision detection.
 */
bool vw_AABBAABBCollision(const VECTOR3D Object1AABB[8], const VECTOR3D &Object1Location,
			  const VECTOR3D Object2AABB[8], const VECTOR3D &Object2Location)
{
	/* check projection's collisions */
	if (fabsf(Object1Location.x - Object2Location.x) > fabsf(Object1AABB[0].x + Object2AABB[0].x))
		return false;
	if (fabsf(Object1Location.y - Object2Location.y) > fabsf(Object1AABB[0].y + Object2AABB[0].y))
		return false;
	if (fabsf(Object1Location.z - Object2Location.z) > fabsf(Object1AABB[0].z + Object2AABB[0].z))
		return false;

	return true;
}

/*
 * OBB-OBB collision detection.
 */
bool vw_OBBOBBCollision(VECTOR3D Object1OBB[8], VECTOR3D Object1OBBLocation, VECTOR3D Object1Location, float Object1RotationMatrix[9],
			VECTOR3D Object2OBB[8], VECTOR3D Object2OBBLocation, VECTOR3D Object2Location, float Object2RotationMatrix[9])
{
	/* calcuate rotation matrix */
	float TMPInvObject1RotationMatrix[9]{Object1RotationMatrix[0], Object1RotationMatrix[1], Object1RotationMatrix[2],
					     Object1RotationMatrix[3], Object1RotationMatrix[4], Object1RotationMatrix[5],
					     Object1RotationMatrix[6], Object1RotationMatrix[7], Object1RotationMatrix[8]};
	Matrix33InverseRotate(TMPInvObject1RotationMatrix);
	/* calcuate first box size */
	VECTOR3D a{(Object1OBB[0] - Object1OBB[6])^0.5f};
	Matrix33CalcPoint(&a, TMPInvObject1RotationMatrix);
	/* calcuate inverse rotation matrix */
	float TMPInvObject2RotationMatrix[9]{Object2RotationMatrix[0], Object2RotationMatrix[1], Object2RotationMatrix[2],
					     Object2RotationMatrix[3], Object2RotationMatrix[4], Object2RotationMatrix[5],
					     Object2RotationMatrix[6], Object2RotationMatrix[7], Object2RotationMatrix[8]};
	Matrix33InverseRotate(TMPInvObject2RotationMatrix);
	/* calcuate second box size */
	VECTOR3D b{(Object2OBB[0] - Object2OBB[6])^0.5f};
	Matrix33CalcPoint(&b, TMPInvObject2RotationMatrix);
	/* calcuate offset in global coordinate systems */
	VECTOR3D T{(Object2Location + Object2OBBLocation) -
		   (Object1Location + Object1OBBLocation)};
	Matrix33CalcPoint(&T, TMPInvObject1RotationMatrix);
	/* calcuate transformation matrix */
	Matrix33Mult(TMPInvObject1RotationMatrix, Object2RotationMatrix);
	float R[3][3]{TMPInvObject1RotationMatrix[0], TMPInvObject1RotationMatrix[3], TMPInvObject1RotationMatrix[6],
		      TMPInvObject1RotationMatrix[1], TMPInvObject1RotationMatrix[4], TMPInvObject1RotationMatrix[7],
		      TMPInvObject1RotationMatrix[2], TMPInvObject1RotationMatrix[5], TMPInvObject1RotationMatrix[8]};

	/* 1 (Ra)x */
	if(fabsf(T.x) > a.x + b.x * fabsf(R[0][0]) + b.y * fabsf(R[0][1]) + b.z * fabsf(R[0][2]))
		return false;
	/* 2 (Ra)y */
	if(fabsf(T.y) > a.y + b.x * fabsf(R[1][0]) + b.y * fabsf(R[1][1]) + b.z * fabsf(R[1][2]))
		return false;
	/* 3 (Ra)z */
	if(fabsf(T.z) > a.z + b.x * fabsf(R[2][0]) + b.y * fabsf(R[2][1]) + b.z * fabsf(R[2][2]))
		return false;

	/* 4 (Rb)x */
	if(fabsf(T.x*R[0][0] + T.y*R[1][0] + T.z*R[2][0]) >
	    (b.x + a.x*fabsf(R[0][0]) + a.y * fabsf(R[1][0]) + a.z*fabsf(R[2][0])))
		return false;
	/* 5 (Rb)y */
	if(fabsf(T.x*R[0][1] + T.y*R[1][1] + T.z*R[2][1]) >
	    (b.y + a.x*fabsf(R[0][1]) + a.y * fabsf(R[1][1]) + a.z*fabsf(R[2][1])))
		return false;
	/* 6 (Rb)z */
	if(fabsf(T.x*R[0][2] + T.y*R[1][2] + T.z*R[2][2]) >
	    (b.z + a.x*fabsf(R[0][2]) + a.y * fabsf(R[1][2]) + a.z*fabsf(R[2][2])))
		return false;

	/* 7 (Ra)x X (Rb)x */
	if(fabsf(T.z*R[1][0] - T.y*R[2][0]) >
	    a.y*fabsf(R[2][0]) + a.z*fabsf(R[1][0]) + b.y*fabsf(R[0][2]) + b.z*fabsf(R[0][1]))
		return false;
	/* 8 (Ra)x X (Rb)y */
	if(fabsf(T.z*R[1][1] - T.y*R[2][1]) >
	    a.y*fabsf(R[2][1]) + a.z*fabsf(R[1][1]) + b.x*fabsf(R[0][2]) + b.z*fabsf(R[0][0]))
		return false;
	/* 9 (Ra)x X (Rb)z */
	if(fabsf(T.z*R[1][2]-T.y*R[2][2]) >
	    a.y*fabsf(R[2][2]) + a.z*fabsf(R[1][2]) + b.x*fabsf(R[0][1]) + b.y*fabsf(R[0][0]))
		return false;
	/* 10 (Ra)y X (Rb)x */
	if(fabsf(T.x*R[2][0]-T.z*R[0][0]) >
	    a.x*fabsf(R[2][0]) + a.z*fabsf(R[0][0]) + b.y*fabsf(R[1][2]) + b.z*fabsf(R[1][1]))
		return false;
	/* 11 (Ra)y X (Rb)y */
	if(fabsf(T.x*R[2][1]-T.z*R[0][1]) >
	    a.x*fabsf(R[2][1]) + a.z*fabsf(R[0][1]) + b.x*fabsf(R[1][2]) + b.z*fabsf(R[1][0]))
		return false;
	/* 12 (Ra)y X (Rb)z */
	if(fabsf(T.x*R[2][2]-T.z*R[0][2]) >
	    a.x*fabsf(R[2][2]) + a.z*fabsf(R[0][2]) + b.x*fabsf(R[1][1]) + b.y*fabsf(R[1][0]))
		return false;
	/* 13 (Ra)z X (Rb)x */
	if(fabsf(T.y*R[0][0]-T.x*R[1][0]) >
	    a.x*fabsf(R[1][0]) + a.y*fabsf(R[0][0]) + b.y*fabsf(R[2][2]) + b.z*fabsf(R[2][1]))
		return false;
	/* 14 (Ra)z X (Rb)y */
	if(fabsf(T.y*R[0][1]-T.x*R[1][1]) >
	    a.x*fabsf(R[1][1]) + a.y*fabsf(R[0][1]) + b.x*fabsf(R[2][2]) + b.z*fabsf(R[2][0]))
		return false;
	/* 15 (Ra)z X (Rb)z */
	if(fabsf(T.y*R[0][2]-T.x*R[1][2]) >
	    a.x*fabsf(R[1][2]) + a.y*fabsf(R[0][2]) + b.x*fabsf(R[2][1]) + b.y*fabsf(R[2][0]))
		return false;

	return true;
}

/*
 * Sphere-Sphere collision detection.
 */
bool vw_SphereSphereCollision(float Object1Radius, const VECTOR3D &Object1Location,
			      float Object2Radius, const VECTOR3D &Object2Location,
			      const VECTOR3D &Object2PrevLocation)
{
	bool Result{true};

	VECTOR3D Object1m2Location{Object1Location.x - Object2Location.x,
				   Object1Location.y - Object2Location.y,
				   Object1Location.z - Object2Location.z};
	float Object1p1Radius{Object1Radius + Object2Radius};

	/* fast check cube collisions */
	if ((fabsf(Object1m2Location.x) > Object1p1Radius) ||
	    (fabsf(Object1m2Location.y) > Object1p1Radius) ||
	    (fabsf(Object1m2Location.z) > Object1p1Radius))
		Result = false;

	/* fast check for sphere collision */
	if (Result) {
		/* power of 2 for distance, no reason in sqrt here */
		float Dist2{Object1m2Location.x*Object1m2Location.x +
			    Object1m2Location.y*Object1m2Location.y +
			    Object1m2Location.z*Object1m2Location.z};

		/* power of 2 for minimal distance */
		float NeedDist2{Object1p1Radius*Object1p1Radius};

		/* if distance less or equal - collision detected */
		if (Dist2 <= NeedDist2)
			return true;
	}

	/* check for distance from point to line (ray)*/
	if (!Result) {
		VECTOR3D Ray{Object2Location.x - Object2PrevLocation.x,
			     Object2Location.y - Object2PrevLocation.y,
			     Object2Location.z - Object2PrevLocation.z};
		Ray.Normalize();

		/* calculate closest point on ray */
		float Point{Ray.x*Object1Location.x +
			    Ray.y*Object1Location.y +
			    Ray.z*Object1Location.z - Ray.x*Object2PrevLocation.x +
						      Ray.y*Object2PrevLocation.y +
						      Ray.z*Object2PrevLocation.z};

		/* calculate closest point on line segment */
		VECTOR3D IntercPoint{Object2PrevLocation.x*Point,
				     Object2PrevLocation.y*Point,
				     Object2PrevLocation.z*Point};

		/* out of our line segment */
		if ((Object2PrevLocation.x - IntercPoint.x)*(Object2Location.x - IntercPoint.x) +
		    (Object2PrevLocation.y - IntercPoint.y)*(Object2Location.y - IntercPoint.y) +
		    (Object2PrevLocation.z - IntercPoint.z)*(Object2Location.z - IntercPoint.z) >= 0.0f)
			return false;

		/* check distance, same idea with power of 2 as above */
		float NewDist2{(IntercPoint.x - Object1Location.x)*(IntercPoint.x - Object1Location.x)+
			       (IntercPoint.y - Object1Location.y)*(IntercPoint.y - Object1Location.y)+
			       (IntercPoint.z - Object1Location.z)*(IntercPoint.z - Object1Location.z)};

		if (NewDist2 <= Object1Radius*Object1Radius)
			return true;
	}

	/* objects too far from each other */
	return false;
}

/*
 * Sphere-AABB collision detection.
 */
bool vw_SphereAABBCollision(VECTOR3D Object1AABB[8], VECTOR3D Object1Location,
			    float Object2Radius, VECTOR3D Object2Location, VECTOR3D Object2PrevLocation)
{
	bool Result{true};

	/* detect distance AABB<->cube */
	if ((fabsf(Object1Location.x - Object2Location.x) > Object1AABB[0].x + Object2Radius) ||
	    (fabsf(Object1Location.y - Object2Location.y) > Object1AABB[0].y + Object2Radius) ||
	    (fabsf(Object1Location.z - Object2Location.z) > Object1AABB[0].z + Object2Radius))
		Result = false;

	/* check for distance to line (ray)*/
	if (!Result) {
		/* middle point */
		VECTOR3D mid{(Object2Location + Object2PrevLocation) / 2.0f};
		/* line (ray) direction */
		VECTOR3D dir{Object2Location - Object2PrevLocation};
		/* half of line */
		float hl{dir.Length() / 2.0f};
		dir.Normalize();

		VECTOR3D T{Object1Location - mid};

		/* check axis */
		if ((fabs(T.x) > Object1AABB[0].x + hl*fabs(dir.x)) ||
		    (fabs(T.y) > Object1AABB[0].y + hl*fabs(dir.y)) ||
		    (fabs(T.z) > Object1AABB[0].z + hl*fabs(dir.z)))
			return false;

		/* check X ^ dir */
		float r{Object1AABB[0].y*fabs(dir.z) + Object1AABB[0].z*fabs(dir.y)};
		if (fabs(T.y*dir.z - T.z*dir.y) > r)
			return false;

		/* check  Y ^ dir */
		r = Object1AABB[0].x*fabs(dir.z) + Object1AABB[0].z*fabs(dir.x);
		if (fabs(T.z*dir.x - T.x*dir.z) > r)
			return false;

		/* check  Z ^ dir */
		r = Object1AABB[0].x*fabs(dir.y) + Object1AABB[0].y*fabs(dir.x);
		if (fabs(T.x*dir.y - T.y*dir.x) > r)
			return false;

		/* collision detected */
		return true;
	}

	return Result;
}

/*
 * Sphere-OBB collision detection.
 */
bool vw_SphereOBBCollision(VECTOR3D Object1OBB[8], VECTOR3D Object1OBBLocation,
			   VECTOR3D Object1Location, float Object1RotationMatrix[9],
			   float Object2Radius, VECTOR3D Object2Location, VECTOR3D Object2PrevLocation)
{
	VECTOR3D TMPMax{Object1OBB[0]};
	VECTOR3D TMPMin{Object1OBB[6]};
	VECTOR3D TMPPoint1{Object2Location - (Object1Location + Object1OBBLocation)};
	VECTOR3D TMPPoint2{Object2PrevLocation - (Object1Location + Object1OBBLocation)};

	/* calculate rotation matrix */
	float TMPInvRotationMatrix[9]{Object1RotationMatrix[0], Object1RotationMatrix[1], Object1RotationMatrix[2],
				      Object1RotationMatrix[3], Object1RotationMatrix[4], Object1RotationMatrix[5],
				      Object1RotationMatrix[6], Object1RotationMatrix[7], Object1RotationMatrix[8]};
	Matrix33InverseRotate(TMPInvRotationMatrix);
	/* move it to coordinates */
	Matrix33CalcPoint(&TMPMax, TMPInvRotationMatrix);
	Matrix33CalcPoint(&TMPMin, TMPInvRotationMatrix);
	Matrix33CalcPoint(&TMPPoint1, TMPInvRotationMatrix);
	Matrix33CalcPoint(&TMPPoint2, TMPInvRotationMatrix);

	/* same idea as for Sphere-AABB collision detection */
	bool Result{true};
	if ((TMPPoint1.x + Object2Radius < TMPMin.x) ||
	    (TMPPoint1.y + Object2Radius < TMPMin.y) ||
	    (TMPPoint1.z + Object2Radius < TMPMin.z) ||
	    (TMPPoint1.x - Object2Radius > TMPMax.x) ||
	    (TMPPoint1.y - Object2Radius > TMPMax.y) ||
	    (TMPPoint1.z - Object2Radius > TMPMax.z))
		Result = false;

	/* check for distance to line (ray)*/
	if (!Result) {
		/* middle point */
		VECTOR3D mid{(Object2Location + Object2PrevLocation) / 2.0f};
		/* line (ray) direction */
		VECTOR3D dir{Object2Location - Object2PrevLocation};
		/* half of line */
		float hl{dir.Length() / 2.0f};
		dir.Normalize();

		VECTOR3D T{Object1Location - mid};

		/* check axis */
		if ( (fabs(T.x) > TMPMax.x + hl*fabs(dir.x)) ||
		     (fabs(T.y) > TMPMax.y + hl*fabs(dir.y)) ||
		     (fabs(T.z) > TMPMax.z + hl*fabs(dir.z)) )
			return false;

		/* check X ^ dir */
		float r{TMPMax.y*fabs(dir.z) + TMPMax.z*fabs(dir.y)};
		if ( fabs(T.y*dir.z - T.z*dir.y) > r )
			return false;

		/* check  Y ^ dir */
		r = TMPMax.x*fabs(dir.z) + TMPMax.z*fabs(dir.x);
		if ( fabs(T.z*dir.x - T.x*dir.z) > r )
			return false;

		/* check  Z ^ dir */
		r = TMPMax.x*fabs(dir.y) + TMPMax.y*fabs(dir.x);
		if ( fabs(T.x*dir.y - T.y*dir.x) > r )
			return false;

		/* collision detected */
		return true;
	}

	return Result;
}

/*
 * Sphere-Mesh collision detection.
 */
bool vw_SphereMeshCollision(VECTOR3D Object1Location, eObjectBlock *Object1DrawObjectList, float Object1RotationMatrix[9],
			    float Object2Radius, VECTOR3D Object2Location, VECTOR3D Object2PrevLocation,
			    VECTOR3D *CollisionLocation)
{
	if (Object1DrawObjectList == nullptr)
		return false;

	// делаем матрицу перемещения точек, для геометрии
	float TransMat[16]{Object1RotationMatrix[0], Object1RotationMatrix[1], Object1RotationMatrix[2], 0.0f,
			   Object1RotationMatrix[3], Object1RotationMatrix[4], Object1RotationMatrix[5], 0.0f,
			   Object1RotationMatrix[6], Object1RotationMatrix[7], Object1RotationMatrix[8], 0.0f,
			   Object1Location.x, Object1Location.y, Object1Location.z, 1.0f};

	// находим точку локального положения объекта в моделе
	VECTOR3D LocalLocation{Object1DrawObjectList->Location};
	Matrix33CalcPoint(&LocalLocation, Object1RotationMatrix);

	// если нужно - создаем матрицу, иначе - копируем ее
	if ((Object1DrawObjectList->Rotation.x != 0.0f) ||
	    (Object1DrawObjectList->Rotation.y != 0.0f) ||
	    (Object1DrawObjectList->Rotation.z != 0.0f)) {
		float TransMatTMP[16];
		Matrix44Identity(TransMatTMP);
		Matrix44CreateRotate(TransMatTMP, Object1DrawObjectList->Rotation);
		Matrix44Translate(TransMatTMP, LocalLocation);
		// и умножаем на основную матрицу, со сведениями по всему объекту
		Matrix44Mult(TransMat, TransMatTMP);
	} else
		Matrix44Translate(TransMat, LocalLocation);

	// проверяем все треугольники объекта
	for (int i = 0; i < Object1DrawObjectList->VertexCount; i += 3) {
		// находим 3 точки треугольника (с учетом индекс буфера)
		int j2{0};
		if (Object1DrawObjectList->IndexBuffer != nullptr)
			j2 = Object1DrawObjectList->IndexBuffer[Object1DrawObjectList->RangeStart+i]*Object1DrawObjectList->VertexStride;
		else
			j2 = (Object1DrawObjectList->RangeStart+i)*Object1DrawObjectList->VertexStride;

		// находим точки триугольника
		VECTOR3D Point1{Object1DrawObjectList->VertexBuffer[j2],
				Object1DrawObjectList->VertexBuffer[j2 + 1],
				Object1DrawObjectList->VertexBuffer[j2 + 2]};
		Matrix44CalcPoint(&Point1, TransMat);

		if (Object1DrawObjectList->IndexBuffer != nullptr)
			j2 = Object1DrawObjectList->IndexBuffer[Object1DrawObjectList->RangeStart + i + 1]*Object1DrawObjectList->VertexStride;
		else
			j2 = (Object1DrawObjectList->RangeStart + i + 1)*Object1DrawObjectList->VertexStride;

		VECTOR3D Point2{Object1DrawObjectList->VertexBuffer[j2],
				Object1DrawObjectList->VertexBuffer[j2 + 1],
				Object1DrawObjectList->VertexBuffer[j2 + 2]};
		Matrix44CalcPoint(&Point2, TransMat);

		if (Object1DrawObjectList->IndexBuffer != nullptr)
			j2 = Object1DrawObjectList->IndexBuffer[Object1DrawObjectList->RangeStart + i + 2]*Object1DrawObjectList->VertexStride;
		else
			j2 = (Object1DrawObjectList->RangeStart + i + 2)*Object1DrawObjectList->VertexStride;

		VECTOR3D Point3{Object1DrawObjectList->VertexBuffer[j2],
				Object1DrawObjectList->VertexBuffer[j2 + 1],
				Object1DrawObjectList->VertexBuffer[j2 + 2]};
		Matrix44CalcPoint(&Point3, TransMat);

		// находим 2 вектора, образующих плоскость
		VECTOR3D PlaneVector1{Point2 - Point1};
		VECTOR3D PlaneVector2{Point3 - Point1};

		// находим нормаль плоскости
		VECTOR3D NormalVector{PlaneVector1};
		NormalVector.Multiply(PlaneVector2);
		NormalVector.Normalize();

		// проверка, сферы
		//	- в сфере или нет?, т.е. расстояние от плоскости до центра сферы
		//	- по нормале находим точку на плоскости
		//	- точка принадлежит треугольнику?

		// находим расстояние от точки до плоскости
		float Distance{(Object2Location - Point1)*NormalVector};

		// если точка дальше радиуса - нам тут делать нечего, берем следующую точку
		if (fabsf(Distance) <= Object2Radius) {
			// находим реальную точку пересечения с плоскостью
			VECTOR3D IntercPoint{Object2Location - (NormalVector^Distance)};

			// передаем точку и флаг успешной коллизии
			if (PointInTriangle(IntercPoint, Point1, Point2, Point3)) {
				*CollisionLocation = IntercPoint;
				return true;
			}
		}

		// проверка, сферы
		// если от точек до текущего положения расстояние меньше

		float Object2Radius2{Object2Radius*Object2Radius};

		// находим расстояние
		VECTOR3D DistancePoint1{Object2Location - Point1};
		float Distance2Point1{DistancePoint1.x*DistancePoint1.x +
				      DistancePoint1.y*DistancePoint1.y +
				      DistancePoint1.z*DistancePoint1.z};
		if (Distance2Point1 <= Object2Radius2) {
			*CollisionLocation = Point1;
			return true;
		}

		// находим расстояние
		VECTOR3D DistancePoint2{Object2Location - Point2};
		float Distance2Point2{DistancePoint2.x*DistancePoint2.x +
				      DistancePoint2.y*DistancePoint2.y +
				      DistancePoint2.z*DistancePoint2.z};
		if (Distance2Point2 <= Object2Radius2) {
			*CollisionLocation = Point2;
			return true;
		}

		// находим расстояние
		VECTOR3D DistancePoint3{Object2Location - Point3};
		float Distance2Point3{DistancePoint3.x*DistancePoint3.x +
				      DistancePoint3.y*DistancePoint3.y +
				      DistancePoint3.z*DistancePoint3.z};
		if (Distance2Point3 <= Object2Radius2) {
			*CollisionLocation = Point3;
			return true;
		}

		// проверка луч, старое положение - новое положение объекта
		//	- пересекает плоскость (обе точки с одной стороны? знак, или ноль)
		//	- точка пересечения плоскости
		//	- принадлежит треугольнику?

		// проверка, если это фронт-часть, нормально... иначе - берем следующую
		VECTOR3D vDir1{Point1 - Object2PrevLocation};
		float d1{vDir1*NormalVector};
		if (d1 <= 0.001f) {
			// находим расстояние от плоскости до точки
			float originDistance{NormalVector*Point1};

			// пересечение прямой с плоскостью
			VECTOR3D vLineDir{Object2Location - Object2PrevLocation};
			// может и не нужно этого делать... т.к. все и так хорошо работает в нашем случае
			//vLineDir.Normalize();

			float Numerator{ -(NormalVector.x * Object2PrevLocation.x +		// Use the plane equation with the normal and the line
					   NormalVector.y * Object2PrevLocation.y +
					   NormalVector.z * Object2PrevLocation.z - originDistance)};

			float Denominator{NormalVector*vLineDir};
			if(Denominator != 0.0f) {
				float dist{Numerator / Denominator};

				// зная расстояние, находим точку пересечения с плоскостью
				VECTOR3D IntercPoint{Object2PrevLocation + (vLineDir ^ dist)};

				// проверяем, на отрезке или нет (до этого работали с прямой/лучем)
				if (((Object2PrevLocation - IntercPoint)*(Object2Location - IntercPoint) < 0.0f) &&
				    (PointInTriangle(IntercPoint, Point1, Point2, Point3))) {
					// передаем точку и флаг успешной коллизии
					*CollisionLocation = IntercPoint;
					return true;
				}
			}
		}
	}

	return false;
}
