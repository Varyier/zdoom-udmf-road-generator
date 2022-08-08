
#include "core.h"
#include "io.h"

#include <fstream>
#include <iostream>

using namespace RoadGen;


static void ReadRoadInputDataFromTableConfigStream(Io::InStream& in_stream, RoadFigureDataConst& road_data);

int main(int argc, char *argv[]) {
	const StringArray args_and_opts(argv, argv + argc);

	const std::string oname_config("-config");
	const std::string oname_mapname("-mapname");

	bool display_help = false;
	std::string config_file_path;
	std::string map_name = "MAP01";
	StringArray args;
	for(size_t i=1; i<args_and_opts.size(); i++) {
		const std::string& arg = args_and_opts[i];

		if(arg.empty()) {
			continue;
		}

		if(arg == "/?") {
			display_help = true;

		} else if(arg[0] == '-') {
			// option
			if(arg == oname_config) {
				if((i+1) >= args_and_opts.size() || args_and_opts[i+1].empty()) {
					std::cout << "Error: a file path must be specified after option '" + arg + "'" << std::endl;
					return 1;
				}

				config_file_path = args_and_opts[i+1];
				i++;

			} else if(arg == oname_mapname) {
				if((i+1) >= args_and_opts.size() || args_and_opts[i+1].empty()) {
					std::cout << "Error: a lump name must be specified after option '" + arg + "'" << std::endl;
					return 1;
				}

				const std::string& value = args_and_opts[i+1];
				if(!Io::WadWriter::IsValidLumpName(value)) {
					std::cout << "Error: bad map name '" + value + "' - must be valid lump name" << std::endl;
					return 1;
				}

				map_name = value;
				i++;

			} else if(arg == "--help" || arg == "-h" || arg == "/?") {
				// display help
				display_help = true;

			} else {
				std::cout << "Error: unknown option - " + arg;
				return 1;
			}

		} else {
			// argument
			args.push_back(arg);
		}
	}

	if(args.empty() || display_help) {
		std::cout << "road-gen - generates roads for Doom maps in UDMF format. "
		             "The tool takes road input data with optional config file and creates the TEXTMAP. "
		             "TEXTMAP is packed into the output WAD file." << std::endl;
		std::cout << "Usage:" << std::endl;
		std::cout << "  road-gen.exe <input-file-path> [<output-file-path>] [<options>]" << std::endl;
		std::cout << "Arguments:" << std::endl;
		std::cout << "  <input-file-path> - file path with road figures to generate (required)" << std::endl;
		std::cout << "  <output-file-path> - output WAD file path (optional, default - 'roads.wad')" << std::endl;
		std::cout << "Options: " << std::endl;
		std::cout << "  -config <file-path> - road config file path; allows to change road sizes, textures, light settings, etc." << std::endl;
		std::cout << "  -mapname <map-marker-lump-name> - output map name in the resulting WAD file; must be valid ZDoom map name, 8 chars maximum length (default - MAP01)" << std::endl;
		std::cout << "  --help, -h or /? - display this message" << std::endl;

		if(args.empty()) {
			return 0;
		}
	}

	//
	// 1) Read road config data from file (if file is specified)
	//

	Io::ConfigData road_config_data;
	if(!config_file_path.empty()) {
		std::ifstream file_input;
		file_input.exceptions(std::ios_base::badbit | std::ios_base::failbit);
		try {
			file_input.open(config_file_path);

		} catch(std::exception& e) {
			std::cout << "Error opening road config file '" << config_file_path << "': " << e.what() << std::endl;
			return 1;
		}

		try {
			Io::ReadConfigFromStream(file_input, road_config_data);

		} catch(Exception& e) {
			std::cout << "Error reading road config file '" << config_file_path << "': " << e.what() << std::endl;
			file_input.close();
			return 1;
		}

		file_input.close();
	}

	//
	// 2) Transform road config data into the struct
	//

	RoadConfig road_config;
	try {
		GetRoadConfig(road_config_data, road_config);

	} catch(Exception& e) {
		std::cout << "Error extracting road config: " << e.what() << std::endl;
		return 1;
	}

	//
	// 3) Read road input data from file
	//

	const std::string input_file_path = args.at(0);
	RoadFigureDataConst road_data;
	{
		std::ifstream file_input;
		file_input.exceptions(std::ios_base::badbit | std::ios_base::failbit);
		try {
			file_input.open(input_file_path);

		} catch(std::exception& e) {
			std::cout << "Error opening road data file '" << input_file_path << "': " << e.what() << std::endl;
			return 1;
		}

		try {
			ReadRoadInputDataFromTableConfigStream(file_input, road_data);

		} catch(Exception& e) {
			std::cout << "Error reading road data file '" << input_file_path << "': " << e.what() << std::endl;
			file_input.close();
			return 1;
		}

		file_input.close();
	}


	//
	// 4) Generate road figures, put them into Map object
	//

	Udmf::Map map;
	try {
		GetGeneratedRoad(road_data, road_config, map);

	} catch(std::exception& e) {
		std::cout << "Error generating the road: " << e.what() << std::endl;
		return 1;
	}


	//
	// 5) Open WAD output stream
	//

	const std::string output_wad_file_path = (args.size() > 1) ? args[1] : "roads.wad";
	std::ofstream file_output;
	file_output.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	try {
		file_output.open(output_wad_file_path, std::ios::binary);

	} catch(std::exception& e) {
		std::cout << "Error opening output WAD file '" << output_wad_file_path << "': " << e.what() << std::endl;
		return 1;
	}
	Io::WadWriter wad_writer(file_output);


	//
	// 6) Put TEXTMAP with generated figures into the WAD file
	//

	try {
		wad_writer.StartLumpWriting(map_name); // (empty marker lump)
		Udmf::WriteMapToStream(map, 3, wad_writer.StartLumpWriting("TEXTMAP"));
		wad_writer.StartLumpWriting("ENDMAP"); // (empty marker lump)

	} catch(std::exception& e) {
		file_output.close();
		std::cout << "Error writing TEXTMAP lump to WAD file: " << e.what() << std::endl;
		return 1;
	}

	//
	// 7) Finish writing WAD file
	//

	try {
		wad_writer.FinishWriting();

	} catch(Exception& e) {
		file_output.close();
		std::cout << "Error writing WAD file '" + output_wad_file_path + "': " << e.what() << std::endl;
		return 1;
	}

	file_output.close();
	std::cout << "Wad file created successfully!" << std::endl;
	return 0;
}

void ReadRoadInputDataFromTableConfigStream(Io::InStream& in_stream, RoadFigureDataConst& road_data) {
	class RoadFigureDataReader: public Io::TableConfigProcessor {
		RoadFigureDataConst& m_road_data;

		IRoadFigureObject* m_cur_obj;
		int m_index;
	public:
		RoadFigureDataReader(RoadFigureDataConst& road_data_)
			: m_road_data(road_data_)
			, m_cur_obj(NULL)
			, m_index(0)
		{
			m_road_data.clear();
		}

		void ProcessValue(const std::string& value) {
			if(!m_cur_obj) {
				if(m_index != 0) {
					throw Exception("internal error - cannot parse road input data");
				}

				// new line - new object
				if(m_road_data.size() >= 1024) {
					throw Exception("bad road input data - too many entries (max 1024)");
				}

				if(value == "Figure") {
					m_road_data.push_back(IRoadFigureObjectPtr(m_cur_obj = new RoadFigureStartModifier));

				} else if(value == "Line") {
					m_road_data.push_back(IRoadFigureObjectPtr(m_cur_obj = new RoadFigureLine));

				} else if(value == "Arc") {
					m_road_data.push_back(IRoadFigureObjectPtr(m_cur_obj = new RoadFigureArc));

				} else if(value == "Slope") {
					m_road_data.push_back(IRoadFigureObjectPtr(m_cur_obj = new RoadFigureSlopeModifier));

				} else {
					throw Exception("bad road input data - unknown road figure object type '" + value + "', must be Figure, Line, Arc or Slope");
				}

				m_index = 1;
				return;
			}

			switch(m_cur_obj->GetType()) {
			case IRoadFigureObject::enType_StartModifier:
				{
					// x0, y0, angle, floorpos, height, mark_shift
					IRoadFigureObject::StartModifierData& figure_start_data = *m_cur_obj->GetStartModifierData();
					switch(m_index) {
					case 1:
						if(!TryParseDouble(value, figure_start_data.x_null)) {
							throw Exception("bad road input data - bad figure x0 value '" + value + "'");
						}
						break;

					case 2:
						if(!TryParseDouble(value, figure_start_data.y_null)) {
							throw Exception("bad road input data - bad figure y0 value '" + value + "'");
						}
						break;

					case 3:
						if(!TryParseDouble(value, figure_start_data.angle)) {
							throw Exception("bad road input data - bad figure angle value '" + value + "'");
						}

						figure_start_data.angle = Math::DegreesToRadians(figure_start_data.angle);
						break;

					case 4:
						if(!TryParseInt(value, figure_start_data.zpos)) {
							throw Exception("bad road input data - bad figure z position value '" + value + "'");
						}
						break;

					case 5:
						if(!TryParseInt(value, figure_start_data.height)) {
							throw Exception("bad road input data - bad figure height value '" + value + "'");
						}
						break;

					case 6:
						{
							int mark_shift_int = 0;
							if(!TryParseInt(value, mark_shift_int)) {
								throw Exception("bad road input data - bad figure road mark shift value '" + value + "'");
							}
							figure_start_data.mark_shift = mark_shift_int;
						}
						break;

					default:
						throw Exception("bad road input data - bad figure start definition, too many fields");
					}
				}
				break;

			case IRoadFigureObject::enType_Line:
				{
					// length
					IRoadFigureObject::LineData& line_data = *m_cur_obj->GetLineData();
					switch(m_index) {
					case 1:
						if(!TryParseDouble(value, line_data.length)) {
							throw Exception("bad road input data - bad line length value '" + value + "'");
						}
						break;

					default:
						throw Exception("bad road input data - bad line definition, too many fields");
					}
				}
				break;

			case IRoadFigureObject::enType_Arc:
				{
					// curve (radius), angle, divider
					IRoadFigureObject::ArcData& arc_data = *m_cur_obj->GetArcData();
					switch(m_index) {
					case 1:
						if(!TryParseDouble(value, arc_data.curve)) {
							throw Exception("bad road input data - bad arc curve value '" + value + "'");
						}
						break;

					case 2:
						if(!TryParseDouble(value, arc_data.angle)) {
							throw Exception("bad road input data - bad arc angle value '" + value + "'");
						}

						arc_data.angle = Math::DegreesToRadians(arc_data.angle);
						break;

					case 3:
						if(!TryParseInt(value, arc_data.divider)) {
							throw Exception("bad road input data - bad arc divider value '" + value + "'");
						}
						break;

					default:
						throw Exception("bad road input data - bad arc definition, too many fields");
					}
				}
				break;

			case IRoadFigureObject::enType_SlopeModifier:
				{
					// tangent
					IRoadFigureObject::SlopeModifierData& slope_modifier_data = *m_cur_obj->GetSlopeModifierData();
					switch(m_index) {
					case 1:
						if(!TryParseDouble(value, slope_modifier_data.tangent)) {
							throw Exception("bad road input data - bad slope modifier tangent value '" + value + "'");
						}
						break;

					default:
						throw Exception("bad road input data - bad slope modifier definition, too many fields");
					}
				}
				break;

			default:
				// must not happen
				throw Exception("internal error - bad parsing of road input data, unknown road object type");
			}

			m_index++;
		}

		void EndRow() {
			if(!m_cur_obj) {
				// must not happen
				throw Exception("internal error - road input data value on the row is not found");
			}

			// compare index with expected number of fields plus 1
			switch(m_cur_obj->GetType()) {
			case IRoadFigureObject::enType_StartModifier:
				if(m_index != 7) {
					throw Exception("bad road input data - bad figure start definition, not enough fields");
				}
				break;

			case IRoadFigureObject::enType_Line:
				if(m_index != 2) {
					throw Exception("bad road input data - bad line definition, not enough fields");
				}
				break;

			case IRoadFigureObject::enType_Arc:
				if(m_index != 4) {
					throw Exception("bad road input data - bad arc definition, not enough fields");
				}
				break;

			case IRoadFigureObject::enType_SlopeModifier:
				if(m_index != 2) {
					throw Exception("bad road input data - bad slope modifier definition, not enough fields");
				}
				break;

			default:
				// must not happen
				throw Exception("internal error - bad parsing of road input data, unknown road object type at end of a row");
			}

			m_cur_obj = NULL;
			m_index = 0;
		}

	private:
		bool TryParseDouble(const std::string& str, double& value) const {
			const int i_part_max = 99999999, f_part_max = 9999999;

			bool negative = false;
			int i_part = 0, f_part = 0;
			bool reading_frac = false;
			int divider = 1;
			for(int i=0; i<str.size(); i++) {
				if(str[i] == '-') {
					if(i == 0) {
						negative = true;

					} else {
						return false;
					}

				} else if(str[i] == '.') {
					if(reading_frac) {
						return false;
					}
					reading_frac = true;

				} else if(str[i] >= '0' && str[i] <= '9') {
					int& part = reading_frac ? f_part : i_part;
					const int& part_max = reading_frac ? f_part_max : i_part_max;
					part = part * 10 + str[i] - '0';
					if(part > part_max) {
						return false;
					}

					if(reading_frac) {
						divider *= 10;
					}

				} else {
					return false;
				}
			}

			value = i_part + (double)f_part / divider;
			if(negative) {
				value = -value;
			}
			return true;
		}

		bool TryParseInt(const std::string& str, int& value) const {
			const int max = 99999999;

			bool negative = false;
			int result = 0;
			for(int i=0; i<str.size(); i++) {
				if(str[i] == '-') {
					if(i == 0) {
						negative = true;

					} else {
						return false;
					}

				} else if(str[i] >= '0' && str[i] <= '9') {
					result = result * 10 + str[i] - '0';
					if(result > max) {
						return false;
					}

				} else {
					return false;
				}
			}

			if(negative) {
				result = -result;
			}

			value = result;
			return true;
		}

	} road_data_reader(road_data);

	Io::ReadTableConfigFromStream(in_stream, &road_data_reader);
}