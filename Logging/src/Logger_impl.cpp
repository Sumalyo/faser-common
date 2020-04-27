/*

  implements and initializes spdlog logger when compiling standalone
  application under daqling

 */

#include "Utils/Logging.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

/*
 * Defines required logging fields.
 */
daqling::utilities::LoggerType daqling::utilities::Logger::m_logger;
daqling::utilities::LoggerType daqling::utilities::Logger::m_module_logger;
bool daqling::utilities::Logger::m_module_logger_set;
using logger = daqling::utilities::Logger;

static struct loggerInit
{
  loggerInit() { // called before main()
    //inspired from daqling main() routine, but without configurability
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto core_logger = std::make_shared<spdlog::logger>("", stdout_sink);
    logger::set_instance(core_logger);
    core_logger->set_level(spdlog::level::info);
  }
} _logger;
