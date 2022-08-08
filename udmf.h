
// UDMF implementation - classes representing map elements and map itself
// UDMF stands for Universal Doom Map Format - map format for Doom-based games
// UDMF map format information: https://zdoom.org/wiki/Universal_Doom_Map_Format
// UDMF specification: https://github.com/ZDoom/gzdoom/blob/master/specs/udmf.txt
// ZDoom extensions: https://github.com/ZDoom/gzdoom/blob/master/specs/udmf_zdoom.txt

#ifndef _ROAD_GEN_UDMF_H_
#define _ROAD_GEN_UDMF_H_

#include "common.h"
#include "umath.h"
#include "io.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDMF data structures and functions

namespace RoadGen {
	namespace Udmf {
		const unsigned int ID_NULL = 0;
		const unsigned int ID_INVALID = ~(unsigned int)0;
		const unsigned int ID_MAX = ~(unsigned int)0 - 1;

		// Vertex - a point on the map, two of them may be linked with a Linedefs
		struct Vertex {
			// 2D coordinates
			double x, y;

			// zfloor, zceiling - z coordinate of the approipriate surface;
			// works only for triangular sectors (if vertex is the part of the bounding triangle)
			bool have_zfloor;
			double zfloor;
			bool have_zceiling;
			double zceiling;

		public:
			Vertex()
				: x(0.0)
				, y(0.0)
				, have_zfloor(false)
				, zfloor(0.0)
				, have_zceiling(false)
				, zceiling(0.0)
			{}

			Vertex(double x_, double y_)
				: x(x_)
				, y(y_)
				, have_zfloor(false)
				, zfloor(0.0)
				, have_zceiling(false)
				, zceiling(0.0)
			{}

			operator Math::Point2D() const {
				return Math::Point2D(x, y);
			}

			Vertex GetMoved(double dx, double dy) const;
			Vertex GetRotated(double cx, double cy, double rad_ang) const;
			Vertex GetRotatedByDegrees(double cx, double cy, int deg_ang) const;
			double GetDistanceTo(const Vertex& v) const;

			Vertex GetWithNoZFloor() const;
			Vertex GetWithZFloor(double zfloor_) const;
			Vertex GetWithNoZCeiling() const;
			Vertex GetWithZCeiling(double zceiling_) const;
			Vertex GetWithNoZ() const;
			Vertex GetWithZ(double zfloor_, double zceiling_) const;
		};

		// Sector - any room on the level, several sidedefs may belong to a Sector.
		struct Sector {
			// floor/ceiling z positions
			int heightfloor, heightceiling;

			// texture ids, strings must be retrieved from elsewhere
			unsigned int texturefloor, textureceiling;

			// light level [0-255]
			unsigned char lightlevel;

			// tag to reference the sector in actions/scripts (id in UDMF)
			unsigned int tag;

			// sector action special
			unsigned int special;

			struct PlaneEquation {
				double cx, cy, cz, c;
				PlaneEquation(): cx(0.0), cy(0.0), cz(0.0), c(0.0) {}
			};

			// if a plane equation is set, then sector will have a slope
			PlaneEquation floor_plane_equation;
			PlaneEquation ceiling_plane_equation;

		public:
			Sector()
				: heightfloor(0)
				, heightceiling(0)
				, texturefloor(ID_NULL)
				, textureceiling(ID_NULL)
				, lightlevel(0)
				, tag(ID_NULL)
				, special(ID_NULL)
			{}

			Sector(int heightfloor_,
				   int heightceiling_,
				   unsigned int texturefloor_,
				   unsigned int textureceiling_,
				   unsigned char lightlevel_,
				   unsigned int tag_ = 0)
				: heightfloor(heightfloor_)
				, heightceiling(heightceiling_)
				, texturefloor(texturefloor_)
				, textureceiling(textureceiling_)
				, lightlevel(lightlevel_)
				, tag(tag_)
				, special(0)
			{}

			Sector GetWithNoFloorPlaneEquation() const;
			Sector GetWithFloorPlaneEquation(double cx, double cy, double cz, double c) const;
			Sector GetWithNoCeilingPlaneEquation() const;
			Sector GetWithCeilingPlaneEquation(double cx, double cy, double cz, double c) const;
			Sector GetWithNoPlaneEquations() const;
			Sector GetWithBothPlaneEquations(double cx, double cy, double cz, double c) const;
		};

		// Sidedef - a sector's wall/bound information; a linedef may have one sidedef or
		// two sidedefs that belong to the same or different sectors
		struct Sidedef {
			// sector reference
			unsigned int sector;

			// textures (IDs, real texture name strings must be retrieved from elsewhere)
			unsigned int texturetop, texturebottom, texturemiddle;

			// texture offsets
			int offsetx, offsety;

		public:
			Sidedef()
				: sector(ID_INVALID)
				, texturetop(ID_NULL)
				, texturebottom(ID_NULL)
				, texturemiddle(ID_NULL)
				, offsetx(0)
				, offsety(0)
			{}

			Sidedef(unsigned int sector_)
				: sector(sector_)
				, texturetop(ID_NULL)
				, texturebottom(ID_NULL)
				, texturemiddle(ID_NULL)
				, offsetx(0)
				, offsety(0)
			{}

			Sidedef(unsigned int sector_,
					unsigned int texturemiddle_)
				: sector(sector_)
				, texturetop(ID_NULL)
				, texturebottom(ID_NULL)
				, texturemiddle(texturemiddle_)
				, offsetx(0)
				, offsety(0)
			{}

			Sidedef(unsigned int sector_,
					unsigned int texturetop_,
					unsigned int texturebottom_)
				: sector(sector_)
				, texturetop(texturetop_)
				, texturebottom(texturebottom_)
				, texturemiddle(ID_NULL)
				, offsetx(0)
				, offsety(0)
			{}

			Sidedef(unsigned int sector_,
					unsigned int texturetop_,
					unsigned int texturebottom_,
					unsigned int texturemiddle_)
				: sector(sector_)
				, texturetop(texturetop_)
				, texturebottom(texturebottom_)
				, texturemiddle(texturemiddle_)
				, offsetx(0)
				, offsety(0)
			{}

			Sidedef GetWithSector(unsigned int sector_) const;
			Sidedef GetWithTopTexture(unsigned int texturetop_) const;
			Sidedef GetWithBottomTexture(unsigned int texturebottom_) const;
			Sidedef GetWithMiddleTexture(unsigned int texturemiddle_) const;
			Sidedef GetWithTopAndBottomTextures(unsigned int texturetop_, unsigned int texturebottom_) const;
			Sidedef GetWithTextures(unsigned int texturetop_, unsigned int texturebottom_, unsigned int texturemiddle_) const;
		};

		// Linedef - a line between two vertices, may have one or two sides (Sidedefs),
		// that belong to the same or different sectors;
		// also may have an action special - some effect that can be triggered by
		// an actor (player, monster, projectile)
		struct Linedef {
			// line vertices and sides
			unsigned int v1, v2;
			unsigned int sidefront, sideback;
		
			// linedef tag, id in UDMF
			unsigned int tag;

			// 'true' means, that line has two sides 'false' - line has one side
			bool twosided;

			// 'true' means, that line can't be passed through. Required with 'twosided = false',
			// otherwise player will be able to move outside the map bounds
			bool blocking;

			// 'true' - line blocks monsters
			bool blockmonsters;

			// 'true' - upper texture will be unpegged
			bool dontpegtop;

			// 'true' - lower texture will be unpegged
			bool dontpegbottom;

			// 'true' - will be shown as an impassable wall on map
			bool secret;

			// 'true' - line is never drawn on the map
			bool dontdraw;

			// 'true' - line appears on the map in the very beginning
			bool mapped;

			struct ActionSpecial {
				// action special id
				unsigned int special;

				// arguments for the line special
				int arg0, arg1, arg2, arg3, arg4;

				// 'true' - action is repeatable, otherwise it can be triggered only once
				bool repeatspecial;

				// 'true' - player can trigger special by pressing the use key
				bool playeruse;
		
				// 'true' - player can trigger special by crossing the line it belongs to
				bool playercross;

				// 'true' - monster can trigger special by crossing the line it belongs to
				bool monstercross;

				// 'true' - monster can trigger special by "using" it
				bool monsteruse;

				// 'true' - special can be triggered by a projectile hit
				bool impact;

				// 'true' - player can trigger special by bumping the line wall
				bool playerpush;

				// 'true' - monster can trigger special by bumping the line wall
				bool monsterpush;

				// 'true' - special can be triggered by a projectile crossing the line
				bool missilecross;

				ActionSpecial()
					: special(ID_NULL)
					, arg0(0)
					, arg1(0)
					, arg2(0)
					, arg3(0)
					, arg4(0)
					, repeatspecial(false)
					, playeruse(false)
					, playercross(false)
					, monstercross(false)
					, monsteruse(false)
					, impact(false)
					, playerpush(false)
					, monsterpush(false)
					, missilecross(false)
				{}

				ActionSpecial(unsigned int special_,
							  int arg0_ = 0,
							  int arg1_ = 0,
							  int arg2_ = 0,
							  int arg3_ = 0,
							  int arg4_ = 0,
							  bool repeatspecial_ = false,
							  bool playeruse_ = false)
					: special(special_)
					, arg0(arg0_)
					, arg1(arg1_)
					, arg2(arg2_)
					, arg3(arg3_)
					, arg4(arg4_)
					, repeatspecial(repeatspecial_)
					, playeruse(playeruse_)
					, playercross(false)
					, monstercross(false)
					, monsteruse(false)
					, impact(false)
					, playerpush(false)
					, monsterpush(false)
					, missilecross(false)
				{}
			};

			// can be triggered by a player/monster/projectle to create an effect on the map;
			// some action specials are static and works since the map is loaded (e.g. texture scrolling)
			ActionSpecial action_special;

		public:
			Linedef()
				: v1(ID_INVALID)
				, v2(ID_INVALID)
				, sidefront(ID_INVALID)
				, sideback(ID_INVALID)
				, tag(ID_NULL)
				, twosided(false)
				, blocking(false)
				, blockmonsters(false)
				, dontpegtop(false)
				, dontpegbottom(false)
				, secret(false)
				, dontdraw(false)
				, mapped(false)
			{}

			Linedef(unsigned int v1_,
					unsigned int v2_,
					unsigned int sidefront_,
					unsigned int sideback_ = ID_INVALID)
				: v1(v1_)
				, v2(v2_)
				, sidefront(sidefront_)
				, sideback(sideback_)
				, tag(ID_NULL)
				, twosided(sideback_ != ID_INVALID)
				, blocking(sideback_ == ID_INVALID)
				, blockmonsters(false)
				, dontpegtop(false)
				, dontpegbottom(false)
				, secret(false)
				, dontdraw(false)
				, mapped(false)
			{}

			Linedef GetFlipped() const;
		};

		// Map - full UDMF map definition;
		// maps also contain Things, but only player start is implemented here
		struct Map {
		public:
			std::vector<Vertex> vertices;
			std::vector<Linedef> linedefs;
			std::vector<Sidedef> sidedefs;
			std::vector<Sector> sectors;

			std::vector<std::string> textures;

			// a vector of things with player start thing
			// should be instead of these variables
			struct PlayerStartInfo {
				double x, y;
				int deg_angle;

				PlayerStartInfo()
					: x(0.0)
					, y(0.0)
					, deg_angle(0)
				{}

				PlayerStartInfo(double x_, double y_, int deg_angle_)
					: x(x_)
					, y(y_)
					, deg_angle(deg_angle_)
				{}
			};

			bool have_player_start;
			PlayerStartInfo player_start_info;
			Map()
				: have_player_start(false)
			{}
		};

		unsigned int operator<< (Map& map, const Vertex& v);
		unsigned int operator<< (Map& map, const Linedef& l);
		unsigned int operator<< (Map& map, const Sidedef& sd);
		unsigned int operator<< (Map& map, const Sector& s);
		void operator<< (Map& map, const Map::PlayerStartInfo& player_start_info);

		enum EnMapCtrl {
			enMapCtrl_Null = 0,
			enMapCtrl_RemovePlayerPos
		};
		void operator<< (Map& map, EnMapCtrl ctrl);

		void WriteMapToStream(const Map& map, unsigned char float_precision, Io::OutStream& out_stream);
	}
}

#endif // _ROAD_GEN_UDMF_H_
