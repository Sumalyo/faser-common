/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// Exceptions.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once

#ifdef DAQLING_BUILD
//#pragma message "Compiled with ERS"
#include <string>
#include <ers/ers.h>
ERS_DECLARE_ISSUE(Exceptions,                                                             // Namespace
                  BaseException,                                                   // Class name
                  message, // Message
                  ((std::string)message))                      // Args

#define THROW(exception,arg) throw exception(ERS_HERE,arg);
#define CREATE_EXCEPTION_TYPE(exceptionName,nameSpace) ERS_DECLARE_ISSUE_BASE(nameSpace, \
      exceptionName, \
      Exceptions::BaseException, \
      ERS_EMPTY, \
      ((std::string)message), \
      ERS_EMPTY)

#else
#pragma message "Compiled without ERS"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#define CREATE_EXCEPTION_TYPE(exceptionName,nameSpace) namespace nameSpace {class exceptionName : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };}
#define THROW(exception,arg) throw exception(arg, __FILE__, __LINE__);


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
#endif
