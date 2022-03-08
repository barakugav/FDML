#include "localizator_daemon.h"

#define DEBUG_MAIN false

int main(int argc, const char *argv[]) {
	if (DEBUG_MAIN) {
		FDML::LocalizatorDaemon daemon(".cmdfile", ".ackfile");
		daemon.load_scene("scenes/scene07.json");
		daemon.query(0.3, "res.json");
	} else {
		FDML::LocalizatorDaemon::daemon_main(argc, argv);
	}
	std::cout << "done successfully" << std::endl;
	return 0;
}
