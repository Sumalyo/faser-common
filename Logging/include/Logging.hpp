/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// Logging.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

//============================================================================
// Name        : SimpleLogger
// Author      : Sam Meehan
// Version     : v0.0
// Copyright   : Your copyright notice
// Description : This is not really a "logger" per say but a template that allows you
//               to print descriptive debugging information to the screen using the same
//               function templates as is used in a real logger like in daqling
//============================================================================

#include <stdio.h>
#include <iostream>
#include <iomanip>

#ifdef DAQLING_LOGGING
//  #pragma message "Compiled with DAQling logger"
  #include "Utils/Ers.hpp"
  #define TRACE(MSG)    do { ERS_DEBUG(1, MSG); } while (0)
#else
#ifndef ERS_HPP
//Check if logging macros already defined.
#pragma message "Compiled without DAQling logger"

// Base log output - just printing to screen
#define LOG(LEVEL,MSG) std::cout << "[" << LEVEL <<"] " \
                                 <<"(file = "<<std::left<<__FILE__<<")" \
                                 <<"(func = "<<std::left<<__FUNCTION__<<")" \
                                 <<"(line = "<<std::left<<__LINE__<<")" \
                                 <<" | "<< MSG << std::endl; \

// Log levels
#define TRACE(MSG)    LOG("TRACE", MSG)
#define DEBUG(MSG)    LOG("DEBUG", MSG)
#define INFO(MSG)     LOG("INFO", MSG)
#define WARNING(MSG)  LOG("WARNING", MSG)
#define ERROR(MSG)    LOG("ERROR", MSG)
#define FATAL(MSG) LOG("FATAL", MSG)

// Level aliases
#define NOTICE(MSG)   LOG("NOTICE", MSG)
#define ALERT(MSG)    LOG("ALERT", MSG)
#endif
#endif
