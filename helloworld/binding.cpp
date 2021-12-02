#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_hello_world(py::module_& m) {
  m.def("hello_world", []() {
    py::print("hello world!");
    return "hello world!";
    });
}