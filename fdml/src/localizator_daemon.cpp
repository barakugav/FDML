#include "localizator_daemon.h"
#include "json_utils.h"
#include "utils.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <stdexcept>
#include <thread>

namespace FDML {

enum localizator_daemon_retcode {
	LOCALIZATOR_DAEMON_RETCODE_OK = 0,
	LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS,
	LOCALIZATOR_DAEMON_RETCODE_UNKNOWN_ARGS,
	LOCALIZATOR_DAEMON_RETCODE_RUNTIME_ERR,
	LOCALIZATOR_DAEMON_RETCODE_TOO_MANY_ARGS,
};

static bool file_exists(const std::string &filename) {
	std::ifstream f(filename.c_str());
	return f.good();
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
	fdml_infoln("[LocalizatorDaemon] Init with scene: " << scene_filename);

	try {
		if (localizator)
			localizator.reset();

		Polygon scene;
		parse_scene_from_json(scene_filename, scene);
		localizator = std::make_unique<Localizator>();
		localizator->init(scene);

	} catch (const std::exception &e) {
		if (localizator)
			localizator.reset();
		throw e;
	}
}

void LocalizatorDaemon::query(double d, const std::string &outfile) const {
	fdml_infoln("[LocalizatorDaemon] Query1: " << d);
	check_state();

	std::vector<Polygon> polygons;
	localizator->query(d, polygons);
	write_polygons_to_json(polygons, outfile);
}

void LocalizatorDaemon::query(double d1, double d2, const std::string &outfile) const {
	fdml_infoln("[LocalizatorDaemon] Query2: " << d1 << " " << d2);
	check_state();

	std::vector<Segment> segments;
	localizator->query(d1, d2, segments);
	write_segments_to_json(segments, outfile);
}

void LocalizatorDaemon::run() {
	fdml_infoln("[LocalizatorDaemon] Daemon is running. Waiting for next command...");
	while (true) {
		if (!file_exists(ack_filename) && file_exists(cmd_filename)) {
			std::vector<std::string> argv;
			parse_cmd_from_file(cmd_filename, argv);

			bool quit = false;
			int err = exec_cmd(argv, quit);

			std::ofstream ackfile(ack_filename);
			ackfile << err;
			ackfile.close();
			if (quit)
				break;
			fdml_infoln("[LocalizatorDaemon] Command proccessed. Waiting for next command...");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); /* 0.1 sec */
		fdml_debug(".");
	}
	fdml_infoln("[LocalizatorDaemon] Quit.");
}

int LocalizatorDaemon::exec_cmd(const std::vector<std::string> &argv, bool &quit) {
	fdml_debug("[LocalizatorDaemon] executing command:");
	for (const auto &arg : argv)
		fdml_debug(' ' << arg);
	fdml_debugln("");

	try {
		const unsigned int MAX_ARGS_NUM = 16;
		if (argv.size() > MAX_ARGS_NUM) {
			fdml_errln("Too many arguments");
			return LOCALIZATOR_DAEMON_RETCODE_TOO_MANY_ARGS;
		}
		const char *argv_arr[MAX_ARGS_NUM + 1];
		argv_arr[0] = "localizator_daemon";
		for (unsigned int i = 0; i < argv.size(); i++)
			argv_arr[1 + i] = argv[i].c_str();
		int argc = 1 + argv.size();

		std::string cmd;
		std::string scene_filename;
		double d;
		double d1, d2;
		std::string out_filename;

		boost::program_options::options_description desc{"Options"};
		desc.add_options()("help", "Help message");
		desc.add_options()("cmd", boost::program_options::value<std::string>(&cmd),
						   "Command [init, query1, query2, quit]");
		desc.add_options()("scene", boost::program_options::value<std::string>(&scene_filename), "Scene filename");
		desc.add_options()("d", boost::program_options::value<double>(&d), "single measurement value");
		desc.add_options()("d1", boost::program_options::value<double>(&d1), "first value of double measurement query");
		desc.add_options()("d2", boost::program_options::value<double>(&d2),
						   "second value of double measurement query");
		desc.add_options()("out", boost::program_options::value<std::string>(&out_filename), "Output file");

		const auto options = boost::program_options::parse_command_line(argc, argv_arr, desc);
		boost::program_options::variables_map vm;
		boost::program_options::store(options, vm);
		notify(vm);

		if (vm.count("help"))
			fdml_info(desc);
		else if (!vm.count("cmd")) {
			fdml_errln("The following flags are required: --cmd");
			return LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS;
		} else if (cmd == "init") {
			if (!vm.count("scene")) {
				fdml_errln("The following flags are required: --scene");
				return LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS;
			} else
				load_scene(scene_filename);
		} else if (cmd == "query1") {
			if (!vm.count("d") || !vm.count("out")) {
				fdml_errln("The following flags are required: --d --out");
				return LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS;
			} else
				query(d, out_filename);
		} else if (cmd == "query2") {
			if (!vm.count("d1") || !vm.count("d2") || !vm.count("out")) {
				fdml_errln("The following flags are required: --d1 --d2 --out");
				return LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS;
			} else
				query(d1, d2, out_filename);
		} else if (cmd == "quit")
			quit = true;
		else {
			fdml_info("Unknown command: " << cmd);
			fdml_info(desc);
			return LOCALIZATOR_DAEMON_RETCODE_UNKNOWN_ARGS;
		}
		return LOCALIZATOR_DAEMON_RETCODE_OK;
	} catch (const std::exception &ex) {
		fdml_errln(ex.what());
		return LOCALIZATOR_DAEMON_RETCODE_RUNTIME_ERR;
	}
}

void LocalizatorDaemon::check_state() const {
	if (!localizator)
		throw std::runtime_error("Localizator wan't initialized");
}

int LocalizatorDaemon::daemon_main(int argc, const char *argv[]) {
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
			fdml_info(desc);
		else if (!vm.count("cmdfile") || !vm.count("ackfile")) {
			fdml_errln("The following flags are required: --cmdfile --ackfile");
			return LOCALIZATOR_DAEMON_RETCODE_MISSING_ARGS;
		} else {
			FDML::LocalizatorDaemon daemon(cmd_filename, ack_filename);
			daemon.run();
		}
		return LOCALIZATOR_DAEMON_RETCODE_OK;

	} catch (const std::exception &ex) {
		fdml_errln(ex.what());
		return LOCALIZATOR_DAEMON_RETCODE_RUNTIME_ERR;
	}
}

} // namespace FDML
