#include "dbn_cpp/py_test_modules.h"

extern "C" {
  int py_test_ode_solver(void)
  {
      return test_ode_solver();
  }
}
