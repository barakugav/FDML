#include <boost/program_options.hpp>

#include "fdml/internal/json_utils.hpp"
#include "fdml/internal/utils.hpp"
#include "fdml/locator.hpp"
#include "fdml/retcode.hpp"

namespace FDML {

int fdml_main(int argc, const char* argv[]) {
  try {
    std::string scenefile, cmd;
    std::string resfile;
    double d, d1, d2;
    boost::program_options::options_description desc{"Options"};
    desc.add_options()("help,h", "Help message");
    desc.add_options()("scenefile", boost::program_options::value<std::string>(&scenefile), "Scene file");
    desc.add_options()("cmd", boost::program_options::value<std::string>(&cmd), "Command [query1, query2]");
    desc.add_options()("d", boost::program_options::value<double>(&d), "single measurement value");
    desc.add_options()("d1", boost::program_options::value<double>(&d1), "first value of double measurement query");
    desc.add_options()("d2", boost::program_options::value<double>(&d2), "second value of double measurement query");
    desc.add_options()("out", boost::program_options::value<std::string>(&resfile), "Output file for results");

    boost::program_options::variables_map vm;
    const auto options = boost::program_options::parse_command_line(argc, argv, desc);
    boost::program_options::store(options, vm);
    notify(vm);

    enum command_type_t {
      CMD_QUERY1,
      CMD_QUERY2,
    } command_type;

    if (vm.count("help"))
      fdml_info(desc);
    else if (!vm.count("scenefile") || !vm.count("cmd") || !vm.count("out")) {
      fdml_errln("The following flags are required: --scenefile --cmd --out");
      return FDML_RETCODE_MISSING_ARGS;
    } else if (cmd == std::string("query1")) {
      if (!vm.count("d")) {
        fdml_errln("The following flags are required: --d");
        return FDML_RETCODE_MISSING_ARGS;
      } else {
        command_type = CMD_QUERY1;
      }
    } else if (cmd == std::string("query2")) {
      if (!vm.count("d1") || !vm.count("d2")) {
        fdml_errln("The following flags are required: --d1 --d2");
        return FDML_RETCODE_MISSING_ARGS;
      } else {
        command_type = CMD_QUERY2;
      }
    } else {
      fdml_infoln("Unknown command: " << cmd);
      fdml_infoln(desc);
      return FDML_RETCODE_UNKNOWN_ARGS;
    }

    Polygon scene = JsonUtils::read_scene(scenefile);

    Locator locator;
    locator.init(scene);

    std::vector<Polygon> polygons;
    std::vector<Segment> segments;

    switch (command_type) {
    case CMD_QUERY1:
      locator.query(d, polygons);
      JsonUtils::write_polygons(polygons, resfile);
      break;
    case CMD_QUERY2:
      locator.query(d1, d2, segments);
      JsonUtils::write_segments(segments, resfile);
      break;
    default:
      fdml_errln("Unknown command_type: " << command_type);
      return FDML_RETCODE_INTERNAL_ERR;
    }

    return FDML_RETCODE_OK;
  } catch (const std::exception& ex) {
    fdml_errln(ex.what());
    return FDML_RETCODE_RUNTIME_ERR;
  }
}

} // namespace FDML

int main(int argc, const char* argv[]) {
  FDML::fdml_main(argc, argv);
}
