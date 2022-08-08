
#include "umath.h"

#include <math.h>


namespace RoadGen {
	namespace Math {
		void Rotate2DPoint(double inx, double iny, double rx, double ry, double& outx, double& outy, double rad_ang) {
			const double dx = inx - rx;
			const double dy = iny - ry;
			const double cos_a = cos(rad_ang);
			const double sin_a = sin(rad_ang);

			outx = rx + cos_a * dx - sin_a * dy;
			outy = ry + sin_a * dx + cos_a * dy;
		}

		void Rotate2DPointByDegrees(double inx, double iny, double rx, double ry, double& outx, double& outy, double deg_ang) {
			Rotate2DPoint(inx, iny, rx, ry, outx, outy, DegreesToRadians(deg_ang));
		}

		double Get2DDistance(double x1, double y1, double x2, double y2) {
			return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
		}

		double Floor(double x) {
			return floor(x);
		}

		double Ceil(double x) {
			return ceil(x);
		}

		double Fmod(double number, double divider) {
			return fmod(number, divider);
		}

		double Cos(double x) {
			return cos(x);
		}

		double Sin(double x) {
			return sin(x);
		}

		EnPointLinePos2D GetPointLinePos2D(const Point2D& p, const LineEquation2D& l_eq) {
			const double val = l_eq.a * p.x + l_eq.b * p.y + l_eq.c;
			if(IsZeroOrCloseTo(val)) {
				return enPointLinePos2D_BelongsTo;
			}

			return (val > 0) ? enPointLinePos2D_HalfPlanePos : enPointLinePos2D_HalfPlaneNeg;
		}

		double GetNormalizedAngle(double rad_ang) {
			if(rad_ang < -PI || rad_ang >= PI) {
				rad_ang /= 2 * PI;
				double angle_i;
				rad_ang = modf(rad_ang, &angle_i);
				if(rad_ang < -0.5) {
					// plus 2*PI
					rad_ang += 1;

				} else if(rad_ang >= 0.5) {
					// minus 2*PI
					rad_ang -= 1;
				}

				rad_ang *= 2 * PI;
			}

			return rad_ang;
		}
	}
}
