
#include "udmf.h"
#include "umath.h"

#include <iomanip>
#include <sstream>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex implementation

namespace RoadGen {
	namespace Udmf {
		Vertex Vertex::GetMoved(double dx, double dy) const {
			Vertex v(*this);
			v.x += dx;
			v.y += dy;
			return v;
		}

		Vertex Vertex::GetRotated(double cx, double cy, double rad_ang) const {
			Vertex v(*this);
			Math::Rotate2DPoint(x, y, cx, cy, v.x, v.y, rad_ang);
			return v;
		}

		Vertex Vertex::GetRotatedByDegrees(double cx, double cy, int deg_ang) const {
			Vertex v(*this);
			Math::Rotate2DPointByDegrees(x, y, cx, cy, v.x, v.y, deg_ang);
			return v;
		}

		double Vertex::GetDistanceTo(const Vertex& v) const {
			const double distance = Math::Get2DDistance(x, y, v.x, v.y);
			return distance;
		}

		Vertex Vertex::GetWithNoZFloor() const {
			Vertex v(*this);
			v.have_zfloor = false;
			v.zfloor = 0.0;
			return v;
		}

		Vertex Vertex::GetWithZFloor(double zfloor_) const {
			Vertex v(*this);
			v.have_zfloor = true;
			v.zfloor = zfloor_;
			return v;
		}

		Vertex Vertex::GetWithNoZCeiling() const {
			Vertex v(*this);
			v.have_zceiling = false;
			v.zceiling = 0.0;
			return v;
		}

		Vertex Vertex::GetWithZCeiling(double zceiling_) const {
			Vertex v(*this);
			v.have_zceiling = true;
			v.zceiling = zceiling_;
			return v;
		}

		Vertex Vertex::GetWithNoZ() const {
			Vertex v(*this);
			v.have_zfloor = false;
			v.zfloor = 0.0;
			v.have_zceiling = false;
			v.zceiling = 0.0;
			return v;
		}

		Vertex Vertex::GetWithZ(double zfloor_, double zceiling_) const {
			Vertex v(*this);
			v.have_zfloor = true;
			v.zfloor = zfloor_;
			v.have_zceiling = true;
			v.zceiling = zceiling_;
			return v;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sector implementation

namespace RoadGen {
	namespace Udmf {
		Sector Sector::GetWithNoFloorPlaneEquation() const {
			Sector s(*this);
			s.floor_plane_equation = PlaneEquation();
			return s;
		}

		Sector Sector::GetWithFloorPlaneEquation(double cx, double cy, double cz, double c) const {
			Sector s(*this);
			s.floor_plane_equation.cx = cx;
			s.floor_plane_equation.cy = cy;
			s.floor_plane_equation.cz = cz;
			s.floor_plane_equation.c = c;
			return s;
		}

		Sector Sector::GetWithNoCeilingPlaneEquation() const {
			Sector s(*this);
			s.ceiling_plane_equation = PlaneEquation();
			return s;
		}

		Sector Sector::GetWithCeilingPlaneEquation(double cx, double cy, double cz, double c) const {
			Sector s(*this);
			s.ceiling_plane_equation.cx = cx;
			s.ceiling_plane_equation.cy = cy;
			s.ceiling_plane_equation.cz = cz;
			s.ceiling_plane_equation.c = c;
			return s;
		}

		Sector Sector::GetWithNoPlaneEquations() const {
			Sector s(*this);
			s.floor_plane_equation = PlaneEquation();
			s.ceiling_plane_equation = PlaneEquation();
			return s;
		}

		Sector Sector::GetWithBothPlaneEquations(double cx, double cy, double cz, double c) const {
			Sector s(*this);

			s.floor_plane_equation.cx = cx;
			s.floor_plane_equation.cy = cy;
			s.floor_plane_equation.cz = cz;
			s.floor_plane_equation.c = c;

			s.ceiling_plane_equation.cx = cx;
			s.ceiling_plane_equation.cy = cy;
			s.ceiling_plane_equation.cz = cz;
			s.ceiling_plane_equation.c = c;

			return s;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sidedef implementation

namespace RoadGen {
	namespace Udmf {
		Sidedef Sidedef::GetWithSector(unsigned int sector_) const {
			Sidedef sd(*this);
			sd.sector = sector_;
			return sd;
		}

		Sidedef Sidedef::GetWithTopTexture(unsigned int texturetop_) const {
			Sidedef sd(*this);
			sd.texturetop = texturetop_;
			return sd;
		}

		Sidedef Sidedef::GetWithBottomTexture(unsigned int texturebottom_) const {
			Sidedef sd(*this);
			sd.texturebottom = texturebottom_;
			return sd;
		}

		Sidedef Sidedef::GetWithMiddleTexture(unsigned int texturemiddle_) const {
			Sidedef sd(*this);
			sd.texturemiddle = texturemiddle_;
			return sd;
		}

		Sidedef Sidedef::GetWithTopAndBottomTextures(unsigned int texturetop_, unsigned int texturebottom_) const {
			Sidedef sd(*this);
			sd.texturetop = texturetop_;
			sd.texturebottom = texturebottom_;
			return sd;
		}

		Sidedef Sidedef::GetWithTextures(unsigned int texturetop_, unsigned int texturebottom_, unsigned int texturemiddle_) const {
			Sidedef sd(*this);
			sd.texturetop = texturetop_;
			sd.texturebottom = texturebottom_;
			sd.texturemiddle = texturemiddle_;
			return sd;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Linedef implementation

namespace RoadGen {
	namespace Udmf {
		Linedef Linedef::GetFlipped() const {
			Linedef l(*this);
			l.v1 = v2;
			l.v2 = v1;
			l.sidefront = sideback;
			l.sideback = sidefront;
			return l;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Map implementation

namespace RoadGen {
	namespace Udmf {
		template< typename T, typename A=std::vector<T> >
		static unsigned int AddMapEntryT(A& entries, const T& new_entry) {
			if(entries.size() > ID_MAX) {
				throw Exception("error constructing UDMF map - too many elements in UDMF map");
			}

			const unsigned int id = (unsigned int)entries.size();
			entries.push_back(new_entry);
			return id;
		}

		unsigned int operator<< (Map& map, const Vertex& v) {
			return AddMapEntryT(map.vertices, v);
		}

		unsigned int operator<< (Map& map, const Linedef& l) {
			return AddMapEntryT(map.linedefs, l);
		}

		unsigned int operator<< (Map& map, const Sidedef& sd) {
			return AddMapEntryT(map.sidedefs, sd);
		}

		unsigned int operator<< (Map& map, const Sector& s) {
			return AddMapEntryT(map.sectors, s);
		}

		void operator<< (Map& map, const Map::PlayerStartInfo& player_start_info) {
			map.have_player_start = true;
			map.player_start_info = player_start_info;
		}

		void operator<< (Map& map, EnMapCtrl ctrl) {
			switch(ctrl) {
			case enMapCtrl_Null:
				break;

			case enMapCtrl_RemovePlayerPos:
				map.have_player_start = false;
				map.player_start_info = Map::PlayerStartInfo();
				break;

			default:
				throw Exception("error constructing UDMF map - unknown control symbol");
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WriteMapToStream implementation

namespace RoadGen {
	namespace Udmf {
		void WriteMapToStream(const Map& map, unsigned char float_precision, Io::OutStream& out_stream) {
			class Utils {
				std::stringstream m_ss;
				double m_eps;

			public:
				Utils(unsigned char precision) {
					m_ss << std::fixed << std::setprecision(precision);
					m_eps = 1;
					const unsigned char max_ix = (precision < 7) ? precision : 7;
					for(unsigned char ix=0; ix<max_ix; ix++) {
						m_eps /= 10;
					}
				}

				std::string FormatDoubleStr(double number) {
					m_ss.str("");
					m_ss << number;
					return m_ss.str();
				}

				bool IsDoubleNonZero(double value) {
					return value <= -m_eps || value >= m_eps;
				}
			} utils(float_precision);

			out_stream << "namespace = \"zdoom\";\n\n";

			// add things (now it is only player start)
			if(map.have_player_start) {
				out_stream << "thing // 0\n"
							<< "{\n"
							<< "x = " << utils.FormatDoubleStr(map.player_start_info.x) << ";\n"
							<< "y = " << utils.FormatDoubleStr(map.player_start_info.y) << ";\n"
							<< "angle = " << map.player_start_info.deg_angle << ";\n"
							<< "type = 1;\n"
							<< "skill1 = true;\n"
							<< "skill2 = true;\n"
							<< "skill3 = true;\n"
							<< "skill4 = true;\n"
							<< "skill5 = true;\n"
							<< "skill6 = true;\n"
							<< "skill7 = true;\n"
							<< "skill8 = true;\n"
							<< "single = true;\n"
							<< "coop = true;\n"
							<< "dm = true;\n"
							<< "class1 = true;\n"
							<< "class2 = true;\n"
							<< "class3 = true;\n"
							<< "class4 = true;\n"
							<< "class5 = true;\n"
							<< "class6 = true;\n"
							<< "class7 = true;\n"
							<< "class8 = true;\n"
							<< "}\n"
							<< "\n";
			}

			// add vertices
			for(unsigned int vix=0; vix<map.vertices.size(); vix++) {
				const Vertex& v = map.vertices[vix];
				if(   v.x < -32768.0 || v.x > 32767.0
				   || v.y < -32768.0 || v.y > 32767.0)
				{
					throw Exception("error writing UDMF map to stream - vertex " + std::to_string(vix) + " has bad coordinates (" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")");
				}

				out_stream << "vertex // " << vix << "\n"
							<< "{\n"
							<< "x = " << utils.FormatDoubleStr(v.x) << ";\n"
							<< "y = " << utils.FormatDoubleStr(v.y) << ";\n";

				if(v.have_zfloor) {
					out_stream << "zfloor = " << utils.FormatDoubleStr(v.zfloor) << ";\n";
				}

				if(v.have_zceiling) {
					out_stream << "zceiling = " << utils.FormatDoubleStr(v.zceiling) << ";\n";
				}

				out_stream << "}\n"
							<< "\n";
			}

			// add linedefs
			for(unsigned int lix=0; lix<map.linedefs.size(); lix++) {
				const Linedef& l = map.linedefs[lix];

				out_stream << "linedef // " << lix << "\n"
							<< "{\n";

				if(l.v1 >= map.vertices.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, linedef " + std::to_string(lix) + " has number of non-existing vertex "
									+ std::to_string(l.v1) + " set as v1");
				}
				out_stream << "v1 = " << l.v1 << ";\n";

				if(l.v2 >= map.vertices.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, linedef " + std::to_string(lix) + " has number of non-existing vertex "
									+ std::to_string(l.v2) + " set as v2");
				}
				out_stream << "v2 = " << l.v2 << ";\n";

				if(l.sidefront >= map.sidedefs.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, linedef " + std::to_string(lix) + " has number of non-existing sidedef "
									+ std::to_string(l.sidefront) + " set as sidefront");
				}
				out_stream << "sidefront = " << l.sidefront << ";\n";

				if(l.sideback != ID_INVALID) {
					if(l.sideback >= map.sidedefs.size()) {
						throw Exception("error writing UDMF map to stream - bad UDMF map, linedef " + std::to_string(lix) + " has number of non-existing sidedef "
										+ std::to_string(l.sideback) + " set as sideback");
					}
					out_stream << "sideback = " << l.sideback << ";\n";
				}

				if(l.tag != ID_NULL) {
					out_stream << "id = " << l.tag << ";\n";
				}

				if(l.twosided) {
					out_stream << "twosided = true;\n";
				}

				if(l.blocking) {
					out_stream << "blocking = true;\n";
				}

				if(l.blockmonsters) {
					out_stream << "blockmonsters = true;\n";
				}

				if(l.dontpegtop) {
					out_stream << "dontpegtop = true;\n";
				}

				if(l.dontpegbottom) {
					out_stream << "dontpegbottom = true;\n";
				}

				if(l.secret) {
					out_stream << "secret = true;\n";
				}

				if(l.dontdraw) {
					out_stream << "dontdraw = true;\n";
				}

				if(l.mapped) {
					out_stream << "mapped = true;\n";
				}

				if(l.action_special.special != ID_NULL) {
					const Linedef::ActionSpecial& asp = l.action_special;
					out_stream << "special = " << asp.special << ";\n";

					if(asp.arg0 != 0) {
						out_stream << "arg0 = " << asp.arg0 << ";\n";
					}

					if(asp.arg1 != 0) {
						out_stream << "arg1 = " << asp.arg1 << ";\n";
					}

					if(asp.arg2 != 0) {
						out_stream << "arg2 = " << asp.arg2 << ";\n";
					}

					if(asp.arg3 != 0) {
						out_stream << "arg3 = " << asp.arg3 << ";\n";
					}

					if(asp.arg4 != 0) {
						out_stream << "arg4 = " << asp.arg4 << ";\n";
					}

					if(asp.repeatspecial) {
						out_stream << "repeatspecial = true;\n";
					}

					if(asp.playeruse) {
						out_stream << "playeruse = true;\n";
					}

					if(asp.playercross) {
						out_stream << "playercross = true;\n";
					}

					if(asp.monstercross) {
						out_stream << "monstercross = true;\n";
					}

					if(asp.monsteruse) {
						out_stream << "monsteruse = true;\n";
					}

					if(asp.impact) {
						out_stream << "impact = true;\n";
					}

					if(asp.playerpush) {
						out_stream << "playerpush = true;\n";
					}

					if(asp.monsterpush) {
						out_stream << "monsterpush = true;\n";
					}

					if(asp.missilecross) {
						out_stream << "missilecross = true;\n";
					}
				}

				out_stream << "}\n"
							<< "\n";
			}

			// add sidedefs
			for(unsigned int sdix=0; sdix<map.sidedefs.size(); sdix++) {
				const Sidedef& sd = map.sidedefs[sdix];

				out_stream << "sidedef // " << sdix << "\n"
							<< "{\n";
				if(sd.sector >= map.sectors.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, sidedef " + std::to_string(sdix) + " has number of non-existing sector "
									+ std::to_string(sd.sector) + " set as sector");
				}
				out_stream << "sector = " << sd.sector << ";\n";

				// do not put "-" texture name,
				// it is the default value and may be skipped
				if(sd.texturetop != ID_NULL) {
					if(sd.texturetop >= map.textures.size()) {
						throw Exception("error writing UDMF map to stream - bad UDMF map, sidedef " + std::to_string(sdix) + " has id of non-existing texture "
										+ std::to_string(sd.texturetop) + " set as texturetop");
					}
					out_stream << "texturetop = \"" << map.textures[sd.texturetop] << "\";\n";
				}

				if(sd.texturebottom != ID_NULL) {
					if(sd.texturebottom >= map.textures.size()) {
						throw Exception("error writing UDMF map to stream - bad UDMF map, sidedef " + std::to_string(sdix) + " has id of non-existing texture "
										+ std::to_string(sd.texturebottom) + " set as texturebottom");
					}
					out_stream << "texturebottom = \"" << map.textures[sd.texturebottom] << "\";\n";
				}

				if(sd.texturemiddle != ID_NULL) {
					if(sd.texturemiddle >= map.textures.size()) {
						throw Exception("error writing UDMF map to stream - bad UDMF map, sidedef " + std::to_string(sdix) + " has id of non-existing texture "
										+ std::to_string(sd.texturemiddle) + " set as texturemiddle");
					}
					out_stream << "texturemiddle = \"" << map.textures[sd.texturemiddle] << "\";\n";
				}

				if(sd.offsetx != 0) {
					out_stream << "offsetx = " << sd.offsetx << ";\n";
				}

				if(sd.offsety != 0) {
					out_stream << "offsety = " << sd.offsety << ";\n";
				}

				out_stream << "}\n"
						   << "\n";
			}

			// add sectors
			for(unsigned int six=0; six<map.sectors.size(); six++) {
				const Sector& s = map.sectors[six];

				out_stream << "sector // " << six << "\n"
							<< "{\n";

				out_stream << "heightfloor = " << s.heightfloor << ";\n";
				out_stream << "heightceiling = " << s.heightceiling << ";\n";

				if(s.texturefloor >= map.textures.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, sector " + std::to_string(six) + " has id of non-existing texture "
									+ std::to_string(s.texturefloor) + " set as texturefloor");
				}
				out_stream << "texturefloor = \"" << map.textures[s.texturefloor] << "\";\n";

				if(s.textureceiling >= map.textures.size()) {
					throw Exception("error writing UDMF map to stream - bad UDMF map, sector " + std::to_string(six) + " has id of non-existing texture "
									+ std::to_string(s.textureceiling) + " set as textureceiling");
				}
				out_stream << "textureceiling = \"" << map.textures[s.textureceiling] << "\";\n";

				if(s.lightlevel != 160) {
					out_stream << "lightlevel = " << (int)s.lightlevel << ";\n";
				}

				if(s.special != ID_NULL) {
					out_stream << "special = " << s.special << ";\n";
				}

				if(s.tag != ID_NULL) {
					out_stream << "id = " << s.tag << ";\n";
				}

				if(   utils.IsDoubleNonZero(s.floor_plane_equation.cx)
				   || utils.IsDoubleNonZero(s.floor_plane_equation.cy)
				   || utils.IsDoubleNonZero(s.floor_plane_equation.cz))
				{
					out_stream << "floorplane_a = " << utils.FormatDoubleStr(s.floor_plane_equation.cx) << ";\n";
					out_stream << "floorplane_b = " << utils.FormatDoubleStr(s.floor_plane_equation.cy) << ";\n";
					out_stream << "floorplane_c = " << utils.FormatDoubleStr(s.floor_plane_equation.cz) << ";\n";
					out_stream << "floorplane_d = " << utils.FormatDoubleStr(s.floor_plane_equation.c) << ";\n";
				}

				if(   utils.IsDoubleNonZero(s.ceiling_plane_equation.cx)
				   || utils.IsDoubleNonZero(s.ceiling_plane_equation.cy)
				   || utils.IsDoubleNonZero(s.ceiling_plane_equation.cz))
				{
					out_stream << "ceilingplane_a = " << utils.FormatDoubleStr(s.ceiling_plane_equation.cx) << ";\n";
					out_stream << "ceilingplane_b = " << utils.FormatDoubleStr(s.ceiling_plane_equation.cy) << ";\n";
					out_stream << "ceilingplane_c = " << utils.FormatDoubleStr(s.ceiling_plane_equation.cz) << ";\n";
					out_stream << "ceilingplane_d = " << utils.FormatDoubleStr(s.ceiling_plane_equation.c) << ";\n";
				}

				out_stream << "}\n"
						   << "\n";
			}
		}
	}
}
