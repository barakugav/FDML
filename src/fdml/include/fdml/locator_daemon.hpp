#ifndef FDML_LOCATOR_DAEMON_HPP
#define FDML_LOCATOR_DAEMON_HPP

#include "fdml/config.hpp"
#include "fdml/locator.hpp"

namespace FDML {

/**
 * @brief Wrapper daemon for the Locator class. Provide files communication with another proccess.
 */
class FDML_FDML_DECL LocatorDaemon {
  private:
    /* file used to pass commands to the daemon */
    std::string cmd_filename;
    /* file used to acknowledge the user of the daemon when a command is finished */
    std::string ack_filename;
    /* The underlying Locator */
    std::unique_ptr<Locator> locator;

  public:
    LocatorDaemon(const std::string& cmd_filename, const std::string& ack_filename);

    /**
     * @brief Load a scene from a json file
     *
     * @param scene_filename path to a json file containing a scene
     */
    void load_scene(const std::string& scene_filename);

    /**
     * @brief Query command of one measurament
     *
     * This function should be called after the load_scene function has been called
     *
     * @param d the single measurament value
     * @param outfile path to an output file for the query result (json)
     */
    void query(double d, const std::string& outfile) const;

    /**
     * @brief Query command of two measurament
     *
     * This function should be called after the load_scene function has been called
     *
     * @param d1 the first measurament value
     * @param d2 the second measurament value
     * @param outfile path to an output file for the query result (json)
     */
    void query(double d1, double d2, const std::string& outfile) const;

    /**
     * @brief Run a infinity loop, reading commands from the given cmd_filename
     *
     * This function return only if the quit command was issued or an error occurred.
     */
    void run();

    /**
     * @brief Execute a single command
     *
     * @param argv args of the command
     * @param quit will be set to true if the quit command was issued
     * @return on success, else on error
     */
    int exec_cmd(const std::vector<std::string>& argv, bool& quit);

    /**
     * @brief main function for daemon
     *
     * this function return only if the quit command was issued or some error occurred.
     *
     * @param argc number of arguments
     * @param argv arguments
     * @return 0 on success, else on error
     */
    static int daemon_main(int argc, const char* argv[]);

  private:
    /* Checks that a scene was actually loaded before handling a query command */
    void check_state() const;
};

} // namespace FDML

#endif
