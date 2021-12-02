#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_hello_world(py::module_&);

PYBIND11_MODULE(demo, m) {
	m.attr("__name__") = "demo";

	init_hello_world(m);
}