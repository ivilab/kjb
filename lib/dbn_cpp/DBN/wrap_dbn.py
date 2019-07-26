from ctypes import cdll
from ctypes import create_string_buffer

lib = cdll.LoadLibrary('./libDBN')

# Calls our C++ routine which is easy because there are no input or output
# parameters. We had to make sure that the function is not name-mangled which we
# did by wrapping test_ode_solver() with py_test_ode_solver() which was declared
# extern "C".
lib.py_test_ode_solver()

# Let's prove that other routines are available. We do not need to wrap any C
# ones because C does not mangle names,. 
hw = create_string_buffer(b"hello world\n")
lib.p_stderr(hw.value)
