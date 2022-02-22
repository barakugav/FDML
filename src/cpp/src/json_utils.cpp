#include "json_utils.h"
#include "boost/json.hpp"

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

static boost::json::value parse_file(const std::string &filename) {
	char filename_arr[filename.size() + 1];
	strcpy(filename_arr, filename.c_str());

	file f(filename_arr, "r");
	boost::json::stream_parser p;
	boost::json::error_code ec;
	do {
		char buf[4096];
		auto const nread = f.read(buf, sizeof(buf));
		p.write(buf, nread, ec);
	} while (!f.eof());
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
		oss << __VA_ARGS__;                                                                                            \
		throw std::runtime_error(oss.str());                                                                           \
	} while (0)

static const boost::json::object &get_object(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::object)
		ERR("Expected field \"" << obj_name << "\" to be Json object (" << boost::json::kind::object << "), actual is "
								<< v.kind() << std::endl);
	return v.get_object();
}

static const boost::json::array &get_array(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::array)
		ERR("Expected field \"" << obj_name << "\" to be Json array (" << boost::json::kind::array << "), actual is "
								<< v.kind() << std::endl);
	return v.get_array();
}

static double get_double(const boost::json::value &v, const char *obj_name) {
	if (v.kind() != boost::json::kind::double_)
		ERR("Expected field \"" << obj_name << "\" to be Json double (" << boost::json::kind::double_ << "), actual is "
								<< v.kind() << std::endl);
	return v.get_double();
}

template <typename JsonObj> static void validate_size(const JsonObj &obj, unsigned int size, const char *obj_name) {
	if (obj.size() != size)
		ERR("Unexpected number of elements in \"" << obj_name << "\" Json object: " << obj.size() << " != " << size
												  << std::endl);
}

void parse_json_scene(const std::string &filename, std::vector<Point> &points) {
	const auto j = parse_file(filename);

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
				points.push_back(Point(x, y));
			}
		} else
			ERR("Unknown Json tag: " << obstacles_key << std::endl);
	}
}