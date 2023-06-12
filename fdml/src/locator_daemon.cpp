#include <chrono>
#include <stdexcept>
#include <thread>

#include <boost/program_options.hpp>

#include "fdml/internal/json_utils.hpp"
#include "fdml/internal/utils.hpp"
#include "fdml/locator_daemon.hpp"
#include "fdml/retcode.hpp"

namespace FDML {

static bool file_exists(const std::string& filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

static void remove_whitespace(std::string& s) {
    while (s.size() != 0 && std::isspace(s.front()))
        s.erase(s.begin());
    while (s.size() != 0 && std::isspace(s.back()))
        s.pop_back();
}

static void split(const std::string& s, char c, std::vector<std::string>& words) {
    std::istringstream iss(s);
    for (std::string word; std::getline(iss, word, c);) {
        remove_whitespace(word);
        if (word.size() != 0)
            words.push_back(word);
    }
}

static void parse_cmd_from_file(const std::string& filename, std::vector<std::string>& argv) {
    std::stringstream buffer;
    buffer << std::ifstream(filename).rdbuf();
    std::string cmd = buffer.str();
    split(cmd, ' ', argv);
}

LocatorDaemon::LocatorDaemon(const std::string& cmd_filename, const std::string& ack_filename)
    : cmd_filename(cmd_filename), ack_filename(ack_filename), locator(nullptr) {}

void LocatorDaemon::load_scene(const std::string& scene_filename) {
    fdml_infoln("[LocatorDaemon] Init with scene: " << scene_filename);

    try {
        if (locator)
            locator.reset();

        Polygon_with_holes scene = JsonUtils::read_scene(scene_filename);
        locator = std::make_unique<Locator>();
        locator->init(scene);

    } catch (const std::exception& e) {
        if (locator)
            locator.reset();
        throw e;
    }
}

void LocatorDaemon::query(double d, const std::string& outfile) const {
    fdml_infoln("[LocatorDaemon] Query1: " << d);
    check_state();

    std::vector<Polygon> polygons;
    for (const auto& res : locator->query(d))
        polygons.push_back(std::move(res.pos));
    JsonUtils::write_polygons(polygons, outfile);
}

void LocatorDaemon::query(double d1, double d2, const std::string& outfile) const {
    fdml_infoln("[LocatorDaemon] Query2: " << d1 << " " << d2);
    check_state();

    std::vector<Segment> segments;
    for (const auto& res : locator->query(d1, d2))
        segments.insert(segments.end(), res.pos.begin(), res.pos.end());
    JsonUtils::write_segments(segments, outfile);
}

void LocatorDaemon::query(double d1, double d2, double d3, double d4, const std::string& outfile) const {
    fdml_infoln("[LocatorDaemon] Query2 Double: " << d1 << " " << d2 << " " << d3 << " " << d4);
    check_state();

    std::vector<Segment> segments1;
    std::vector<Segment> segments2;
    for (const auto& res : locator->query(d1, d2))
        segments1.insert(segments1.end(), res.pos.begin(), res.pos.end());
    for (const auto& res : locator->query(d3, d4))
        segments2.insert(segments2.end(), res.pos.begin(), res.pos.end());

    std::vector<Point> points;
    for (const auto& seg1 : segments1) {
        for (const auto& seg2 : segments2) {
            CGAL::Object res = CGAL::intersection(seg1, seg2);
            Point point;
            if (CGAL::assign(point, res))
                points.push_back(point);
        }
    }

    JsonUtils::write_points(points, outfile);
}

void LocatorDaemon::run() {
    fdml_infoln("[LocatorDaemon] Daemon is running. Waiting for next command...");
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
            fdml_infoln("[LocatorDaemon] Command proccessed. Waiting for next command...");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); /* 0.1 sec */
        fdml_debug(".");
    }
    fdml_infoln("[LocatorDaemon] Quit.");
}

int LocatorDaemon::exec_cmd(const std::vector<std::string>& argv, bool& quit) {
    fdml_debug("[LocatorDaemon] executing command:");
    for (const auto& arg : argv)
        fdml_debug(' ' << arg);
    fdml_debugln("");

    try {
        const unsigned int MAX_ARGS_NUM = 16;
        if (argv.size() > MAX_ARGS_NUM) {
            fdml_errln("Too many arguments");
            return FDML_RETCODE_TOO_MANY_ARGS;
        }
        const char* argv_arr[MAX_ARGS_NUM + 1];
        argv_arr[0] = "locator_daemon";
        for (unsigned int i = 0; i < argv.size(); i++)
            argv_arr[1 + i] = argv[i].c_str();
        int argc = 1 + argv.size();

        std::string cmd;
        std::string scene_filename;
        double d;
        double d1, d2;
        double d3, d4;
        std::string out_filename;

        boost::program_options::options_description desc{"Options"};
        desc.add_options()("help", "Help message");
        desc.add_options()("cmd", boost::program_options::value<std::string>(&cmd),
                           "Command [init, query1, query2, query2_double, quit]");
        desc.add_options()("scene", boost::program_options::value<std::string>(&scene_filename), "Scene filename");
        desc.add_options()("d", boost::program_options::value<double>(&d), "single measurement value");
        desc.add_options()("d1", boost::program_options::value<double>(&d1), "first value of double measurement query");
        desc.add_options()("d2", boost::program_options::value<double>(&d2),
                           "second value of double measurement query");
        desc.add_options()("d3", boost::program_options::value<double>(&d3),
                           "first value of a second double measurement query");
        desc.add_options()("d4", boost::program_options::value<double>(&d4),
                           "second value of a second double measurement query");
        desc.add_options()("out", boost::program_options::value<std::string>(&out_filename), "Output file");

        const auto options = boost::program_options::parse_command_line(argc, argv_arr, desc);
        boost::program_options::variables_map vm;
        boost::program_options::store(options, vm);
        notify(vm);

        if (vm.count("help"))
            fdml_info(desc);
        else if (!vm.count("cmd")) {
            fdml_errln("The following flags are required: --cmd");
            return FDML_RETCODE_MISSING_ARGS;
        } else if (cmd == std::string("init")) {
            if (!vm.count("scene")) {
                fdml_errln("The following flags are required: --scene");
                return FDML_RETCODE_MISSING_ARGS;
            } else
                load_scene(scene_filename);
        } else if (cmd == std::string("query1")) {
            if (!vm.count("d") || !vm.count("out")) {
                fdml_errln("The following flags are required: --d --out");
                return FDML_RETCODE_MISSING_ARGS;
            } else
                query(d, out_filename);
        } else if (cmd == std::string("query2")) {
            if (!vm.count("d1") || !vm.count("d2") || !vm.count("out")) {
                fdml_errln("The following flags are required: --d1 --d2 --out");
                return FDML_RETCODE_MISSING_ARGS;
            } else
                query(d1, d2, out_filename);
        } else if (cmd == std::string("query2_double")) {
            if (!vm.count("d1") || !vm.count("d2") || !vm.count("d3") || !vm.count("d4") || !vm.count("out")) {
                fdml_errln("The following flags are required: --d1 --d2 --d3 --d4 --out");
                return FDML_RETCODE_MISSING_ARGS;
            } else
                query(d1, d2, d3, d4, out_filename);
        } else if (cmd == std::string("quit"))
            quit = true;
        else {
            fdml_info("Unknown command: " << cmd);
            fdml_info(desc);
            return FDML_RETCODE_UNKNOWN_ARGS;
        }
        return FDML_RETCODE_OK;
    } catch (const std::exception& ex) {
        fdml_errln(ex.what());
        return FDML_RETCODE_RUNTIME_ERR;
    }
}

void LocatorDaemon::check_state() const {
    if (!locator)
        throw std::runtime_error("Locator wasn't initialized");
}

int LocatorDaemon::daemon_main(int argc, const char* argv[]) {
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
            return FDML_RETCODE_MISSING_ARGS;
        } else {
            FDML::LocatorDaemon daemon(cmd_filename, ack_filename);
            daemon.run();
        }
        return FDML_RETCODE_OK;

    } catch (const std::exception& ex) {
        fdml_errln(ex.what());
        return FDML_RETCODE_RUNTIME_ERR;
    }
}

} // namespace FDML
