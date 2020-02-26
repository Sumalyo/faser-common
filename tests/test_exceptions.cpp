#include "Exceptions/Exceptions.hpp"
#include "Logging.hpp"

int main(int /*argc*/, char **/*argv*/) {
  try {
    THROW(Exceptions::BaseException,"Example Exception");
  } catch(const Exceptions::BaseException &e)  {
    INFO(e.what());
  }
}
