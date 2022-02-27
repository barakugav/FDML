#include "json_utils.h"
#include "boost/json.hpp"
#include "utils.hpp"

class file {
	FILE *f_ = nullptr;
	long size_ = 0;

	void fail(boost::json::error_code &ec) { ec.assign(errno, boost::json::generic_category()); }

  public:
	~file() {
		if (f_)
			std::fclose(f_);
	}

	file() = default;

	file(file &&other) noexcept : f_(other.f_) { other.f_ = nullptr; }

	file(char const *path, char const *mode) { open(path, mode); }

	file &operator=(file &&other) noexcept {
		close();
		f_ = other.f_;
		other.f_ = nullptr;
		return *this;
	}

	void close() {
		if (f_) {
			std::fclose(f_);
			f_ = nullptr;
			size_ = 0;
		}
	}

	void open(char const *path, char const *mode, boost::json::error_code &ec) {
		close();
		f_ = std::fopen(path, mode);
		if (!f_)
			return fail(ec);
		if (std::fseek(f_, 0, SEEK_END) != 0)
			return fail(ec);
		size_ = std::ftell(f_);
		if (size_ == -1) {
			size_ = 0;
			return fail(ec);
		}
		if (std::fseek(f_, 0, SEEK_SET) != 0)
			return fail(ec);
	}

	void open(char const *path, char const *mode) {
		boost::json::error_code ec;
		open(path, mode, ec);
		if (ec)
			throw boost::json::system_error(ec);
	}

	long size() const noexcept { return size_; }

	bool eof() const noexcept { return std::feof(f_) != 0; }

	std::size_t read(char *data, std::size_t size, boost::json::error_code &ec) {
		auto const nread = std::fread(data, 1, size, f_);
		if (std::ferror(f_))
			ec.assign(errno, boost::json::generic_category());
		return nread;
	}

	std::size_t read(char *data, std::size_t size) {
		boost::json::error_code ec;
		auto const nread = read(data, size, ec);
		if (ec)
			throw boost::json::system_error(ec);
		return nread;
	}
};

inline std::string read_file(char const *path, boost::json::error_code &ec) {
	file f;
	f.open(path, "r", ec);
	if (ec)
		return {};
	std::string s;
	s.resize(f.size());
	s.resize(f.read(&s[0], s.size(), ec));
	if (ec)
		return {};
	return s;
}

inline std::string read_file(char const *path) {
	boost::json::error_code ec;
	auto s = read_file(path, ec);
	if (ec)
		throw boost::json::system_error(ec);
	return s;
}

static boost::json::value parse_file(std::string filename) {
	const unsigned int filename_maxsize = 256;
	if (filename.size() >= filename_maxsize)
		throw std::runtime_error("failed to parse json file. filename is too big");

	char filename_arr[filename_maxsize];
	strcpy(filename_arr, filename.c_str());

	file f(filename_arr, "r");
	boost::json::stream_parser p;
	boost::json::error_code ec;
	do {
		char buf[4096];
		auto const nread = f.read(buf, sizeof(buf));
		p.write(buf, nread, ec);
	} while (!f.eof());
	f.close();

	if (ec)
		throw std::runtime_error("failed to parse json file");
	p.finish(ec);
	if (ec)
		throw std::runtime_error("failed to parse json file");
	return p.release();
}

#define ERR(...)                                                                                                       \
	do {                                                                                                               \
		std::ostringstream oss;                                                                                        \
		oss << __VA_ARGS__ << std::endl;                                                                               \
		throw std::runtime_error(oss.str());                                                                           \
	} while (0)

static const boost::json::object &get_object(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::object)
		ERR("Expected field \"" << obj_name << "\" to be Json object (" << boost::json::kind::object << "), actual is "
								<< v.kind());
	return v.get_object();
}

static const boost::json::array &get_array(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::array)
		ERR("Expected field \"" << obj_name << "\" to be Json array (" << boost::json::kind::array << "), actual is "
								<< v.kind());
	return v.get_array();
}

static double get_double(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::double_)
		ERR("Expected field \"" << obj_name << "\" to be Json double (" << boost::json::kind::double_ << "), actual is "
								<< v.kind());
	return v.get_double();
}

template <typename JsonObj> static void validate_size(const JsonObj &obj, unsigned int size, const char *obj_name) {
	if (obj.size() != size)
		ERR("Unexpected number of elements in \"" << obj_name << "\" Json object: " << obj.size() << " != " << size);
}

void parse_scene_from_json(const std::string &filename, Polygon &scene) {
	debugln("[JsonUtils] parsing scene from file: " << filename);
	const auto j = parse_file(filename);

	scene = Polygon();

	const auto &obj = get_object(j, "top_level");
	validate_size(obj, 1, "top_level");
	for (auto obj_it = obj.begin(); obj_it != obj.end(); ++obj_it) {
		const auto &obstacles_key = obj_it->key().to_string();
		if (std::string(obstacles_key) == "obstacles") {
			const auto &obstacles = get_array(obj_it->value(), "obstacles");
			validate_size(obstacles, 1, "obstacles");

			const auto &obstacle = get_array(obstacles[0], "obstacle");
			for (auto obs_it = obstacle.begin(); obs_it != obstacle.end(); ++obs_it) {
				const auto &point = get_array(*obs_it, "point");
				validate_size(point, 2, "point");

				double x = get_double(point[0], "x");
				double y = get_double(point[1], "y");
				scene.push_back(Point(x, y));
			}
		} else
			ERR("Unknown Json tag: " << obstacles_key);
	}
}

template <typename Out> void json_format_pretty(Out &os, boost::json::value const &jv, std::string *indent = nullptr) {
	std::string indent_;
	if (!indent)
		indent = &indent_;
	switch (jv.kind()) {
	case boost::json::kind::object: {
		os << "{\n";
		indent->append(4, ' ');
		auto const &obj = jv.get_object();
		if (!obj.empty()) {
			auto it = obj.begin();
			for (;;) {
				os << *indent << boost::json::serialize(it->key()) << " : ";
				json_format_pretty(os, it->value(), indent);
				if (++it == obj.end())
					break;
				os << ",\n";
			}
		}
		os << "\n";
		indent->resize(indent->size() - 4);
		os << *indent << "}";
		break;
	}

	case boost::json::kind::array: {
		os << "[\n";
		indent->append(4, ' ');
		auto const &arr = jv.get_array();
		if (!arr.empty()) {
			auto it = arr.begin();
			for (;;) {
				os << *indent;
				json_format_pretty(os, *it, indent);
				if (++it == arr.end())
					break;
				os << ",\n";
			}
		}
		os << "\n";
		indent->resize(indent->size() - 4);
		os << *indent << "]";
		break;
	}

	case boost::json::kind::string: {
		os << boost::json::serialize(jv.get_string());
		break;
	}

	case boost::json::kind::uint64:
		os << jv.get_uint64();
		break;

	case boost::json::kind::int64:
		os << jv.get_int64();
		break;

	case boost::json::kind::double_:
		os << jv.get_double();
		break;

	case boost::json::kind::bool_:
		if (jv.get_bool())
			os << "true";
		else
			os << "false";
		break;

	case boost::json::kind::null:
		os << "null";
		break;
	}

	if (indent->empty())
		os << "\n";
}

void write_polygons_to_json(const std::vector<Polygon> &polygons, const std::string &filename) {
	debugln("[JsonUtils] writing polygons into: " << filename);
	std::vector<boost::json::array> polygon_objs;
	for (const Polygon &polygon : polygons) {
		std::vector<boost::json::array> point_objs;
		for (auto it = polygon.vertices_begin(); it != polygon.vertices_end(); ++it) {
			double x = it->hx().exact().convert_to<double>();
			double y = it->hy().exact().convert_to<double>();
			std::vector<double> point{x, y};
			point_objs.push_back(boost::json::array(point.begin(), point.end()));
		}
		boost::json::array polygon_obj(point_objs.begin(), point_objs.end());
		polygon_objs.push_back(polygon_obj);
	}
	auto polygons_obj = std::make_pair("polygons", boost::json::array(polygon_objs.begin(), polygon_objs.end()));
	std::vector<std::pair<std::string, boost::json::value>> top_lvl_fields;
	top_lvl_fields.push_back(polygons_obj);
	boost::json::object top_lvl_obj(top_lvl_fields.begin(), top_lvl_fields.end());
	boost::json::value top_lvl_obj2(top_lvl_obj);

	std::ofstream outfile(filename);
	json_format_pretty(outfile, top_lvl_obj2);
	outfile.close();
}
