#include "fdml/localizator_daemon.h"

int main(int argc, const char *argv[]) {
	if (false) {
		FDML::LocalizatorDaemon daemon(".cmdfile", ".ackfile");
		daemon.load_scene("scenes/scene02.json");
		daemon.query(1, 1, "res.json");
	} else {
		FDML::LocalizatorDaemon::daemon_main(argc, argv);
	}
	std::cout << "done successfully" << std::endl;
	return 0;
}
