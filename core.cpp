
#include "core.h"

#include <unordered_set>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoadConfig functions implementation

namespace RoadGen {
	template<typename T, typename TSrc>
	static void SetRoadConfigRetrievedIntOrDefaultPropValue(const std::vector<TSrc>* prop_values_src,
															const std::unordered_map<std::string, int>& prop_indices,
															const std::string& prop_name,
															T& dest,
															T default_value)
	{
		if(prop_values_src) {
			const auto prop_it = prop_indices.find(prop_name);
			if(prop_it != prop_indices.end() && prop_it->second >= 0) {
				dest = prop_values_src->at(prop_it->second).value_int;
				return;
			}
		}

		dest = default_value;
	}

	void GetRoadConfig(const Io::ConfigData& config_data, RoadConfig& config) {
		config = RoadConfig();

		//
		// read sizes
		//

		{
			// map: property-name -> index in sizes or -1
			std::unordered_map<std::string, int> found_known_props;
			found_known_props["BackgroundDist"] = -1;
			found_known_props["FenceHeight"] = -1;
			found_known_props["RoadWidth"] = -1;
			found_known_props["RoadSideWidth"] = -1;
			found_known_props["RoadSideHeight"] = -1;
			found_known_props["RoadMarkWidth"] = -1;
			found_known_props["RoadMarkLength"] = -1;
			found_known_props["RoadMarkGap"] = -1;

			const auto sizes_it = config_data.find("Sizes");
			const Io::ConfigDataEntry::FieldArray* sizes = NULL;
			if(sizes_it != config_data.end()) {
				if(sizes_it->second.size() != 1 || !sizes_it->second[0].name.empty()) {
					throw Exception("bad road config - bad value of 'Sizes' setting");
				}
				sizes = &sizes_it->second[0].fields;

				for(int i=0; i<sizes->size(); i++) {
					const auto known_prop_it = found_known_props.find((*sizes)[i].name);
					if(known_prop_it == found_known_props.end()) {
						// unknown size entry in config data
						continue;
					}

					if((*sizes)[i].field_type != Io::ConfigDataEntry::Field::enType_Int || (*sizes)[i].value_int < 0) {
						throw Exception("bad road config - bad value of size property '" + (*sizes)[i].name + "' in 'Sizes' setting value (must be non-negative number)");
					}

					known_prop_it->second = i;
				}
			}

			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "BackgroundDist", config.sizes.background_dist, 128.0);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "FenceHeight", config.sizes.fence_height, 128);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadWidth", config.sizes.road_width, 384.0);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadSideWidth", config.sizes.road_side_width, 128.0);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadSideHeight", config.sizes.road_side_height, 8);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadMarkWidth", config.sizes.road_mark_width, 16.0);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadMarkLength", config.sizes.road_mark_length, 256.0);
			SetRoadConfigRetrievedIntOrDefaultPropValue(sizes, found_known_props, "RoadMarkGap", config.sizes.road_mark_gap, 512.0);

			// not all size combinations are allowed, also check that sizes are not too big;
			// some of these constraints may be weaken if needed
			if(config.sizes.background_dist < 33.0 || config.sizes.background_dist > 4096.0) {
				throw Exception("bad road config - bad value of size property 'BackgroundDist' in 'Sizes' setting value - must be between 33 and 4096");
			}

			if(config.sizes.road_side_width < 1.0 || config.sizes.road_side_width > 4096.0) {
				throw Exception("bad road config - bad value of size property 'RoadSideWidth' in 'Sizes' setting value - must be between 1 and 4096");
			}

			if(config.sizes.road_side_height > config.sizes.fence_height) {
				throw Exception("bad road config - bad value of size properties 'RoadSideHeight' and/or 'FenceHeight' in 'Sizes' setting value -"
								" 'RoadSideHeight' must be less than or equal to 'FenceHeight'");
			}

			if(config.sizes.road_width > 4096.0) {
				throw Exception("bad road config - bad value of size property 'RoadWidth' in 'Sizes' setting value - must be less than or equal to 4096");
			}

			if(config.sizes.road_mark_width < 4.0) {
				throw Exception("bad road config - bad value of size property 'RoadMarkWidth' in 'Sizes' setting value - must be greater than or equal to 4");
			}

			if(config.sizes.road_mark_width + 2.0 > config.sizes.road_width) {
				throw Exception("bad road config - bad value of size properties 'RoadWidth' and/or 'RoadMarkWidth' in 'Sizes' setting value -"
								" 'RoadWidth' must be greater than or equal to 'RoadMarkWidth + 2'");
			}

			if(config.sizes.road_mark_gap < 16.0 || config.sizes.road_mark_gap > 65535.0) {
				throw Exception("bad road config - bad value of size property 'RoadMarkGap' in 'Sizes' setting value - must be between 16 and 65535");
			}

			if(config.sizes.road_mark_length < 16.0 || config.sizes.road_mark_length > 65535.0) {
				throw Exception("bad road config - bad value of size property 'RoadMarkLength' in 'Sizes' setting value - must be between 16 and 65535");
			}
		}

		{
			// map: property-name -> (dest-var, dest-var-default-value)
			std::unordered_map< std::string, std::pair<std::string*, std::string> > known_props_map;
			known_props_map["Sky"] = std::make_pair(&config.textures[RoadConfig::enTexture_Sky], "F_SKY1");
			known_props_map["Background"] = std::make_pair(&config.textures[RoadConfig::enTexture_Background], "FWATER1");
			known_props_map["Fence"] = std::make_pair(&config.textures[RoadConfig::enTexture_Fence], "BIGBRIK1");
			known_props_map["FenceFloor"] = std::make_pair(&config.textures[RoadConfig::enTexture_FenceFloor], "FLOOR7_1");
			known_props_map["RoadSide"] = std::make_pair(&config.textures[RoadConfig::enTexture_RoadSide], "SLIME14");
			known_props_map["RoadSideWall"] = std::make_pair(&config.textures[RoadConfig::enTexture_RoadSideWall], "STEP4");
			known_props_map["RoadBody"] = std::make_pair(&config.textures[RoadConfig::enTexture_RoadBody], "CEIL5_1");
			known_props_map["RoadMark"] = std::make_pair(&config.textures[RoadConfig::enTexture_RoadMark], "FLAT19");

			const auto textures_it = config_data.find("Textures");
			if(textures_it != config_data.end()) {
				if(textures_it->second.size() != 1 || !textures_it->second[0].name.empty()) {
					throw Exception("bad road config - bad value of 'Textures' setting");
				}
				const Io::ConfigDataEntry::FieldArray& textures = textures_it->second[0].fields;

				for(int i=0; i<textures.size(); i++) {
					const auto known_prop_it = known_props_map.find(textures[i].name);
					if(known_prop_it == known_props_map.end()) {
						// unknown texture entry in config data
						continue;
					}

					// we restrict long texture names and/or names with some predefined chars
					// to reduce number of checks and simplify them;
					// UDMF may allow much wider range of texture names
					if(    textures[i].field_type != Io::ConfigDataEntry::Field::enType_String
						|| textures[i].value_string.empty()
						|| textures[i].value_string.length() > 256
						|| textures[i].value_string.find('"') < textures[i].value_string.length()
						|| textures[i].value_string.find('\\') < textures[i].value_string.length())
					{
						throw Exception("bad road config - bad or empty value of texture property '" + textures[i].name + "' in 'Textures' setting value");
					}

					*known_prop_it->second.first = textures[i].value_string;
					known_props_map.erase(known_prop_it);
				}
			}

			// assign default values to props that are not found in the config
			for(auto known_prop_it = known_props_map.begin(); known_prop_it != known_props_map.end(); known_prop_it++) {
				*known_prop_it->second.first = known_prop_it->second.second;
			}
		}

		const auto light_level_it = config_data.find("LightLevel");
		if(light_level_it != config_data.end()) {
			const Io::ConfigDataEntryArray& entries = light_level_it->second;
			if(    entries.size() != 1
				|| !entries[0].name.empty()
				|| entries[0].fields.size() != 1
				|| !entries[0].fields[0].name.empty()
				|| entries[0].fields[0].field_type != Io::ConfigDataEntry::Field::enType_Int
				|| entries[0].fields[0].value_int < 0
				|| entries[0].fields[0].value_int > 255)
			{
				throw Exception("bad road config - bad or empty value of 'LightLevel' setting");
			}

			config.light_level = (unsigned char)entries[0].fields[0].value_int;

		} else {
			config.light_level = 192;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IntersectionChecker implementation

namespace RoadGen {
	void IntersectionChecker::AddQuad(const Math::Point2D& p1, const Math::Point2D& p2, const Math::Point2D& p3, const Math::Point2D& p4) {
		const QuadInfo quad_info = GetQuadInfo(p1, p2, p3, p4);
		if(!QuadDoesNotIntersectOthers(quad_info)) {
			throw Exception("error generating a road - some figures have an intersection");
		}

		if(HavePendingEncirclingQuad()) {
			m_quads.back().second->push_back(quad_info);

		} else {
			m_quads.push_back(t_quad_with_subquads());
			m_quads.back().first = quad_info;
		}

		m_new_figure = false;
	}

	void IntersectionChecker::StartEncirclingQuad() {
		EndEncirclingQuad();
		m_quads.push_back(t_quad_with_subquads());
		m_quads.back().second = t_quad_array_ptr(new t_quad_array);
	}

	void IntersectionChecker::EndEncirclingQuad() {
		if(!HavePendingEncirclingQuad()) {
			return;
		}

		t_quad_array& encircled_quads = *m_quads.back().second;
		if(encircled_quads.size() == 0) {
			// no quads have been added - remove the entry
			m_quads.pop_back();
			return;
		}

		// calculate encircling quad according to encircled ones:
		// [(xmin, ymin), (xmin, ymax), (xmax, ymax), (xmax, ymin)]
		double xmin = encircled_quads[0].vertices[0].x;
		double xmax = encircled_quads[0].vertices[0].x;
		double ymin = encircled_quads[0].vertices[0].y;
		double ymax = encircled_quads[0].vertices[0].y;
		for(int qix=0; qix<encircled_quads.size(); qix++) {
			const QuadInfo& quad = encircled_quads[qix];
			for(int vix=0; vix<4; vix++) {
				const Point& p = quad.vertices[vix];
				if(p.x < xmin) {
					xmin = p.x;
				}
				if(p.x > xmax) {
					xmax = p.x;
				}
				if(p.y < ymin) {
					ymin = p.y;
				}
				if(p.y > ymax) {
					ymax = p.y;
				}
			}
		}

		if(Math::IsZeroOrCloseTo(xmax - xmin) || Math::IsZeroOrCloseTo(ymax - ymin)) {
			// only bad quads were added (not good though), but anyway do not throw
			m_quads.pop_back();

		} else {
			m_quads.back().first = GetQuadInfo(Point(xmin, ymin), Point(xmin, ymax), Point(xmax, ymax), Point(xmax, ymin));
		}
	}

	void IntersectionChecker::CutFigure() {
		m_new_figure = true;
	}

	IntersectionChecker::QuadInfo IntersectionChecker::GetQuadInfo(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
		class Utils {
		public:
			std::string ToString(const Point& p) const {
				return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
			}
		} utils;

		QuadInfo quad_info;

		// set vertices for the new quad
		quad_info.vertices[0] = p1;
		quad_info.vertices[1] = p2;
		quad_info.vertices[2] = p3;
		quad_info.vertices[3] = p4;

		// set line equations for the new quad
		for(int i=0; i<4; i++) {
			const Point& p1 = quad_info.vertices[i];
			const Point& p2 = quad_info.vertices[(i+1) % 4];
			LineEquation& cur_l_eq = quad_info.segments[i].line_equation;

			// calculate normal vector of the line (a, b);
			// e.g. normal vector can be (y2-y1, x1-x2);
			// then calculate constant c in the line equation ax + by + c = 0;
			// divide all coeffs by max(a, b) among non-null a, b
			cur_l_eq.a = p2.y - p1.y;
			const double sign_a = cur_l_eq.a < 0 ? -1.0 : 1.0;
			cur_l_eq.b = p1.x - p2.x;
			const double sign_b = cur_l_eq.b < 0 ? -1.0 : 1.0;
			const double coeffs_divider =
				Math::IsZeroOrCloseTo(cur_l_eq.a)
					? (Math::IsZeroOrCloseTo(cur_l_eq.b)
						? 1.0
						: cur_l_eq.b)
					: (Math::IsZeroOrCloseTo(cur_l_eq.b)
						? cur_l_eq.a
						: (sign_a*cur_l_eq.a > sign_b*cur_l_eq.b
							? cur_l_eq.a
							: cur_l_eq.b));
			cur_l_eq.a /= coeffs_divider;
			cur_l_eq.b /= coeffs_divider;
			cur_l_eq.c = p2.x / coeffs_divider * p1.y - p1.x / coeffs_divider * p2.y;

			const Point& next_point = quad_info.vertices[(i+2) % 4];
			const EnPointLinePos next_point_pos = Math::GetPointLinePos2D(next_point, cur_l_eq);
			if(next_point_pos == Math::enPointLinePos2D_BelongsTo) {
				// not a valid quad with 3 or 4 points belonging to a single line
				throw Exception("error generating a road - bad quad with vertices "
					+ utils.ToString(quad_info.vertices[0]) + ", " + utils.ToString(quad_info.vertices[1]) + ", "
					+ utils.ToString(quad_info.vertices[2]) + ", " + utils.ToString(quad_info.vertices[3]));
			}

			const Point& post_next_point = quad_info.vertices[(i+3) % 4];
			const EnPointLinePos post_next_point_pos = Math::GetPointLinePos2D(post_next_point, cur_l_eq);
			if(post_next_point_pos == Math::enPointLinePos2D_BelongsTo) {
				// not a valid quad with 3 points belonging to a single line
				throw Exception("error generating a road - bad quad with vertices "
					+ utils.ToString(quad_info.vertices[0]) + ", " + utils.ToString(quad_info.vertices[1]) + ", "
					+ utils.ToString(quad_info.vertices[2]) + ", " + utils.ToString(quad_info.vertices[3]));
			}

			if(next_point_pos != post_next_point_pos) {
				// concave quad (should not be used) or self-intersecting quad (invalid)
				throw Exception("error generating a road - bad quad with vertices "
					+ utils.ToString(quad_info.vertices[0]) + ", " + utils.ToString(quad_info.vertices[1]) + ", "
					+ utils.ToString(quad_info.vertices[2]) + ", " + utils.ToString(quad_info.vertices[3]));
			}

			quad_info.segments[i].other_points_pos = next_point_pos;
			quad_info.segments[i].length_squared = Math::Get2DDistanceSquared(p1.x, p1.y, p2.x, p2.y);
		}

		return quad_info;
	}

	bool IntersectionChecker::QuadDoesNotIntersectOthers(const QuadInfo& quad_to_check_info) const {
		// last added quad must be connected with the new one:
		// they must have a common segment (if this is not a new figure)
		const QuadInfo* neighbor_quad_ptr = NULL;
		if(!m_new_figure && m_quads.size() != 0) {
			if(m_quads.back().second == NULL) {
				neighbor_quad_ptr = &m_quads.back().first;

			} else {
				if(!m_quads.back().second->empty()) {
					neighbor_quad_ptr = &m_quads.back().second->back();

				} else {
					// started to build an encircling rect, no rects are added yet
					if(m_quads.size() > 1) {
						const t_quad_with_subquads& pre_last_quad = m_quads[m_quads.size() - 2];
						if(pre_last_quad.second == NULL) {
							neighbor_quad_ptr = &pre_last_quad.first;

						} else if(pre_last_quad.second->size() != 0) {
							neighbor_quad_ptr = &pre_last_quad.second->back();

						} else {
							throw Exception("internal error - bad intersection checker state");
						}
					}
				}
			}
		}

		for(int oqix=0; oqix<m_quads.size(); oqix++) {
			const t_quad_with_subquads& outer_quad_info = m_quads[oqix];
			if(neighbor_quad_ptr == &outer_quad_info.first) {
				// do not check
				continue;
			}

			// check inner quads in case encircling quad is not built yet
			bool force_inner_quads_check = false;
			if(outer_quad_info.second != NULL) {
				if(outer_quad_info.second->size() == 0) {
					// nothing to check
					continue;
				}

				if(outer_quad_info.first.IsNull()) {
					if(oqix != m_quads.size()-1) {
						throw Exception("internal error - bad intersection checker state");
					}

					// ok, building in progress
					force_inner_quads_check = true;
				}
			}

			// check inner quads only if have intersection with the encircling quad
			if(force_inner_quads_check || QuadsHaveCommonPoints(outer_quad_info.first, quad_to_check_info)) {
				if(outer_quad_info.second == NULL) {
					// outer quad is the quad to check - intersection
					return false;
				}

				// inner quads are the quads to check - check each of them
				for(int iqix=0; iqix<outer_quad_info.second->size(); iqix++) {
					const QuadInfo& inner_quad_info = (*outer_quad_info.second)[iqix];
					if(neighbor_quad_ptr == &inner_quad_info) {
						// do not check
						continue;
					}

					if(QuadsHaveCommonPoints(inner_quad_info, quad_to_check_info)) {
						// intersection
						return false;
					}
				}
			}
		}

		// do not check intersections with neighbor quad - they always won't intersect
		// (except the single common line segment)
		//if(neighbor_quad_ptr) {
		//	// this quad and new one must have a common segment
		//	// points and line equation must be exactly the same
		//	bool found_common_segment = false;
		//	for(int i=0; i<16; i++) {
		//		const LineSegmentInfo& seg1 = neighbor_quad_ptr->segments[i/4];
		//		const LineEquation& l_eq1 = seg1.line_equation;
		//		const LineSegmentInfo& seg2 = quad_to_check_info.segments[i%4];
		//		const LineEquation& l_eq2 = seg2.line_equation;
		//		if(l_eq1.a == l_eq2.a && l_eq1.b == l_eq2.b && l_eq1.c == l_eq2.c) {
		//			// same line
		//			if(seg1.other_points_pos == seg2.other_points_pos) {
		//				// other points of both quads located on the same side of the common segment (the line) -
		//				// intersection (we have only convex quads)
		//				return false;
		//			}

		//			// both quads have common segment and other points are isolated for each quad
		//			// from points of the other quad - no intersection
		//			break;
		//		}
		//	}
		//}

		return true;
	}

	bool IntersectionChecker::QuadsHaveCommonPoints(const QuadInfo& quad_info1, const QuadInfo& quad_info2) {
		// this value is valid only if quads have no contour intersections
		bool second_inside_first = true;

		// this value is valid only if quads have no contour intersections
		bool first_inside_second = true;

		for(int ix1=0; ix1<4; ix1++) {
			const LineSegmentInfo& seg1 = quad_info1.segments[ix1];
			const LineEquation& l_eq1 = seg1.line_equation;

			// check if quad 1 is inside quad 2
			if(first_inside_second) {
				const Point& test_point1 = quad_info1.vertices[0];
				const int ix = ix1;
				const LineSegmentInfo& seg2 = quad_info2.segments[ix];
				const EnPointLinePos test_point1_pos = Math::GetPointLinePos2D(test_point1, seg2.line_equation);
				if(test_point1_pos != seg2.other_points_pos) {
					// a point of quad 1 is outside quad 2
					first_inside_second = false;
				}
			}

			// check if quad 2 is inside quad 1
			if(second_inside_first) {
				const Point& test_point2 = quad_info2.vertices[0];
				const EnPointLinePos test_point2_pos = Math::GetPointLinePos2D(test_point2, l_eq1);
				if(test_point2_pos != seg1.other_points_pos) {
					// the point of quad 2 is outside quad 1
					second_inside_first = false;
				}
			}

			for(int ix2=0; ix2<4; ix2++) {
				const LineSegmentInfo& seg2 = quad_info2.segments[ix2];
				const LineEquation& l_eq2 = seg2.line_equation;

				const double divider = l_eq1.a * l_eq2.b - l_eq2.a * l_eq1.b;
				if(Math::IsZeroOrCloseTo(divider)) {
					const EnPointLinePos point_pos = Math::GetPointLinePos2D(quad_info1.vertices[ix1], l_eq2);
					if(point_pos == Math::enPointLinePos2D_BelongsTo) {
						// both segments are part of the same line
						const double sqr_length_11_21 = Math::Get2DDistanceSquared(quad_info1.vertices[ix1], quad_info2.vertices[ix2]);
						const double sqr_length_11_22 = Math::Get2DDistanceSquared(quad_info1.vertices[ix1], quad_info2.vertices[(ix2 + 1) % 4]);
						const double sqr_length_12_21 = Math::Get2DDistanceSquared(quad_info1.vertices[(ix1 + 1) % 4], quad_info2.vertices[ix2]);
						const double sqr_length_12_22 = Math::Get2DDistanceSquared(quad_info1.vertices[(ix1 + 1) % 4], quad_info2.vertices[(ix2 + 1) % 4]);
						if(   Math::IsZeroOrCloseTo(sqr_length_11_21)
						   || Math::IsZeroOrCloseTo(sqr_length_11_22)
						   || Math::IsZeroOrCloseTo(sqr_length_12_21)
						   || Math::IsZeroOrCloseTo(sqr_length_12_22))
						{
							// two points of different quads have same coordinates - intersection
							return true;
						}

						if(seg1.length_squared > seg2.length_squared) {
							// check a vertex of quad 2 belongs to the segment of quad 1
							if(sqr_length_11_21 <= seg1.length_squared && sqr_length_12_21 <= seg1.length_squared) {
								// first vertex of quad 2 belongs to the segment of quad 1 - intersection
								return true;
							}
							if(sqr_length_11_22 <= seg1.length_squared && sqr_length_12_22 <= seg1.length_squared) {
								// second vertex of quad 2 belongs to the segment of quad 1 - intersection
								return true;
							}

						} else {
							// check a vertex of quad 1 belongs to the segment of quad 2
							if(sqr_length_11_21 <= seg2.length_squared && sqr_length_11_22 <= seg2.length_squared) {
								// first vertex of quad 1 belongs to the segment of quad 2 - intersection
								return true;
							}
							if(sqr_length_12_21 <= seg2.length_squared && sqr_length_12_22 <= seg2.length_squared) {
								// second vertex of quad 1 belongs to the segment of quad 2 - intersection
								return true;
							}
						}

						// ok - no intersection
						continue;
					}

					// parallel lines - ok, check others
					continue;
				}

				// an intersection point
				const double isec_x = -(l_eq1.c * l_eq2.b - l_eq2.c * l_eq1.b) / divider;
				const double isec_y = (l_eq2.a * l_eq1.c - l_eq1.a * l_eq2.c) / divider;

				// if distance between an intersection point and both segment vertices is
				// less than the segment length, then the intersection point belongs to the segment
				// optimisation - use squared length values in comparisons
				const Point& p11 = quad_info1.vertices[ix1];
				const Point& p12 = quad_info1.vertices[(ix1+1) % 4];
				const double dist_to_p11_squared = Math::Get2DDistanceSquared(p11.x, p11.y, isec_x, isec_y);
				if(dist_to_p11_squared > seg1.length_squared) {
					// out of the segment
					continue;
				}

				const double dist_to_p12_squared = Math::Get2DDistanceSquared(p12.x, p12.y, isec_x, isec_y);
				if(dist_to_p12_squared > seg1.length_squared) {
					// out of the segment
					continue;
				}

				const Point& p21 = quad_info2.vertices[ix2];
				const Point& p22 = quad_info2.vertices[(ix2+1) % 4];
				const double dist_to_p21_squared = Math::Get2DDistanceSquared(p21.x, p21.y, isec_x, isec_y);
				if(dist_to_p21_squared > seg2.length_squared) {
					// out of the segment
					continue;
				}

				const double dist_to_p22_squared = Math::Get2DDistanceSquared(p22.x, p22.y, isec_x, isec_y);
				if(dist_to_p22_squared > seg2.length_squared) {
					// out of the segment
					continue;
				}

				// contours intersect each other
				return true;
			}
		}

		if(first_inside_second || second_inside_first) {
			// one quad inside another treated as intersection
			return true;
		}

		return false;
	}

	bool IntersectionChecker::HavePendingEncirclingQuad() const {
		if(m_quads.empty()) {
			return false;
		}

		const QuadInfo& encircling_quad = m_quads.back().first;
		const t_quad_array_ptr& encircled_quads_array_ptr = m_quads.back().second;
		if(!encircling_quad.IsNull() || encircled_quads_array_ptr == NULL) {
			return false;
		}

		return true;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoadFigure implementation

namespace RoadGen {
	void RoadFigure::Start() {
		if(m_started_drawing) {
			throw Exception("error generating a road - already started generating this figure");
		}

		if(m_finished_drawing) {
			throw Exception("error generating a road - already finished generating this figure, cannot restart");
		}

		if(m_v_null.x < -30000.0 || m_v_null.x > 30000.0 || m_v_null.y < -30000.0 || m_v_null.y > 30000.0) {
			throw Exception("error generating a road - bad figure start position, coordinates must be between -30000 and 30000 but got "
							"(" + std::to_string(m_v_null.x) + ", " + std::to_string(m_v_null.y) + ")");
		}

		if(m_floorpos < -32767 || m_floorpos > 32767) {
			throw Exception("error generating a road - bad figure floor position, must be between -32767 and 32767 but got '" + std::to_string(m_floorpos) + "'");
		}

		if(m_ceilingpos < m_floorpos) {
			throw Exception("error generating a road - bad figure height, must be non-negative but got '" + std::to_string(m_ceilingpos - m_floorpos) + "'");
		}

		if(m_ceilingpos > 32767) {
			throw Exception("error generating a road - bad figure floor position and/or height, ceiling position is greater than 32767;"
							" floor position = " + std::to_string(m_floorpos) + ", height = " + std::to_string(m_ceilingpos - m_floorpos));
		}

		if(m_ceilingpos - m_floorpos > 32767) {
			throw Exception("error generating a road - bad figure height, must be less than or equal to 32767 but got '" + std::to_string(m_ceilingpos - m_floorpos) + "'");
		}

		if(m_floorpos >= m_ceilingpos - m_config.sizes.fence_height) {
			throw Exception("error generating a road - bad figure floor position and/or height, fence is higher than the ceiling;"
							" floor position = " + std::to_string(m_floorpos) + ", height = " + std::to_string(m_ceilingpos - m_floorpos)
							+ ", fence_hight = " + std::to_string(m_config.sizes.fence_height));
		}

		// set map object template objects;
		// then during building process copy values from these template objects
		// and insert into the map with proper linking (setting of IDs)

		// add background
		const double background_gap_big = m_config.sizes.background_dist + BACKGROUND_THICKNESS;
		const double background_gap_small = m_config.sizes.background_dist;

		m_v_background_west_left =
			m_v_null.GetMoved(-background_gap_big, -background_gap_big - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_background_west_left] = m_map << m_v_background_west_left;

		m_v_background_west_right =
			m_v_null.GetMoved(-background_gap_small, -background_gap_small - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_background_west_right] = m_map << m_v_background_west_right;

		m_v_background_east_left =
			m_v_null.GetMoved(-background_gap_small, background_gap_small + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_background_east_left] = m_map << m_v_background_east_left;

		m_v_background_east_right =
			m_v_null.GetMoved(-background_gap_big, background_gap_big + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_background_east_right] = m_map << m_v_background_east_right;

		m_last_ids[&m_s_background_sky] = m_map << m_s_background_sky;
		const unsigned int id_s_background_body =
			m_last_ids[&m_s_background_body_east] =
			m_last_ids[&m_s_background_body_west] =
				m_map << m_s_background_body_east;

		const unsigned int id_sd_background_sky = m_map << Sidedef(m_last_ids.at(&m_s_background_sky));
		m_map << Linedef(m_last_ids.at(&m_v_background_west_left), m_last_ids.at(&m_v_background_east_right),
						 id_sd_background_sky);
		const unsigned int id_sd_background_body_outer = m_map << Sidedef(m_last_ids.at(&m_s_background_sky));
		const unsigned int id_sd_background_body_inner = m_map << Sidedef(id_s_background_body);
		m_map << Linedef(m_last_ids.at(&m_v_background_west_right), m_last_ids.at(&m_v_background_east_left),
						 id_sd_background_body_inner, id_sd_background_body_outer);

		const Vertex v_background_next_west_left =
			m_v_null.GetMoved(0, -background_gap_big - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_background_next_west_right = 
			m_v_null.GetMoved(0, -background_gap_small - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_background_next_east_left = 
			m_v_null.GetMoved(0, background_gap_small + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_background_next_east_right =
			m_v_null.GetMoved(0, background_gap_big + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);

		ExtendBackground(v_background_next_west_left, v_background_next_west_right,
						 v_background_next_east_left, v_background_next_east_right);

		// add fence
		m_v_fence_west_left =
			m_v_null.GetMoved(-background_gap_big + FENCE_BACKGROUND_GAP,
							  -background_gap_big - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width + FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_fence_west_left] = m_map << m_v_fence_west_left;

		m_v_fence_west_right =
			m_v_null.GetMoved(-background_gap_small + FENCE_BACKGROUND_GAP,
							  -background_gap_small - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width + FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_fence_west_right] = m_map << m_v_fence_west_right;

		m_v_fence_east_left =
			m_v_null.GetMoved(-background_gap_small + FENCE_BACKGROUND_GAP,
							  background_gap_small + m_config.sizes.road_width/2.0f + m_config.sizes.road_side_width - FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_fence_east_left] = m_map << m_v_fence_east_left;

		m_v_fence_east_right =
			m_v_null.GetMoved(-background_gap_big + FENCE_BACKGROUND_GAP,
							  background_gap_big + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width - FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_fence_east_right] = m_map << m_v_fence_east_right;

		const unsigned int id_s_fence =
			m_last_ids[&m_s_fence_west] =
			m_last_ids[&m_s_fence_east] =
				m_map << m_s_fence_west;

		const unsigned int id_sd_fence_far_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body);
		const unsigned int id_sd_fence_far_inner = m_map << Sidedef(id_s_fence);
		m_map << Linedef(m_last_ids.at(&m_v_fence_west_left), m_last_ids.at(&m_v_fence_east_right),
						 id_sd_fence_far_inner, id_sd_fence_far_outer);

		const unsigned int id_sd_fence_near_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body);
		const unsigned int id_sd_fence_near_inner = m_map << Sidedef(id_s_fence);
		m_map << Linedef(m_last_ids.at(&m_v_fence_east_left), m_last_ids.at(&m_v_fence_west_right),
						 id_sd_fence_near_inner, id_sd_fence_near_outer);

		const Vertex v_fence_next_west_left =
			m_v_null.GetMoved(0, -background_gap_big - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width + FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_fence_next_west_right =
			m_v_null.GetMoved(0, -background_gap_small - m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width + FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_fence_next_east_left =
			m_v_null.GetMoved(0, background_gap_small + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width - FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		const Vertex v_fence_next_east_right =
			m_v_null.GetMoved(0, background_gap_big + m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width - FENCE_BACKGROUND_GAP)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);

		ExtendFence(v_fence_next_west_left, v_fence_next_west_right,
					v_fence_next_east_left, v_fence_next_east_right);

		// add road frame
		m_v_west_left =
			m_v_null.GetMoved(0, -m_config.sizes.road_width/2.0 - m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_west_left] = m_map << m_v_west_left;

		m_v_west_right =
			m_v_null.GetMoved(0, -m_config.sizes.road_width/2.0)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_west_right] = m_map << m_v_west_right;

		m_v_east_left =
			m_v_null.GetMoved(0, m_config.sizes.road_width/2.0)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_east_left] = m_map << m_v_east_left;

		m_v_east_right =
			m_v_null.GetMoved(0, m_config.sizes.road_width/2.0 + m_config.sizes.road_side_width)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_last_ids[&m_v_east_right] = m_map << m_v_east_right;

		m_last_ids[&m_s_west_side] = m_map << m_s_west_side;
		m_last_ids[&m_s_east_side] = m_map << m_s_east_side;

		const unsigned int id_sd_west_outer = m_map << m_sd_roadside.GetWithSector(id_s_background_body);
		const unsigned int id_sd_west_inner = m_map << Sidedef(m_last_ids.at(&m_s_west_side));
		m_map << Linedef(m_last_ids.at(&m_v_west_left), m_last_ids.at(&m_v_west_right),
						 id_sd_west_inner, id_sd_west_outer);

		const unsigned int id_sd_east_outer = m_map << m_sd_roadside.GetWithSector(id_s_background_body);
		const unsigned int id_sd_east_inner = m_map << Sidedef(m_last_ids.at(&m_s_east_side));
		m_map << Linedef(m_last_ids.at(&m_v_east_left), m_last_ids.at(&m_v_east_right),
						 id_sd_east_inner, id_sd_east_outer);

		m_last_ids[&m_s_body] = m_map << m_s_body;
		const unsigned int id_sd_middle_outer = m_map << Sidedef(id_s_background_body);
		const unsigned int id_sd_middle_inner = m_map << Sidedef(m_last_ids.at(&m_s_body));

		// add road mark
		m_v_mark_west =
			m_v_null.GetMoved(0, -m_config.sizes.road_mark_width/2.0)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);
		m_v_mark_east =
			m_v_null.GetMoved(0, m_config.sizes.road_mark_width/2.0)
				.GetRotated(m_v_null.x, m_v_null.y, m_angle);

		const bool have_mark = m_mark_coord < m_config.sizes.road_mark_length;
		if(have_mark) {
			m_last_ids[&m_v_mark_east] = m_map << m_v_mark_east;
			m_last_ids[&m_v_mark_west] = m_map << m_v_mark_west;
			m_last_ids[&m_s_mark] = m_map << m_s_mark;

			m_map << Linedef(m_last_ids.at(&m_v_west_right), m_last_ids.at(&m_v_mark_west),
							 id_sd_middle_inner, id_sd_middle_outer);

			const unsigned int id_sd_mark_inner = m_map << Sidedef(m_last_ids.at(&m_s_mark));
			const unsigned int id_sd_mark_outer = m_map << m_sd_markside.GetWithSector(id_s_background_body);
			m_map << Linedef(m_last_ids.at(&m_v_mark_west), m_last_ids.at(&m_v_mark_east),
							 id_sd_mark_inner, id_sd_mark_outer);

			const unsigned int id_sd_middle_inner2 = m_map << Sidedef(m_last_ids.at(&m_s_body));
			const unsigned int id_sd_middle_outer2 = m_map << Sidedef(id_s_background_body);
			m_map << Linedef(m_last_ids.at(&m_v_mark_east), m_last_ids.at(&m_v_east_left),
							 id_sd_middle_inner2, id_sd_middle_outer2);

			m_mark_sector_closed = false;

		} else {
			m_map << Linedef(m_last_ids.at(&m_v_west_right), m_last_ids.at(&m_v_east_left),
							 id_sd_middle_inner, id_sd_middle_outer);
		}

		m_sectors_closed = false;
		m_started_drawing = true;
	}

	void RoadFigure::Finish() {
		if(!m_started_drawing) {
			throw Exception("error generating a road - did not start generating this figure, cannot finish");
		}

		if(m_finished_drawing) {
			throw Exception("error generating a road - already finished generating this figure, cannot finish again");
		}

		if(!Math::IsZeroOrCloseTo(m_slope_modifier.tangent) || !m_have_shape_with_current_slope) {
			throw Exception("error generating a road - slope in the end of a figure is not allowed, make a horizontal line/arc after");
		}

		const double cos_angle = Math::Cos(m_angle);
		const double sin_angle = Math::Sin(m_angle);

		// close background (maybe two separated sectors)
		const double background_gap_big = m_config.sizes.background_dist + BACKGROUND_THICKNESS;
		const double background_gap_small = m_config.sizes.background_dist;
		ExtendBackground(m_v_background_west_left.GetMoved(background_gap_big * cos_angle, background_gap_big * sin_angle),
						 m_v_background_west_right.GetMoved(background_gap_small * cos_angle, background_gap_small * sin_angle),
						 m_v_background_east_left.GetMoved(background_gap_small * cos_angle, background_gap_small * sin_angle),
						 m_v_background_east_right.GetMoved(background_gap_big * cos_angle, background_gap_big * sin_angle));

		const unsigned int id_v_background_west_left = m_last_ids.at(&m_v_background_west_left);
		const unsigned int id_v_background_west_right = m_last_ids.at(&m_v_background_west_right);
		const unsigned int id_v_background_east_left = m_last_ids.at(&m_v_background_east_left);
		const unsigned int id_v_background_east_right = m_last_ids.at(&m_v_background_east_right);

		const unsigned int id_s_background_sky = m_last_ids.at(&m_s_background_sky);
		const unsigned int id_s_background_body_east = m_last_ids.at(&m_s_background_body_east);
		const unsigned int id_s_background_body_west = m_last_ids.at(&m_s_background_body_west);

		const unsigned int id_sd_background_sky = m_map << Sidedef(id_s_background_sky);
		m_map << Linedef(id_v_background_east_right, id_v_background_west_left,
						 id_sd_background_sky);

		const Vertex v_background_middle((m_v_background_east_left.x + m_v_background_west_right.x) / 2.0,
										 (m_v_background_east_left.y + m_v_background_west_right.y) / 2.0);
		const unsigned int id_v_background_middle = m_map << v_background_middle;

		const unsigned int id_sd_background_body_outer_east = m_map << Sidedef(id_s_background_sky);
		const unsigned int id_sd_background_body_inner_east = m_map << Sidedef(id_s_background_body_east);
		m_map << Linedef(id_v_background_east_left, id_v_background_middle,
						 id_sd_background_body_inner_east, id_sd_background_body_outer_east);

		const unsigned int id_sd_background_body_outer_west = m_map << Sidedef(id_s_background_sky);
		const unsigned int id_sd_background_body_inner_west = m_map << Sidedef(id_s_background_body_west);
		m_map << Linedef(id_v_background_middle, id_v_background_west_right,
						 id_sd_background_body_inner_west, id_sd_background_body_outer_west);

		// close fence (maybe two separated sectors)
		const double fence_gap_larger = background_gap_small - FENCE_BACKGROUND_GAP + BACKGROUND_THICKNESS;
		const double fence_gap_smaller = fence_gap_larger - FENCE_THICKNESS;
		ExtendFence(m_v_fence_west_left.GetMoved(fence_gap_larger * cos_angle, fence_gap_larger * sin_angle),
					m_v_fence_west_right.GetMoved(fence_gap_smaller *cos_angle, fence_gap_smaller * sin_angle),
					m_v_fence_east_left.GetMoved(fence_gap_smaller * cos_angle, fence_gap_smaller * sin_angle),
					m_v_fence_east_right.GetMoved(fence_gap_larger * cos_angle, fence_gap_larger * sin_angle));

		const unsigned int id_v_fence_west_left = m_last_ids.at(&m_v_fence_west_left);
		const unsigned int id_v_fence_west_right = m_last_ids.at(&m_v_fence_west_right);
		const unsigned int id_v_fence_east_left = m_last_ids.at(&m_v_fence_east_left);
		const unsigned int id_v_fence_east_right = m_last_ids.at(&m_v_fence_east_right);

		const unsigned int id_s_fence_east = m_last_ids.at(&m_s_fence_east);
		const unsigned int id_s_fence_west = m_last_ids.at(&m_s_fence_west);

		const Vertex v_fence_middle_far((m_v_fence_west_left.x + m_v_fence_east_right.x) / 2.0,
										(m_v_fence_west_left.y + m_v_fence_east_right.y) / 2.0);
		const unsigned int id_v_fence_middle_far = m_map << v_fence_middle_far;

		const unsigned int id_sd_fence_east_far_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_east);
		const unsigned int id_sd_fence_east_far_inner = m_map << Sidedef(id_s_fence_east);
		m_map << Linedef(id_v_fence_east_right, id_v_fence_middle_far,
						 id_sd_fence_east_far_inner, id_sd_fence_east_far_outer);

		const unsigned int id_sd_fence_west_far_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_west);
		const unsigned int id_sd_fence_west_far_inner = m_map << Sidedef(id_s_fence_west);
		m_map << Linedef(id_v_fence_middle_far, id_v_fence_west_left,
						 id_sd_fence_west_far_inner, id_sd_fence_west_far_outer);

		const Vertex v_fence_middle_near((m_v_fence_west_right.x + m_v_fence_east_left.x) / 2.0,
										 (m_v_fence_west_right.y + m_v_fence_east_left.y) / 2.0);
		const unsigned int id_v_fence_middle_near = m_map << v_fence_middle_near;

		const unsigned int id_sd_fence_east_near_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_east);
		const unsigned int id_sd_fence_east_near_inner = m_map << Sidedef(id_s_fence_east);
		m_map << Linedef(id_v_fence_middle_near, id_v_fence_east_left,
						 id_sd_fence_east_near_inner, id_sd_fence_east_near_outer);

		const unsigned int id_sd_fence_west_near_outer = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_west);
		const unsigned int id_sd_fence_west_near_inner = m_map << Sidedef(id_s_fence_west);
		m_map << Linedef(id_v_fence_west_right, id_v_fence_middle_near,
						 id_sd_fence_west_near_inner, id_sd_fence_west_near_outer);

		// close road frame
		const unsigned int id_s_west = m_last_ids.at(&m_s_west_side);
		const unsigned int id_s_east = m_last_ids.at(&m_s_east_side);
		const unsigned int id_s_body = m_last_ids.at(&m_s_body);

		const unsigned int id_v_west_left = m_last_ids.at(&m_v_west_left);
		const unsigned int id_v_west_right = m_last_ids.at(&m_v_west_right);
		const unsigned int id_v_east_left = m_last_ids.at(&m_v_east_left);
		const unsigned int id_v_east_right = m_last_ids.at(&m_v_east_right);

		const unsigned int id_sd_west_outer = m_map << m_sd_roadside.GetWithSector(id_s_background_body_west);
		const unsigned int id_sd_west_inner = m_map << Sidedef(id_s_west);
		m_map << Linedef(id_v_west_right, id_v_west_left,
						 id_sd_west_inner, id_sd_west_outer);

		const unsigned int id_sd_east_outer = m_map << m_sd_roadside.GetWithSector(id_s_background_body_east);
		const unsigned int id_sd_east_inner = m_map << Sidedef(id_s_east);
		m_map << Linedef(id_v_east_right, id_v_east_left,
						 id_sd_east_inner, id_sd_east_outer);

		const Vertex v_body_middle((m_v_east_left.x + m_v_west_right.x) / 2.0,
								   (m_v_east_left.y + m_v_west_right.y) / 2.0);
		const unsigned int id_v_body_middle = m_map << v_body_middle;

		const unsigned int id_sd_middle_outer_east = m_map << Sidedef(id_s_background_body_east);
		const unsigned int id_sd_middle_inner_east = m_map << Sidedef(id_s_body);

		const unsigned int id_sd_middle_outer_west = m_map << Sidedef(id_s_background_body_west);
		const unsigned int id_sd_middle_inner_west = m_map << Sidedef(id_s_body);

		const bool have_mark =   !Math::IsZeroOrCloseTo(m_mark_coord) && m_mark_coord < m_config.sizes.road_mark_length
							  || Math::IsZeroOrCloseTo(m_mark_coord - m_config.sizes.road_mark_length);
		if(have_mark && !m_mark_sector_closed) {
			// close road mark sector
			const unsigned int id_v_mark_east = m_last_ids.at(&m_v_mark_east);
			const unsigned int id_v_mark_west = m_last_ids.at(&m_v_mark_west);
			const unsigned int id_s_mark = m_last_ids.at(&m_s_mark);

			m_map << Linedef(id_v_east_left, id_v_mark_east,
							 id_sd_middle_inner_east, id_sd_middle_outer_east);

			const unsigned int id_sd_mark_inner_east = m_map << Sidedef(id_s_mark);
			const unsigned int id_sd_mark_outer_east = m_map << m_sd_markside.GetWithSector(id_s_background_body_east);
			m_map << Linedef(id_v_mark_east, id_v_body_middle,
							 id_sd_mark_inner_east, id_sd_mark_outer_east);

			const unsigned int id_sd_mark_inner_west = m_map << Sidedef(id_s_mark);
			const unsigned int id_sd_mark_outer_west = m_map << m_sd_markside.GetWithSector(id_s_background_body_west);
			m_map << Linedef(id_v_body_middle, id_v_mark_west,
							 id_sd_mark_inner_west, id_sd_mark_outer_west);

			m_map << Linedef(id_v_mark_west, id_v_west_right,
							 id_sd_middle_inner_west, id_sd_middle_outer_west);

			m_mark_sector_closed = true;

		} else {
			// close road frame sector with no road mark
			m_map << Linedef(id_v_east_left, id_v_body_middle,
							 id_sd_middle_inner_east, id_sd_middle_outer_east);
			m_map << Linedef(id_v_body_middle, id_v_west_right,
							 id_sd_middle_inner_west, id_sd_middle_outer_west);
		}

		// draw the line, separating two sectors for each group, that were created for slope needs,
		// groups are: background body, fence
		const unsigned int seam_vertex_ids[] = { 
					id_v_background_middle, 
					id_v_fence_middle_far,
					id_v_fence_middle_near,
					id_v_body_middle };
		const IdPair seam_sector_id_pairs[] = {
			IdPair(id_s_background_body_east, id_s_background_body_west),
			IdPair(id_s_fence_east, id_s_fence_west),
			IdPair(id_s_background_body_east, id_s_background_body_west),
		};

		for(int i=0; i<_countof(seam_sector_id_pairs); i++) {
			const IdPair& sector_id_pair = seam_sector_id_pairs[i];
			const unsigned int id_sd_front = m_map << Sidedef(sector_id_pair.first);
			const unsigned int id_sd_back = m_map << Sidedef(sector_id_pair.second);
			m_map << Linedef(seam_vertex_ids[i], seam_vertex_ids[i+1],
							 id_sd_front, id_sd_back);
		}

		m_sectors_closed = true;
		m_started_drawing = false;
		m_finished_drawing = true;
	}

	RoadFigure& RoadFigure::operator<< (const IRoadFigureObject::LineData& line) {
		if(!m_started_drawing || m_finished_drawing) {
			throw Exception("error generating a road - did not start drawing or already finished drawing before drawing a line");
		}

		if(line.length < 1.0 || line.length > 32767.0) {
			throw Exception("error generating a road - line length must be between 1 and 32767 but got '" + std::to_string(line.length) + "'");
		}

		const double dx = line.length * Math::Cos(m_angle);
		const double dy = line.length * Math::Sin(m_angle);

		const Vertex v_new_background_west_left = m_v_background_west_left.GetMoved(dx, dy);
		const Vertex v_new_background_west_right = m_v_background_west_right.GetMoved(dx, dy);
		const Vertex v_new_background_east_left = m_v_background_east_left.GetMoved(dx, dy);
		const Vertex v_new_background_east_right = m_v_background_east_right.GetMoved(dx, dy);

		ExtendBackground(v_new_background_west_left, v_new_background_west_right,
						 v_new_background_east_left, v_new_background_east_right);

		const Vertex v_new_fence_west_left = m_v_fence_west_left.GetMoved(dx, dy);
		const Vertex v_new_fence_west_right = m_v_fence_west_right.GetMoved(dx, dy);
		const Vertex v_new_fence_east_left = m_v_fence_east_left.GetMoved(dx, dy);
		const Vertex v_new_fence_east_right = m_v_fence_east_right.GetMoved(dx, dy);
		ExtendFence(v_new_fence_west_left, v_new_fence_west_right,
					v_new_fence_east_left, v_new_fence_east_right);

		const Vertex v_new_west_left = m_v_west_left.GetMoved(dx, dy);
		const Vertex v_new_west_right = m_v_west_right.GetMoved(dx, dy);
		const Vertex v_new_east_left = m_v_east_left.GetMoved(dx, dy);
		const Vertex v_new_east_right = m_v_east_right.GetMoved(dx, dy);
		ExtendBody(v_new_west_left, v_new_west_right, v_new_east_left, v_new_east_right);

		const Vertex v_new_mark_west = m_v_mark_west.GetMoved(dx, dy);
		const Vertex v_new_mark_east = m_v_mark_east.GetMoved(dx, dy);
		ExtendMark(v_new_mark_west, v_new_mark_east);

		const Vertex v_new_null = m_v_null.GetMoved(dx, dy);
		SetNullVertex(v_new_null);

		m_sectors_closed = false;

		if(!Math::IsZeroOrCloseTo(m_slope_modifier.tangent)) {
			CloseSectors();
		}

		m_v_mark_ids_prev.clear();
		m_have_shapes = true;
		m_have_shape_with_current_slope = true;

		return *this;
	}

	RoadFigure& RoadFigure::operator<< (const IRoadFigureObject::ArcData& arc) {
		if(!m_started_drawing || m_finished_drawing) {
			throw Exception("error generating a road - did not start drawing or already finished drawing before drawing an arc");
		}

		if(arc.curve < m_total_width/2.0 + 1.0) {
			throw Exception("error generating a road - too small arc radius value '" + std::to_string(arc.curve) + "'");
		}

		if(arc.curve > 65535.0) {
			throw Exception("error generating a road - too big arc radius value, must be less than 65535, but got '" + std::to_string(arc.curve) + "'");
		}

		if(arc.divider < 2) {
			throw Exception("error generating a road - bad arc divider value '" + std::to_string(arc.divider) + "'");
		}

		const double turn_sign = (arc.angle > 0) ? 1.0 : -1.0;
		double angle_abs = turn_sign * arc.angle;

		// numbers close to 2*PI will surely lead to collisions
		// and therefore they are not allowed either
		// (intersection checker will check these collisions)
		if(angle_abs > 2 * Math::PI || Math::IsZeroOrCloseTo(arc.angle)) {
			throw Exception("error generating a road - bad arc angle value '" + std::to_string(arc.angle) + "', must be between -360 and 360 at least"
							" and absolute value must not be too small");
		}

		const double cx = m_v_null.x - turn_sign * arc.curve * Math::Sin(m_angle);
		const double cy = m_v_null.y + turn_sign * arc.curve * Math::Cos(m_angle);
		const int steps_count = arc.divider;
		const double step = angle_abs / steps_count;

		// restrict little angles;
		// figure out, whether it is too small angle or not
		// figure between b and (b + 2*'lineinc') is the one, we draw on every iteration
		//				^			<- 'aleft' (angle left)
		//			   / \
		//			  /   \
		//			 /  b  \
		//			---------		|
		//		   /         \		| 'rightinc'
		//		  /           \		|
		//		  -------------		|
		//		 (b + 2*'lineinc')
		// the goal is: b must not be less that an EPSILON (e. g. 16 units)

		if(Math::IsZeroOrCloseTo(step)) {
			throw Exception("error generating a road - too big divider is specified");

		} else {
			const Vertex& v0 = (turn_sign > 0) ? m_v_background_east_right : m_v_background_west_left;
			const Vertex v = v0.GetRotated(cx, cy, turn_sign * step);
			const double dist = v.GetDistanceTo(v0);
			if(dist < SMALLEST_ARC_LINE) {
				throw Exception("road generation error - arc radius is too small and/or arc divider is too big");
			}
		}

		if(m_intersection_checker) {
			m_intersection_checker->StartEncirclingQuad();
		}

		for(int angix=0; angix<steps_count; angix++) {
			const double da = (angix < (steps_count - 1)) ? step : (angle_abs - (steps_count - 1)*step);

			const double cur_angle = Math::GetNormalizedAngle(m_angle + turn_sign * da);
			const Vertex v_new_null = m_v_null.GetRotated(cx, cy, turn_sign * da);
			std::vector<Vertex*> source_vertices;
			source_vertices.push_back(&m_v_background_west_left);
			source_vertices.push_back(&m_v_background_west_right);
			source_vertices.push_back(&m_v_background_east_left);
			source_vertices.push_back(&m_v_background_east_right);
			source_vertices.push_back(&m_v_fence_west_left);
			source_vertices.push_back(&m_v_fence_west_right);
			source_vertices.push_back(&m_v_fence_east_left);
			source_vertices.push_back(&m_v_fence_east_right);
			source_vertices.push_back(&m_v_west_left);
			source_vertices.push_back(&m_v_west_right);
			source_vertices.push_back(&m_v_east_left);
			source_vertices.push_back(&m_v_east_right);
			source_vertices.push_back(&m_v_mark_west);
			source_vertices.push_back(&m_v_mark_east);

			// map: prev vertex address -> new vertex
			std::unordered_map<Vertex*, Vertex> vertices;
			GetMovedVertices(source_vertices, m_v_null, v_new_null, cur_angle, vertices);

			ExtendBackground(vertices.at(&m_v_background_west_left), vertices.at(&m_v_background_west_right),
							 vertices.at(&m_v_background_east_left), vertices.at(&m_v_background_east_right));
			ExtendFence(vertices.at(&m_v_fence_west_left), vertices.at(&m_v_fence_west_right),
						vertices.at(&m_v_fence_east_left), vertices.at(&m_v_fence_east_right));
			ExtendBody(vertices.at(&m_v_west_left), vertices.at(&m_v_west_right),
					   vertices.at(&m_v_east_left), vertices.at(&m_v_east_right));
			ExtendMark(vertices.at(&m_v_mark_west), vertices.at(&m_v_mark_east));

			SetNullVertex(v_new_null);

			m_sectors_closed = false;

			if(!Math::IsZeroOrCloseTo(m_slope_modifier.tangent)) {
				CloseSectors();
			}

			m_v_mark_ids_prev.clear();
			m_angle = cur_angle;
		}

		if(m_intersection_checker) {
			m_intersection_checker->EndEncirclingQuad();
		}

		m_have_shapes = true;
		m_have_shape_with_current_slope = true;

		return *this;
	}

	RoadFigure& RoadFigure::operator<< (const SlopeModifier& slope_modifier) {
		if(!m_started_drawing || m_finished_drawing) {
			throw Exception("error generating a road - did not start drawing or already finished drawing before adding a slope");
		}

		if(!Math::IsZeroOrCloseTo(m_slope_modifier.tangent) && !m_have_shapes) {
			throw Exception("error generating a road - slope in the beginning of a figure is not allowed, add a line/arc first");
		}

		if(m_slope_modifier.tangent < -0.5 || m_slope_modifier.tangent > 0.5) {
			throw Exception("error generating a road - absolute value of slope tangent is too big, must be 0.5 or less");
		}

		if(Math::IsZeroOrCloseTo(m_slope_modifier.tangent)) {
			CloseSectors();
		}

		m_slope_modifier = slope_modifier;
		m_have_shape_with_current_slope = false;
		return *this;
	}

	void RoadFigure::InitMapElements() {
		m_sd_roadside = Sidedef().GetWithBottomTexture(RoadConfig::enTexture_RoadSideWall);

		m_s_body = Sector(m_floorpos, m_ceilingpos,
						  RoadConfig::enTexture_RoadBody, RoadConfig::enTexture_Sky,
						  m_config.light_level);
		m_s_east_side =
		m_s_west_side = Sector(m_floorpos + m_config.sizes.road_side_height, m_ceilingpos,
							   RoadConfig::enTexture_RoadSide, RoadConfig::enTexture_Sky,
							   m_config.light_level);

		m_sd_markside = Sidedef().GetWithBottomTexture(RoadConfig::enTexture_RoadMark);

		m_s_mark = Sector(m_floorpos + MARK_HEIGHT, m_ceilingpos,
						  RoadConfig::enTexture_RoadMark, RoadConfig::enTexture_Sky,
						  m_config.light_level);

		m_s_background_body_east =
		m_s_background_body_west = Sector(m_floorpos, m_ceilingpos,
										  RoadConfig::enTexture_Background, RoadConfig::enTexture_Sky,
										  m_config.light_level);

		m_s_background_sky = Sector(m_floorpos, m_floorpos,
									RoadConfig::enTexture_Background, RoadConfig::enTexture_Sky,
									m_config.light_level);

		m_sd_fenceside = Sidedef().GetWithBottomTexture(RoadConfig::enTexture_Fence);
		
		m_s_fence_east =
		m_s_fence_west = Sector(m_floorpos + m_config.sizes.fence_height, m_ceilingpos,
								RoadConfig::enTexture_FenceFloor, RoadConfig::enTexture_Sky,
								m_config.light_level);
	}

	void RoadFigure::GetMovedVertices(const std::vector<Vertex*>& source_vertices,
									  const Vertex& v_prev_null,
									  const Vertex& v_null,
									  double angle,
									  std::unordered_map<Vertex*, Vertex>& vertices)
	{
		vertices.clear();
		if(source_vertices.empty()) {
			return;
		}

		const double dir_prev_x = source_vertices[0]->x - v_prev_null.x;
		const double dir_prev_y = source_vertices[0]->y - v_prev_null.y;

		const double p_dir_x = -Math::Sin(angle);
		const double p_dir_y = Math::Cos(angle);

		const bool dir_ang_positive = (dir_prev_x*p_dir_x + dir_prev_y*p_dir_y) > 0;
		
		// consider that angle between dirs is always acute (do not turn around like 90+ degrees)
		const double dir_x = dir_ang_positive ? p_dir_x : -p_dir_x;
		const double dir_y = dir_ang_positive ? p_dir_y : -p_dir_y;

		for(int i=0; i<source_vertices.size(); i++) {
			const Vertex& v = *source_vertices[i];

			const double dist = v.GetDistanceTo(v_prev_null);

			const double v_dir_prev_x = v.x - v_prev_null.x;
			const double v_dir_prev_y = v.y - v_prev_null.y;

			bool dir_suits = false;
			if(!Math::IsZeroOrCloseTo(v_dir_prev_x) && !Math::IsZeroOrCloseTo(dir_prev_x)) {
				if((v_dir_prev_x > 0 && dir_prev_x > 0) || (v_dir_prev_x < 0 && dir_prev_x < 0)) {
					dir_suits = true;
				}
			} else if(!Math::IsZeroOrCloseTo(v_dir_prev_y) && !Math::IsZeroOrCloseTo(dir_prev_y)) {
				if((v_dir_prev_y > 0 && dir_prev_y > 0) || (v_dir_prev_y < 0 && dir_prev_y < 0)) {
					dir_suits = true;
				}
			}

			const double v_dir_x = dir_suits ? dir_x : -dir_x;
			const double v_dir_y = dir_suits ? dir_y : -dir_y;

			vertices[source_vertices[i]] = Vertex(v_null.x + dist * v_dir_x, v_null.y + dist * v_dir_y);
		}
	}

	void RoadFigure::SetNullVertex(const Vertex& v_new_null) {
		m_v_null_prev = m_v_null;
		m_v_null = v_new_null;
	}

	void RoadFigure::ExtendBackground(const Vertex& v_new_west_left,
									  const Vertex& v_new_west_right,
									  const Vertex& v_new_east_left,
									  const Vertex& v_new_east_right)
	{
		if(m_intersection_checker) {
			m_intersection_checker->AddQuad(m_v_background_west_left, v_new_west_left, v_new_east_right, m_v_background_east_right);
		}

		const unsigned int id_v_prev_background_west_left = m_last_ids.at(&m_v_background_west_left);
		const unsigned int id_v_prev_background_west_right = m_last_ids.at(&m_v_background_west_right);
		const unsigned int id_v_prev_background_east_left = m_last_ids.at(&m_v_background_east_left);
		const unsigned int id_v_prev_background_east_right = m_last_ids.at(&m_v_background_east_right);

		m_v_background_west_left = v_new_west_left;
		m_v_background_west_right = v_new_west_right;
		m_v_background_east_left = v_new_east_left;
		m_v_background_east_right = v_new_east_right;
		
		m_last_ids[&m_v_background_west_left] = m_map << m_v_background_west_left;
		m_last_ids[&m_v_background_west_right] = m_map << m_v_background_west_right;
		m_last_ids[&m_v_background_east_left] = m_map << m_v_background_east_left;
		m_last_ids[&m_v_background_east_right] = m_map << m_v_background_east_right;

		const unsigned int id_s_background_sky = m_last_ids.at(&m_s_background_sky);

		const unsigned int id_sd_background_sky_west = m_map << Sidedef(id_s_background_sky);
		m_map << Linedef(m_last_ids.at(&m_v_background_west_left), id_v_prev_background_west_left,
						 id_sd_background_sky_west);

		const unsigned int id_sd_background_sky_east = m_map << Sidedef(id_s_background_sky);
		m_map << Linedef(id_v_prev_background_east_right, m_last_ids.at(&m_v_background_east_right),
						 id_sd_background_sky_east);

		const unsigned int id_sd_background_body_outer_west = m_map << Sidedef(id_s_background_sky);
		const unsigned int id_sd_background_body_inner_west = m_map << Sidedef(m_last_ids.at(&m_s_background_body_west));
		m_map << Linedef(m_last_ids.at(&m_v_background_west_right), id_v_prev_background_west_right,
						 id_sd_background_body_inner_west, id_sd_background_body_outer_west);

		const unsigned int id_sd_background_body_outer_east = m_map << Sidedef(id_s_background_sky);
		const unsigned int id_sd_background_body_inner_east = m_map << Sidedef(m_last_ids.at(&m_s_background_body_east));
		m_map << Linedef(id_v_prev_background_east_left, m_last_ids.at(&m_v_background_east_left),
						 id_sd_background_body_inner_east, id_sd_background_body_outer_east);
	}

	void RoadFigure::ExtendFence(const Vertex& v_new_west_left,
								 const Vertex& v_new_west_right,
								 const Vertex& v_new_east_left,
								 const Vertex& v_new_east_right)
	{
		const unsigned int id_v_prev_west_left = m_last_ids.at(&m_v_fence_west_left);
		const unsigned int id_v_prev_west_right = m_last_ids.at(&m_v_fence_west_right);
		const unsigned int id_v_prev_east_left = m_last_ids.at(&m_v_fence_east_left);
		const unsigned int id_v_prev_east_right = m_last_ids.at(&m_v_fence_east_right);

		m_v_fence_west_left = v_new_west_left;
		m_v_fence_west_right = v_new_west_right;
		m_v_fence_east_left = v_new_east_left;
		m_v_fence_east_right = v_new_east_right;
		
		m_last_ids[&m_v_fence_west_left] = m_map << m_v_fence_west_left;
		m_last_ids[&m_v_fence_west_right] = m_map << m_v_fence_west_right;
		m_last_ids[&m_v_fence_east_left] = m_map << m_v_fence_east_left;
		m_last_ids[&m_v_fence_east_right] = m_map << m_v_fence_east_right;

		const unsigned int id_s_background_body_west = m_last_ids.at(&m_s_background_body_west);
		const unsigned int id_s_west = m_last_ids.at(&m_s_fence_west);

		const unsigned int id_sd_fence_outer_west_left = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_west);
		const unsigned int id_sd_fence_inner_west_left = m_map << Sidedef(id_s_west);
		m_map << Linedef(m_last_ids.at(&m_v_fence_west_left), id_v_prev_west_left,
						 id_sd_fence_inner_west_left, id_sd_fence_outer_west_left);

		const unsigned int id_sd_fence_outer_west_right = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_west);
		const unsigned int id_sd_fence_inner_west_right = m_map << Sidedef(id_s_west);
		m_map << Linedef(id_v_prev_west_right, m_last_ids.at(&m_v_fence_west_right),
						 id_sd_fence_inner_west_right, id_sd_fence_outer_west_right);

		const unsigned int id_s_background_body_east = m_last_ids.at(&m_s_background_body_east);
		const unsigned int id_s_east = m_last_ids.at(&m_s_fence_east);

		const unsigned int id_sd_fence_outer_east_left = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_east);
		const unsigned int id_sd_fence_inner_east_left = m_map << Sidedef(id_s_east);
		m_map << Linedef(m_last_ids.at(&m_v_fence_east_left), id_v_prev_east_left,
						 id_sd_fence_inner_east_left, id_sd_fence_outer_east_left);

		const unsigned int id_sd_fence_outer_east_right = m_map << m_sd_fenceside.GetWithSector(id_s_background_body_east);
		const unsigned int id_sd_fence_inner_east_right = m_map << Sidedef(id_s_east);
		m_map << Linedef(id_v_prev_east_right, m_last_ids.at(&m_v_fence_east_right),
						 id_sd_fence_inner_east_right, id_sd_fence_outer_east_right);
	}

	void RoadFigure::ExtendBody(const Vertex& v_new_west_left,
								const Vertex& v_new_west_right,
								const Vertex& v_new_east_left,
								const Vertex& v_new_east_right)
	{
		// vertices
		const unsigned int id_v_prev_west_left = m_last_ids.at(&m_v_west_left);
		const unsigned int id_v_prev_west_right = m_last_ids.at(&m_v_west_right);
		const unsigned int id_v_prev_east_left = m_last_ids.at(&m_v_east_left);
		const unsigned int id_v_prev_east_right = m_last_ids.at(&m_v_east_right);

		m_v_west_left = v_new_west_left;
		m_v_west_right = v_new_west_right;
		m_v_east_left = v_new_east_left;
		m_v_east_right = v_new_east_right;

		m_last_ids[&m_v_west_left] = m_map << m_v_west_left;
		m_last_ids[&m_v_west_right] = m_map << m_v_west_right;
		m_last_ids[&m_v_east_left] = m_map << m_v_east_left;
		m_last_ids[&m_v_east_right] = m_map << m_v_east_right;

		const unsigned int id_sd_west_outer = m_map << m_sd_roadside.GetWithSector(m_last_ids.at(&m_s_background_body_west));
		const unsigned int id_sd_west_left = m_map << Sidedef(m_last_ids.at(&m_s_west_side));
		m_map << Linedef(m_last_ids.at(&m_v_west_left), id_v_prev_west_left,
						 id_sd_west_left, id_sd_west_outer);

		const unsigned int id_sd_west_right = m_map << Sidedef(m_last_ids.at(&m_s_west_side));
		const unsigned int id_sd_body_left = m_map << m_sd_roadside.GetWithSector(m_last_ids.at(&m_s_body));
		m_map << Linedef(m_last_ids.at(&m_v_west_right), id_v_prev_west_right,
						 id_sd_body_left, id_sd_west_right);

		const unsigned int id_sd_east_left = m_map << Sidedef(m_last_ids.at(&m_s_east_side));
		const unsigned int id_sd_body_right = m_map << m_sd_roadside.GetWithSector(m_last_ids.at(&m_s_body));
		m_map << Linedef(id_v_prev_east_left, m_last_ids.at(&m_v_east_left),
						 id_sd_body_right, id_sd_east_left);

		const unsigned int id_sd_east_outer = m_map << m_sd_roadside.GetWithSector(m_last_ids.at(&m_s_background_body_east));
		const unsigned int id_sd_east_right = m_map << Sidedef(m_last_ids.at(&m_s_east_side));
		m_map << Linedef(id_v_prev_east_right, m_last_ids.at(&m_v_east_right),
						 id_sd_east_right, id_sd_east_outer);
	}

	void RoadFigure::ExtendMark(const Vertex& v_new_mark_west, const Vertex& v_new_mark_east) {
		const Vertex v_west_dir(v_new_mark_west.x - m_v_mark_west.x, v_new_mark_west.y - m_v_mark_west.y);
		const Vertex v_east_dir(v_new_mark_east.x - m_v_mark_east.x, v_new_mark_east.y - m_v_mark_east.y);

		const double length_east = m_v_mark_east.GetDistanceTo(v_new_mark_east);
		const double length_west = m_v_mark_west.GetDistanceTo(v_new_mark_west);
		const double length_middle = (length_east + length_west) / 2.0;

		double length = length_middle;
		if(length < SMALLEST_MARK_LINE) {
			// should not happen
			throw Exception("road generation error - too small mark to draw");
		}

		const auto id_v_mark_west_last_it = m_last_ids.find(&m_v_mark_west);
		const auto id_v_mark_east_last_it = m_last_ids.find(&m_v_mark_east);
		const bool is_v_mark_west_here =   id_v_mark_west_last_it != m_last_ids.end()
										&& Math::IsZeroOrCloseTo(m_v_mark_west.GetDistanceTo(m_map.vertices.at(id_v_mark_west_last_it->second)));
		const bool is_v_mark_east_here =    id_v_mark_east_last_it != m_last_ids.end()
										&& Math::IsZeroOrCloseTo(m_v_mark_east.GetDistanceTo(m_map.vertices.at(id_v_mark_east_last_it->second)));

		bool first_vertices_added = Math::IsZeroOrCloseTo(m_mark_coord) && is_v_mark_west_here && is_v_mark_east_here;

		const bool have_slope = !Math::IsZeroOrCloseTo(m_slope_modifier.tangent);

		// skip small lengths - just do not change the state
		while(length >= 1.0) {
			if(Math::IsZeroOrCloseTo(m_mark_coord) && !first_vertices_added) {
				// start drawing, insert vertices, sector and line
				m_last_ids[&m_v_mark_west] = m_map << m_v_mark_west;
				m_last_ids[&m_v_mark_east] = m_map << m_v_mark_east;

				m_last_ids[&m_s_mark] = m_map << m_s_mark;

				const unsigned int id_sd_outside_west = m_map << m_sd_markside.GetWithSector(m_last_ids.at(&m_s_body));
				const unsigned int id_sd_inside_west = m_map << Sidedef(m_last_ids.at(&m_s_mark));
				m_map << Linedef(m_last_ids.at(&m_v_mark_west), m_last_ids.at(&m_v_mark_east),
								 id_sd_inside_west, id_sd_outside_west);

				m_v_mark_ids_prev.push_back(std::make_pair(length_middle - length,
														   IdPair(m_last_ids.at(&m_v_mark_west), m_last_ids.at(&m_v_mark_east))));
			}
			first_vertices_added = false;

			if(m_mark_coord < m_config.sizes.road_mark_length && !Math::IsZeroOrCloseTo(m_mark_coord - m_config.sizes.road_mark_length)) {
				// inside the mark drawing - insert vertices
				const double diff = m_config.sizes.road_mark_length - m_mark_coord;
				const double mark_length = (diff > length) ? length : diff;

				const unsigned int id_v_prev_west = m_last_ids.at(&m_v_mark_west);
				const unsigned int id_v_prev_east = m_last_ids.at(&m_v_mark_east);

				const double mark_length_west = mark_length / length_middle;
				const double mark_length_east = mark_length / length_middle;

				m_v_mark_west = m_v_mark_west.GetMoved(mark_length_west * v_west_dir.x, mark_length_west * v_west_dir.y);
				m_v_mark_east = m_v_mark_east.GetMoved(mark_length_east * v_east_dir.x, mark_length_east * v_east_dir.y);

				m_last_ids[&m_v_mark_west] = m_map << m_v_mark_west;
				m_last_ids[&m_v_mark_east] = m_map << m_v_mark_east;

				const unsigned int id_s_mark = m_last_ids.at(&m_s_mark);
				if(have_slope) {
					// triangulate in case of slope to make vertex heights work
					m_last_ids[&m_s_mark] = m_map << m_s_mark;
				}
				const unsigned int id_s_mark_new = m_last_ids.at(&m_s_mark);

				// insert east and west lines
				const unsigned int id_sd_outside_west = m_map << m_sd_markside.GetWithSector(m_last_ids.at(&m_s_body));
				const unsigned int id_sd_inside_west = m_map << Sidedef(id_s_mark_new);
				m_map << Linedef(m_last_ids.at(&m_v_mark_west), id_v_prev_west,
									id_sd_inside_west, id_sd_outside_west);

				const unsigned int id_sd_outside_east = m_map << m_sd_markside.GetWithSector(m_last_ids.at(&m_s_body));
				const unsigned int id_sd_inside_east = m_map << Sidedef(id_s_mark);
				m_map << Linedef(id_v_prev_east, m_last_ids.at(&m_v_mark_east),
									id_sd_inside_east, id_sd_outside_east);

				if(have_slope) {
					// triangulate - divide quad
					const unsigned int id_sd_front = m_map << Sidedef(id_s_mark);
					const unsigned int id_sd_back = m_map << Sidedef(id_s_mark_new);
					Linedef l(m_last_ids.at(&m_v_mark_east), id_v_prev_west,
							  id_sd_front, id_sd_back);
					l.dontdraw = true;
					m_map << l;
				}

				m_v_mark_ids_prev.push_back(std::make_pair(length_middle - (length - mark_length),
															IdPair(m_last_ids.at(&m_v_mark_west), m_last_ids.at(&m_v_mark_east))));

				m_mark_coord += mark_length;
				length -= mark_length;
				m_mark_sector_closed = false;

			} else {
				if(Math::IsZeroOrCloseTo(m_mark_coord - m_config.sizes.road_mark_length) && !m_mark_sector_closed) {
					// end current mark - insert line
					const auto id_v_mark_west_it = m_last_ids.find(&m_v_mark_west);
					const auto id_v_mark_east_it = m_last_ids.find(&m_v_mark_east);
					if(id_v_mark_west_it != m_last_ids.end() && id_v_mark_east_it != m_last_ids.end()) {
						const unsigned int id_sd_outside_west = m_map << m_sd_markside.GetWithSector(m_last_ids.at(&m_s_body));
						const unsigned int id_sd_inside_west = m_map << Sidedef(m_last_ids.at(&m_s_mark));
						m_map << Linedef(id_v_mark_east_it->second, id_v_mark_west_it->second,
										 id_sd_inside_west, id_sd_outside_west);
					}

					m_mark_sector_closed = true;
				}

				// empty space - skip, don't draw anything
				const double diff = m_config.sizes.road_mark_length + m_config.sizes.road_mark_gap - m_mark_coord;
				const double space_length = (diff > length) ? length : diff;

				const double space_length_west = space_length / length_middle;
				const double space_length_east = space_length / length_middle;

				m_v_mark_west = m_v_mark_west.GetMoved(space_length_west * v_west_dir.x, space_length_west * v_west_dir.y);
				m_v_mark_east = m_v_mark_east.GetMoved(space_length_east * v_east_dir.x, space_length_east * v_east_dir.y);

				m_mark_coord = !Math::IsZeroOrCloseTo(space_length - diff) ? (m_mark_coord + space_length) : 0;
				length -= space_length;
			}
		}
	}

	void RoadFigure::CloseSectors() {
		if(m_sectors_closed) {
			// nothing to close
			return;
		}

		const bool have_slope = !Math::IsZeroOrCloseTo(m_slope_modifier.tangent);
		const bool at_mark_start = Math::IsZeroOrCloseTo(m_mark_coord);
		const bool at_mark_end = Math::IsZeroOrCloseTo(m_mark_coord - m_config.sizes.road_mark_length);
		const bool have_mark = m_mark_coord < m_config.sizes.road_mark_length || at_mark_start || at_mark_end;

		// modify all sectors according to the slope modifier
		const int floorpos_prev = m_floorpos;
		if(have_slope) {
			Sector* body_west_sector_ptr =
				(m_last_ids.at(&m_s_background_body_east) != m_last_ids.at(&m_s_background_body_west))
					? &m_s_background_body_west
					: NULL;
			Sector* fence_west_sector_ptr =
				(m_last_ids.at(&m_s_fence_east) != m_last_ids.at(&m_s_fence_west))
					? &m_s_fence_west
					: NULL;
			Sector* slope_sectors[] = {
				&m_s_background_sky, &m_s_background_body_east, body_west_sector_ptr,
				&m_s_fence_east, fence_west_sector_ptr,
				&m_s_west_side, &m_s_east_side,
				&m_s_body, &m_s_mark
			};

			// make height difference between sectors, then make slopes using linedef specials;
			// determine the value by which the far bound will be lowered
			const double width_diff_f = m_v_null.GetDistanceTo(m_v_null_prev);
			const int height_diff = (int)(m_slope_modifier.tangent * width_diff_f);
			if(m_floorpos + height_diff >= m_ceilingpos - m_config.sizes.fence_height) {
				throw Exception("error generating a road - slope goes up after the ceiling, change slope tangent and/or figure height");
			}

			const bool had_mark = !have_mark && (m_mark_coord - width_diff_f) < m_config.sizes.road_mark_length;
			for(unsigned int i=0; i<_countof(slope_sectors); i++) {
				if(!slope_sectors[i]) {
					continue;
				}

				// modify new sector and maybe already added sector too
				Sector& cur_sector = *slope_sectors[i];

				// propagate floor changes to the ceiling for background
				const bool is_background_sky_sector = slope_sectors[i] == &m_s_background_sky;

				const int new_floorpos = cur_sector.heightfloor + height_diff;
				cur_sector.heightfloor = new_floorpos;
				const int new_heightceiling = cur_sector.heightceiling + height_diff;
				if(is_background_sky_sector) {
					cur_sector.heightceiling = new_heightceiling;
				}

				bool modify_prev_sector = true;
				if(m_slope_modifier.tangent > 0.0 && is_background_sky_sector) {
					modify_prev_sector = false;

				} else if(slope_sectors[i] == &m_s_mark && !(have_mark && !at_mark_start || had_mark)) {
					modify_prev_sector = false;
				}

				if(modify_prev_sector) {
					Sector& prev_sector = m_map.sectors.at(m_last_ids.at(slope_sectors[i]));
					prev_sector.heightfloor = new_floorpos;
					if(is_background_sky_sector) {
						prev_sector.heightceiling = new_heightceiling;
					}
				}
			}

			m_floorpos += height_diff;
		}

		if(at_mark_start) {
			// in case here is the very beginning of the new mark
			// the linedef between mark vertices will be special
			// having one sidedef as a part of the mark sector (next sector set, if it is a slope)
			// and other one as a part of the body sector (current sector set, if it is a slope)
			m_last_ids[&m_v_mark_west] = m_map << m_v_mark_west.GetWithZFloor(m_floorpos);
			m_last_ids[&m_v_mark_east] = m_map << m_v_mark_east.GetWithZFloor(m_floorpos);
			//m_v_mark_ids_prev.push_back(std::make_pair(0, IdPair(m_last_ids.at(&m_v_mark_west), m_last_ids.at(&m_v_mark_east))));
		}

		if(!have_slope) {
			// closing sectors before the slope - assign z position
			// for bounding vertices of the mark
			if(have_mark && !at_mark_start) {
				const auto id_v_mark_west_it = m_last_ids.find(&m_v_mark_west);
				const auto id_v_mark_east_it = m_last_ids.find(&m_v_mark_east);
				if (id_v_mark_west_it != m_last_ids.end()
					&& id_v_mark_east_it != m_last_ids.end()
					&& Math::IsZeroOrCloseTo(m_map.vertices.at(id_v_mark_west_it->second).GetDistanceTo(m_v_mark_west))
					&& Math::IsZeroOrCloseTo(m_map.vertices.at(id_v_mark_east_it->second).GetDistanceTo(m_v_mark_east)))
				{
					m_v_mark_ids_prev.push_back(std::make_pair(0, IdPair(id_v_mark_west_it->second, id_v_mark_east_it->second)));
				}
			}
		}

		// mark sector may be surrounded by a single body sector;
		// assign height to mark vertices to make slopes
		for(auto it = m_v_mark_ids_prev.begin(); it != m_v_mark_ids_prev.end(); it++) {
			bool have_cur_vertices = false;
			if(have_mark) {
				const unsigned int id_v_mark_west_prev = m_last_ids.at(&m_v_mark_west);
				if(id_v_mark_west_prev == it->second.first || id_v_mark_west_prev == it->second.second) {
					have_cur_vertices = true;
				}
			}

			const double vertex_height_diff = m_slope_modifier.tangent * it->first;
			double mark_floorpos = (floorpos_prev + MARK_HEIGHT) + vertex_height_diff;
			if(have_cur_vertices) {
				mark_floorpos = (mark_floorpos > 0) ? Math::Floor(mark_floorpos) : Math::Ceil(mark_floorpos);
			}

			// set floor pos for each of two mark vertices
			Vertex& mark_v1 = m_map.vertices.at(it->second.first);
			mark_v1 = mark_v1.GetWithZFloor(mark_floorpos);
			Vertex& mark_v2 = m_map.vertices.at(it->second.second);
			mark_v2 = mark_v2.GetWithZFloor(mark_floorpos);
		}

		//
		// close sectors
		//

		// close each sector the same way - create a line (linedef + 2 sidedefs),
		// create new sector to replace the old one in the flow

		std::vector<Vertex*> vertices;
		vertices.push_back(&m_v_background_west_left);
		vertices.push_back(&m_v_background_west_right);
		vertices.push_back(&m_v_fence_west_left);
		vertices.push_back(&m_v_fence_west_right);
		vertices.push_back(&m_v_west_left);
		vertices.push_back(&m_v_west_right);
		if(have_mark) {
			vertices.push_back(&m_v_mark_west);
			vertices.push_back(&m_v_mark_east);
		}
		vertices.push_back(&m_v_east_left);
		vertices.push_back(&m_v_east_right);
		vertices.push_back(&m_v_fence_east_left);
		vertices.push_back(&m_v_fence_east_right);
		vertices.push_back(&m_v_background_east_left);
		vertices.push_back(&m_v_background_east_right);

		std::vector<Sector*> sectors;
		sectors.push_back(&m_s_background_sky);
		sectors.push_back(&m_s_background_body_west);
		sectors.push_back(&m_s_fence_west);
		sectors.push_back(&m_s_background_body_west);
		sectors.push_back(&m_s_west_side);
		sectors.push_back(&m_s_body);
		if(have_mark) {
			sectors.push_back(&m_s_mark);
			sectors.push_back(&m_s_body);
		}
		sectors.push_back(&m_s_east_side);
		sectors.push_back(&m_s_background_body_east);
		sectors.push_back(&m_s_fence_east);
		sectors.push_back(&m_s_background_body_east);
		sectors.push_back(&m_s_background_sky);

		// array of sectors may contain the same sector more than once
		std::unordered_map<void *, unsigned int> prev_sectors_ids;
		for(int i=0; i<sectors.size(); i++) {
			Sector* sector = sectors[i];
			if(prev_sectors_ids.find(sector) != prev_sectors_ids.end()) {
				// already processed
				continue;
			}

			if(at_mark_start && sector == &m_s_mark) {
				// do not have previous sector

			} else {
				prev_sectors_ids[sector] = m_last_ids.at(sector);
			}

			if(at_mark_end && sector == &m_s_mark) {
				// do not have next sector

			} else {
				m_last_ids[sector] = m_map << *sector;
			}
		}

		for(int i=0; i<sectors.size(); i++) {
			Sector* sector = sectors[i];
			const unsigned int id_sd_front =
				(at_mark_end && sector == &m_s_mark)
					? m_map << m_sd_roadside.GetWithSector(m_last_ids.at(&m_s_body))
					: m_map << Sidedef(m_last_ids.at(sector));
			const unsigned int id_sd_back =
				(at_mark_start && sector == &m_s_mark)
					? m_map << m_sd_roadside.GetWithSector(prev_sectors_ids.at(&m_s_body))
					: m_map << Sidedef(prev_sectors_ids.at(sector));

			Linedef l(m_last_ids.at(vertices[i]), m_last_ids.at(vertices[i+1]),
					  id_sd_front, id_sd_back);
			l.dontdraw = true;

			if(vertices[i] != &m_v_mark_west) {
				// action 181 - Plane Aling (slope), 'arg0 = 1' - aling floor on front
				const bool have_slope =    (vertices[i] != &m_v_background_west_left)
										&& (vertices[i] != &m_v_background_east_left);
				if(have_slope) {
					l.action_special.special = 181;
					l.action_special.arg0 = 1;
				}

			} else {
				m_mark_sector_closed = true;
			}

			m_map << l;
		}

		m_sectors_closed = true;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GetGeneratedRoad implementation

namespace RoadGen {
	void GetGeneratedRoad(const RoadFigureDataConst& input_data, const RoadConfig& road_config, Udmf::Map& map) {
		map = Udmf::Map();

		// set textures first
		for(int i=0; i<RoadConfig::EnTexture_Size; i++) {
			map.textures.push_back(road_config.textures[i]);
		}

		IntersectionChecker intersection_checker;

		// default start data
		IRoadFigureObject::StartModifierData default_start_data;
		default_start_data.x_null = 0.0;
		default_start_data.y_null = 0.0;
		default_start_data.angle = 0.0;
		default_start_data.zpos = 0;
		default_start_data.height = 1024;
		default_start_data.mark_shift = road_config.sizes.road_mark_length + 3 * road_config.sizes.road_mark_gap / 4;
		std::unique_ptr<RoadFigure> cur_figure(new RoadFigure(road_config, default_start_data, map, &intersection_checker));
		bool drawing_started = false;

		Udmf::Map::PlayerStartInfo player_start_info;
		player_start_info.x = 0.0;
		player_start_info.y = 0.0;
		player_start_info.deg_angle = 0;
		bool have_player_start = false;

		for(int i=0; i<input_data.size(); i++) {
			const IRoadFigureObjectConstPtr& obj_ptr = input_data[i];
			const IRoadFigureObject::EnType obj_type = obj_ptr->GetType();
			if(obj_type == IRoadFigureObject::enType_StartModifier) {
				// a new figure
				if(cur_figure != NULL && drawing_started) {
					cur_figure->Finish();
				}

				const IRoadFigureObject::StartModifierData* start_data = obj_ptr->GetStartModifierData();
				cur_figure = std::unique_ptr<RoadFigure>(new RoadFigure(road_config, *start_data, map, &intersection_checker));
				intersection_checker.CutFigure();
				drawing_started = false;

				player_start_info.x = start_data->x_null;
				player_start_info.y = start_data->y_null;
				player_start_info.deg_angle = (int)Math::RadiansToDegrees(Math::GetNormalizedAngle(start_data->angle));

				continue;
			}

			if(!drawing_started) {
				cur_figure->Start();
				drawing_started = true;
			}

			if(!have_player_start) {
				map << player_start_info;
				have_player_start = true;
			}

			// extend existing figure
			switch(obj_type) {
			case IRoadFigureObject::enType_Line:
				*cur_figure << *obj_ptr->GetLineData();
				break;

			case IRoadFigureObject::enType_Arc:
				*cur_figure << *obj_ptr->GetArcData();
				break;

			case IRoadFigureObject::enType_SlopeModifier:
				*cur_figure << *obj_ptr->GetSlopeModifierData();
				break;

			default:
				throw Exception("road generation error - unknown road figure object type at pos " + std::to_string(i) + " to draw");
			}
		}

		if(cur_figure != NULL && drawing_started) {
			cur_figure->Finish();
		}
	}
}
