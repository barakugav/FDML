#include "localizator_daemon.h"
#include "json_utils.h"
#include "utils.hpp"
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

static void remove_whitespace(std::string &s) {
	while (s.size() != 0 && std::isspace(s.front()))
		s.erase(s.begin());
	while (s.size() != 0 && std::isspace(s.back()))
		s.pop_back();
}

static void split(const std::string &s, char c, std::vector<std::string> &words) {
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

LocalizatorDaemon::LocalizatorDaemon(const std::string &cmd_filename, const std::string &ack_filename)
	: cmd_filename(cmd_filename), ack_filename(ack_filename), localizator(nullptr) {}

void LocalizatorDaemon::load_scene(const std::string &scene_filename) {
	std::cout << "# init: " << scene_filename << std::endl;

	try {
		if (localizator)
			localizator.reset();

		std::vector<Point> points;
		parse_scene_from_json(scene_filename, points);
		localizator = std::make_unique<Localizator>();
		localizator->init(points);

	} catch (const std::exception &e) {
		if (localizator)
			localizator.reset();
		std::cerr << e.what() << std::endl;
		throw e;
	}
}

void LocalizatorDaemon::query(double d, const std::string &outfile) {
	std::cout << "# query1: " << d << std::endl;
	check_state();

	std::vector<Polygon> polygons;
	localizator->query(d, polygons);
	write_polygons_to_json(polygons, outfile);
}

void LocalizatorDaemon::query(double d1, double d2, const std::string &outfile) {
	std::cout << "# query2: " << d1 << " " << d2 << std::endl;
	check_state();
	throw std::runtime_error("not supported"); // TODO
}

void LocalizatorDaemon::run() {
	while (true) {
		if (!file_exists(ack_filename) && file_exists(cmd_filename)) {
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

int LocalizatorDaemon::exec_cmd(const std::vector<std::string> &argv) {
	try {
		std::string cmd;
		std::string scene_filename;
		double d;
		double d1, d2;
		std::string out_filename;

		boost::program_options::options_description desc{"Options"};
		desc.add_options()("help,h", "Help message");
		desc.add_options()("cmd", boost::program_options::value<std::string>(&cmd), "Command [init, query1, query2]");
		desc.add_options()("scene", boost::program_options::value<std::string>(&scene_filename), "Scene filename");
		desc.add_options()("d", boost::program_options::value<double>(&d), "single measurement value");
		desc.add_options()("d1", boost::program_options::value<double>(&d1), "first value of double measurement query");
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
		int argc = 1 + argv.size();

		const auto options = boost::program_options::parse_command_line(argc, argv_arr, desc);
		boost::program_options::variables_map vm;
		boost::program_options::store(options, vm);
		notify(vm);

		if (vm.count("help"))
			std::cout << desc << '\n';
		else if (!vm.count("cmd")) {
			std::cerr << "The following flags are required: --cmd" << '\n';
			return -1;
		} else if (cmd == "init") {
			if (!vm.count("scene")) {
				std::cerr << "The following flags are required: --scene" << '\n';
				return -1;
			} else
				load_scene(scene_filename);
		} else if (cmd == "query1") {
			if (!vm.count("d") || !vm.count("out")) {
				std::cerr << "The following flags are required: --d --out" << '\n';
				return -1;
			} else
				query(d, out_filename);
		} else if (cmd == "query2") {
			if (!vm.count("d1") || !vm.count("d2") || !vm.count("out")) {
				std::cerr << "The following flags are required: --d1 --d2 --out" << '\n';
				return -1;
			} else {
				// query(d1, d2, out_filename);
				std::cerr << "query with two measurements is not supported yet" << '\n';
				return -3;
			}
		}
		return 0;
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << '\n';
		return -2;
	}
}

void LocalizatorDaemon::check_state() {
	if (!localizator)
		throw std::runtime_error("Localizator wan't initiated");
}

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
			LocalizatorDaemon daemon(cmd_filename, ack_filename);
			daemon.run();
		}
		return 0;

	} catch (const std::exception &ex) {
		std::cerr << ex.what() << '\n';
		return -1;
	}
}
