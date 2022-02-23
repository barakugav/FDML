#include "localizator.h"

#ifndef __LOCALIZATOR_DAEMON_H__
#define __LOCALIZATOR_DAEMON_H__

class LocalizatorDaemon {
  private:
	std::string cmd_filename;
	std::string ack_filename;
	std::unique_ptr<Localizator> localizator;

  public:
	LocalizatorDaemon(const std::string &cmd_filename, const std::string &ack_filename);

	void load_scene(const std::string &scene_filename);
	void query(double d, const std::string &outfile);
	void query(double d1, double d2, const std::string &outfile);

	void run();
	int exec_cmd(const std::vector<std::string> &argv);

  private:
	void check_state();
};

#endif