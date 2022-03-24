#include "fdml/localizator_daemon.h"

int main(int argc, const char *argv[]) {
	return FDML::LocalizatorDaemon::daemon_main(argc, argv);
}
