
// user math utilities

#ifndef _ROAD_GEN_UMATH_H_
#define _ROAD_GEN_UMATH_H_

namespace RoadGen {
	namespace Math {
		const double DBL_EPS = 0.00001;

		const double PI = 3.1415926535;

		inline double DegreesToRadians(double degrees) { return degrees * PI / 180.0; }

		inline double RadiansToDegrees(double radians) { return radians / PI * 180.0; }

		void Rotate2DPoint(double inx, double iny, double rx, double ry, double& outx, double& outy, double rad_ang);

		void Rotate2DPointByDegrees(double inx, double iny, double rx, double ry, double& outx, double& outy, double deg_ang);

		double Get2DDistance(double x1, double y1, double x2, double y2);

		inline double Get2DDistanceSquared(double x1, double y1, double x2, double y2) {
			return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
		}

		double Floor(double x);

		double Ceil(double x);

		double Fmod(double number, double divider);

		double Cos(double x);

		double Sin(double x);

		struct Point2D {
			double x, y;

			Point2D(): x(0.0), y(0.0) {}
			Point2D(double x_, double y_): x(x_), y(y_) {}
		};

		struct LineEquation2D {
			double a, b, c;

			LineEquation2D()
				: a(0.0)
				, b(0.0)
				, c(0.0)
			{}
		};

		enum EnPointLinePos2D {
			enPointLinePos2D_BelongsTo = 0,
			enPointLinePos2D_HalfPlanePos = 1,
			enPointLinePos2D_HalfPlaneNeg = -1
		};

		inline double Get2DDistanceSquared(const Point2D& p1, const Point2D& p2) {
			return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
		}

		EnPointLinePos2D GetPointLinePos2D(const Point2D& p, const LineEquation2D& l_eq);

		// any real number angle value in radians -> [-PI, PI) angle value
		double GetNormalizedAngle(double rad_ang);

		inline bool IsZeroOrCloseTo(double val) {
			return val > -DBL_EPS && val < DBL_EPS;
		}
	}
}

#endif // _ROAD_GEN_UMATH_H_
