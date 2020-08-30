/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// Exceptions.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Exceptions {

  class BaseException : public std::runtime_error {
    std::string _arg;
    std::string _file;
    int _line;
    std::string _msg;
  public:
    BaseException(const std::string &arg, const char *file, int line) :
      std::runtime_error(arg), _arg(arg), _file(file),_line(line) {}
    ~BaseException() throw() {}
    const char *what() const throw() {
      std::ostringstream o;
      o << "Exception thrown: "<<_file << ":" << _line << ": " << _arg;
      const_cast<BaseException*>(this)->_msg=o.str();
      return _msg.c_str();
    }
  };
}

#define THROW(exception,arg) throw exception(arg, __FILE__, __LINE__);
