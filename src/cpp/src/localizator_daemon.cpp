#include "json_utils.h"
#include "localizator.h"
#include <boost/program_options.hpp>
#include <stdexcept>

static bool file_exists(const std::string &filename) {
	if (FILE *file = fopen(filename.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

void remove_whitespace(std::string &s) {
	while (s.size() != 0 && std::isspace(s.front()))
		s.erase(s.begin());
	while (s.size() != 0 && std::isspace(s.back()))
		s.pop_back();
}

void split(const std::string &s, char c, std::vector<std::string> &words) {
	std::istringstream iss(s);
	for (std::string word; std::getline(iss, word, c);) {
		remove_whitespace(word);
		if (word.size() != 0)
			words.push_back(word);
	}
}

static void parse_cmd_from_file(const std::string &filename, std::vector<std::string> &argv) {
	std::stringstream buffer;
	buffer << std::ifstream(filename).rdbuf();
	std::string cmd = buffer.str();
	split(cmd, ' ', argv);
}

class LocalizatorDemon {
  private:
	std::string cmd_filename;
	std::string ack_filename;
	Localizator localizator;
	bool is_init;

  public:
	LocalizatorDemon(const std::string &cmd_filename, const std::string &ack_filename)
		: cmd_filename(cmd_filename), ack_filename(ack_filename), is_init(false) {}

	void load_scene(const std::string &scene_filename) {
		std::cerr << "# init: " << scene_filename << std::endl;
		// try {
		// 	std::vector<Point> points;
		// 	parse_json_scene(scene_filename, points);
		// 	localizator.init(points);
		// 	is_init = true;
		// } catch (const std::exception &e) {
		// 	is_init = false;
		// 	throw e;
		// }
	}

	void query(double d, const std::string &outfile) {
		std::cerr << "# query1: " << d << std::endl;
		if (!is_init)
			throw std::runtime_error("Localizator wan't initiated");
		// std::vector<std::pair<Segment, Polygon>> polygons;
		// localizator.query(d, polygons);
		// TODO
	}

	void query(double d1, double d2, const std::string &outfile) {
		std::cerr << "# query2: " << d1 << " " << d2 << std::endl;
		if (!is_init)
			throw std::runtime_error("Localizator wan't initiated");
		// TODO
	}

	void run() {
		while (true) {
			if (file_exists(cmd_filename) && !file_exists(ack_filename)) {
				std::vector<std::string> argv;
				parse_cmd_from_file(cmd_filename, argv);
				int err = exec_cmd(argv);

				std::ofstream ackfile(ack_filename);
				ackfile << err;
				ackfile.close();
			}
			usleep(100000); /* 0.1 sec */
		}
	}

	int exec_cmd(const std::vector<std::string> &argv) {
		try {
			std::string cmd;
			std::string scene_filename;
			double d;
			double d1, d2;
			std::string out_filename;

			boost::program_options::options_description desc{"Options"};
			desc.add_options()("help,h", "Help message");
			desc.add_options()("cmd", boost::program_options::value<std::string>(&cmd),
							   "Command [init, query1, query2]");
			desc.add_options()("scene", boost::program_options::value<std::string>(&scene_filename), "Scene filename");
			desc.add_options()("d", boost::program_options::value<double>(&d), "single measurement value");
			desc.add_options()("d1", boost::program_options::value<double>(&d1),
							   "first value of double measurement query");
			desc.add_options()("d2", boost::program_options::value<double>(&d2),
							   "second value of double measurement query");
			desc.add_options()("out", boost::program_options::value<std::string>(&out_filename), "Output file");

			const unsigned int MAX_ARGS_NUM = 16;
			if (argv.size() > MAX_ARGS_NUM)
				throw std::runtime_error("too many args");
			const char *argv_arr[MAX_ARGS_NUM + 1];
			argv_arr[0] = "localizator_daemon";
			for (unsigned int i = 0; i < argv.size(); i++)
				argv_arr[1 + i] = argv[i].c_str();
			int agrc = 1 + argv.size();

			const auto options = boost::program_options::parse_command_line(agrc, argv_arr, desc);
			boost::program_options::variables_map vm;
			boost::program_options::store(options, vm);
			notify(vm);

			if (vm.count("help"))
				std::cout << desc << '\n';
			else if (!vm.count("cmd"))
				std::cout << "The following flags are required: --cmd" << '\n';
			else if (cmd == "init") {
				if (!vm.count("scene"))
					std::cout << "The following flags are required: --scene" << '\n';
				else
					load_scene(scene_filename);
			} else if (cmd == "query1") {
				if (!vm.count("d") || !vm.count("out_filename"))
					std::cout << "The following flags are required: --d --out" << '\n';
				else
					query(d, out_filename);
			} else if (cmd == "query2") {
				if (!vm.count("d1") || !vm.count("d2") || !vm.count("out_filename"))
					std::cout << "The following flags are required: --d1 --d2 --out" << '\n';
				else
					query(d1, d2, out_filename);
			}
			return 0;
		} catch (const std::exception &ex) {
			std::cerr << ex.what() << '\n';
			return -1;
		}
	}
};

int main(int argc, const char *argv[]) {
	try {
		std::string cmd_filename, ack_filename;
		boost::program_options::options_description desc{"Options"};
		desc.add_options()("help,h", "Help message");
		desc.add_options()("cmdfile", boost::program_options::value<std::string>(&cmd_filename), "Commands file");
		desc.add_options()("ackfile", boost::program_options::value<std::string>(&ack_filename), "Acknowledges file");

		boost::program_options::variables_map vm;
		const auto options = boost::program_options::parse_command_line(argc, argv, desc);
		boost::program_options::store(options, vm);
		notify(vm);

		if (vm.count("help"))
			std::cout << desc << '\n';
		else if (!vm.count("cmdfile") || !vm.count("ackfile"))
			std::cout << "The following flags are required: --cmdfile --ackfile" << '\n';
		else {
			LocalizatorDemon demon(cmd_filename, ack_filename);
			demon.run();
		}
		return 0;

	} catch (const std::exception &ex) {
		std::cerr << ex.what() << '\n';
		return -1;
	}
}
