
// all classes and functions related to actual road generation

#ifndef _ROAD_GEN_CORE_H_
#define _ROAD_GEN_CORE_H_

#include "common.h"
#include "udmf.h"
#include "umath.h"
#include "io.h"

#include <memory>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoadConfig - structure that contains all road parameters (width, mark length, etc.)

namespace RoadGen {
	struct RoadConfig {
		struct Sizes {
			double background_dist;
			int fence_height;
			double road_width, road_side_width;
			int road_side_height;
			double road_mark_width, road_mark_length, road_mark_gap;

			Sizes()
				: background_dist(0.0)
				, fence_height(0)
				, road_width(0.0)
				, road_side_width(0.0)
				, road_side_height(0)
				, road_mark_width(0.0)
				, road_mark_length(0.0)
				, road_mark_gap(0.0)
			{}
		} sizes;

		enum EnTexture {
			enTexture_Null = 0,
			enTexture_Sky,
			enTexture_Background,
			enTexture_Fence,
			enTexture_FenceFloor,
			enTexture_RoadSide,
			enTexture_RoadSideWall,
			enTexture_RoadBody,
			enTexture_RoadMark,

			EnTexture_Size
		};
		std::string textures[EnTexture_Size];

		unsigned char light_level;

		RoadConfig()
			: light_level(0)
		{
			textures[enTexture_Null] = "-";
		}
	};

	void GetRoadConfig(const Io::ConfigData& config_data, RoadConfig& config);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IRoadFigureObject - common interface for objects, that are consumed by RoadFigure,
// this includes basic shapes (lines, arcs) and modifiers (start and slope modifiers)

namespace RoadGen {
	class IRoadFigureObject {
	public:
		enum EnType {
			enType_StartModifier = 0,
			enType_Line,
			enType_Arc,
			enType_SlopeModifier
		};

		virtual ~IRoadFigureObject() {}

		// return the only type this object offers data for
		virtual EnType GetType() const = 0;

		struct StartModifierData {
			static const EnType obj_type_id = enType_StartModifier;

			double x_null, y_null;
			double angle;
			int zpos;
			int height;
			double mark_shift;

			StartModifierData()
				: x_null(0.0)
				, y_null(0.0)
				, angle(0.0)
				, zpos(0)
				, height(0)
				, mark_shift(0.0)
			{}
		};
		virtual StartModifierData* GetStartModifierData() = 0;
		virtual const StartModifierData* GetStartModifierData() const = 0;

		struct LineData {
			static const EnType obj_type_id = enType_Line;

			double length;

			LineData()
				: length(0.0)
			{}
		};
		virtual LineData* GetLineData() = 0;
		virtual const LineData* GetLineData() const = 0;
		
		struct ArcData {
			static const EnType obj_type_id = enType_Arc;

			double curve;
			double angle;
			int divider;

			ArcData()
				: curve(0.0)
				, angle(0.0)
				, divider(0)
			{}
		};
		virtual ArcData* GetArcData() = 0;
		virtual const ArcData* GetArcData() const = 0;
		
		struct SlopeModifierData {
			static const EnType obj_type_id = enType_SlopeModifier;

			double tangent;

			SlopeModifierData()
				: tangent(0.0)
			{}
		};
		virtual SlopeModifierData* GetSlopeModifierData() = 0;
		virtual const SlopeModifierData* GetSlopeModifierData() const = 0;
	};

	template<typename T>
	class RoadFigureObjectT: public IRoadFigureObject {
	public:
		T data;

		EnType GetType() const {
			return T::obj_type_id;
		}

		StartModifierData* GetStartModifierData() {
			return GetData<StartModifierData>();
		}

		const StartModifierData* GetStartModifierData() const {
			return GetData<StartModifierData>();
		}

		LineData* GetLineData() {
			return GetData<LineData>();
		}

		const LineData* GetLineData() const {
			return GetData<LineData>();
		}

		ArcData* GetArcData() {
			return GetData<ArcData>();
		}

		const ArcData* GetArcData() const {
			return GetData<ArcData>();
		}

		SlopeModifierData* GetSlopeModifierData() {
			return GetData<SlopeModifierData>();
		}

		const SlopeModifierData* GetSlopeModifierData() const {
			return GetData<SlopeModifierData>();
		}

	private:
		template<typename G>
		G* GetData() {
			return static_cast<G*>(ThrowOnBadDataType<G>());
		}

		template<>
		T* GetData<T>() {
			return &data;
		}

		template<typename G>
		const G* GetData() const {
			return static_cast<const G*>(ThrowOnBadDataType<G>());
		}

		template<>
		const T* GetData<T>() const {
			return &data;
		}

		template<typename G>
		void* ThrowOnBadDataType() const {
			static_assert(G::obj_type_id != T::obj_type_id,
				"undefined data for a road figure object is specified - please, do not use a custom data struct for this template");
			throw Exception("error generating a road - wrong type of road figure object is picked");
		}
	};

	typedef RoadFigureObjectT<IRoadFigureObject::StartModifierData> RoadFigureStartModifier;
	typedef RoadFigureObjectT<IRoadFigureObject::LineData> RoadFigureLine;
	typedef RoadFigureObjectT<IRoadFigureObject::ArcData> RoadFigureArc;
	typedef RoadFigureObjectT<IRoadFigureObject::SlopeModifierData> RoadFigureSlopeModifier;

	typedef std::unique_ptr<IRoadFigureObject> IRoadFigureObjectPtr;
	typedef std::vector<IRoadFigureObjectPtr> RoadFigureData;
	typedef std::unique_ptr<const IRoadFigureObject> IRoadFigureObjectConstPtr;
	typedef std::vector<IRoadFigureObjectConstPtr> RoadFigureDataConst;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IntersectionChecker - checks 2D figures for intersections

namespace RoadGen {
	class IntersectionChecker {
	private:
		using Point = Math::Point2D;
		using LineEquation = Math::LineEquation2D;
		using EnPointLinePos = Math::EnPointLinePos2D;

		// info about a line segment of a quad
		struct LineSegmentInfo {
			// equation of the line containing this segment
			LineEquation line_equation;

			// line segment length squared
			double length_squared;

			// position of other two points related
			// to this segment's line (all our quads are convex)
			EnPointLinePos other_points_pos;

			LineSegmentInfo()
				: length_squared(0.0)
				, other_points_pos(Math::enPointLinePos2D_BelongsTo)
			{}
		};

		// a quad info (all our quads are convex)
		struct QuadInfo {
			Point vertices[4];
			LineSegmentInfo segments[4];

			// NULL only if points are not initialized
			bool IsNull() const {
				return     vertices[0].x == 0.0 && vertices[0].y == 0.0
						&& vertices[1].x == 0.0 && vertices[1].y == 0.0
						&& vertices[2].x == 0.0 && vertices[2].y == 0.0
						&& vertices[3].x == 0.0 && vertices[3].y == 0.0;
			}
		};

	public:
		IntersectionChecker()
			: m_new_figure(true)
		{}

		void AddQuad(const Math::Point2D& p1, const Math::Point2D& p2, const Math::Point2D& p3, const Math::Point2D& p4);

		void StartEncirclingQuad();

		void EndEncirclingQuad();

		void CutFigure();

	private:
		static QuadInfo GetQuadInfo(const Point& p1, const Point& p2, const Point& p3, const Point& p4);

		bool QuadDoesNotIntersectOthers(const QuadInfo& quad_to_check_info) const;

		static bool QuadsHaveCommonPoints(const QuadInfo& quad_info1, const QuadInfo& quad_info2);

		bool HavePendingEncirclingQuad() const;

	private:
		typedef std::vector<QuadInfo> t_quad_array;
		typedef std::unique_ptr<t_quad_array> t_quad_array_ptr;

		// some quads with a quad [exactly] encircling them all
		typedef std::pair<QuadInfo, t_quad_array_ptr> t_quad_with_subquads;
		std::vector<t_quad_with_subquads> m_quads;
		bool m_new_figure;
	};
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoadFigure - single road segment; may be built using arcs or lines with the support of slopes

namespace RoadGen {
	typedef std::pair<unsigned int, unsigned int> IdPair;

	class RoadFigure {
		using Vertex = Udmf::Vertex;
		using Linedef = Udmf::Linedef;
		using Sidedef = Udmf::Sidedef;
		using Sector = Udmf::Sector;

		using StartModifier = IRoadFigureObject::StartModifierData;
		using SlopeModifier = IRoadFigureObject::SlopeModifierData;

		// background sector thickness
		static const int BACKGROUND_THICKNESS = 16;

		// distance between fence and the border of the map
		static const int FENCE_BACKGROUND_GAP = 32;

		// fence sector thickness
		static const int FENCE_THICKNESS = 16;

		// mark sector height
		static const int MARK_HEIGHT = 1;

		static const int SMALLEST_ARC_LINE = 4;
		static const int SMALLEST_MARK_LINE = 4;

	public:
		RoadFigure(const RoadConfig& config,
				   const IRoadFigureObject::StartModifierData& start_data,
				   Udmf::Map& map,
				   IntersectionChecker* intersection_checker = NULL)
			: m_config(config)
			, m_map(map)
			, m_intersection_checker(intersection_checker)
			, m_total_width(2 * (BACKGROUND_THICKNESS + config.sizes.background_dist + config.sizes.road_side_width) + config.sizes.road_width)
			, m_floorpos(start_data.zpos)
			, m_ceilingpos(start_data.zpos + start_data.height)
			, m_angle(start_data.angle)
			, m_mark_coord(0.0)
			, m_v_null(start_data.x_null, start_data.y_null)
			, m_started_drawing(false)
			, m_finished_drawing(false)
			, m_have_shapes(false)
			, m_have_shape_with_current_slope(false)
			, m_sectors_closed(true)
			, m_mark_sector_closed(true)
		{
			m_angle = Math::GetNormalizedAngle(m_angle);

			m_mark_coord = Math::Fmod(start_data.mark_shift, config.sizes.road_mark_length + config.sizes.road_mark_gap);
			if(m_mark_coord < 0) {
				m_mark_coord += config.sizes.road_mark_length + config.sizes.road_mark_gap;
			}

			InitMapElements();
		}

		void Start();
		void Finish();

		RoadFigure& operator<< (const IRoadFigureObject::LineData& line);
		RoadFigure& operator<< (const IRoadFigureObject::ArcData& arc);
		RoadFigure& operator<< (const IRoadFigureObject::SlopeModifierData& slope_modifier);

	private:
		void InitMapElements();

		static void GetMovedVertices(const std::vector<Vertex*>& source_vertices,
									 const Vertex& v_prev_null,
									 const Vertex& v_null,
									 double angle,
									 std::unordered_map<Vertex*, Vertex>& vertices);

		void SetNullVertex(const Vertex& v_new_null);

		void ExtendBackground(const Vertex& v_new_west_left,
							  const Vertex& v_new_west_right,
							  const Vertex& v_new_east_left,
							  const Vertex& v_new_east_right);

		void ExtendFence(const Vertex& v_new_west_left,
						 const Vertex& v_new_west_right,
						 const Vertex& v_new_east_left,
						 const Vertex& v_new_east_right);

		void ExtendBody(const Vertex& v_new_west_left,
						const Vertex& v_new_west_right,
						const Vertex& v_new_east_left,
						const Vertex& v_new_east_right);

		void ExtendMark(const Vertex& v_new_mark_west,
						const Vertex& v_new_mark_east);

		void CloseSectors();

	private:
		const RoadConfig& m_config;
		Udmf::Map& m_map;
		IntersectionChecker* const m_intersection_checker;
		const double m_total_width;

		int m_floorpos, m_ceilingpos;
		double m_angle;

		// range: [0, road_mark_length + road_mark_gap)
		// [0, road_mark_length) - drawing a mark
		// [road_mark_length, road_mark_length + road_mark_gap) - leaving a gap
		double m_mark_coord;

		// center vertex, represents current position of the drawing
		Vertex m_v_null;

		SlopeModifier m_slope_modifier;

		bool m_started_drawing;
		bool m_finished_drawing;
		bool m_have_shapes;
		bool m_have_shape_with_current_slope;
		bool m_sectors_closed;
		bool m_mark_sector_closed;

		//
		// main road frame
		//

		Vertex m_v_west_left, m_v_west_right,
			   m_v_east_left, m_v_east_right;

		Sidedef m_sd_roadside;

		Sector m_s_body, m_s_east_side, m_s_west_side;

		//
		// road mark
		//

		Vertex m_v_mark_west, m_v_mark_east;

		Sidedef m_sd_markside;

		Sector m_s_mark;

		//
		// background
		//

		Vertex m_v_background_west_left, m_v_background_west_right;
		Vertex m_v_background_east_left, m_v_background_east_right;
	
		Sector m_s_background_body_west,
			   m_s_background_body_east,
			   m_s_background_sky;

		//
		// fence
		//

		Vertex m_v_fence_west_left,
			   m_v_fence_west_right;
		Vertex m_v_fence_east_left,
			   m_v_fence_east_right;

		Sidedef m_sd_fenceside;

		Sector m_s_fence_west, m_s_fence_east;

		// misc
		Vertex m_v_null_prev;

		// map: <map-obj-template-address> -> <last-object-id>
		std::unordered_map<void *, unsigned int> m_last_ids;

		// pair: [<mark-vertices-distance-from-the-start>, pair: [<mark-vertex-1>, <mark-vertex-2>]]
		std::vector< std::pair<double, IdPair> > m_v_mark_ids_prev;
	};

	//
	// Put a road into the map according to given object set in input data and config
	//

	void GetGeneratedRoad(const RoadFigureDataConst& input_data, const RoadConfig& road_config, Udmf::Map& map);
}

#endif // _ROAD_GEN_CORE_H_
