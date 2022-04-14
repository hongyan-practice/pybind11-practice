#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;

class Matrix {
    public:
        Matrix(py::ssize_t rows, py::ssize_t cols) : m_rows(rows), m_cols(cols) {
            py::print(this, std::to_string(m_rows) + "x" + std::to_string(m_cols) + " matrix");
            m_data = new float[(size_t) (rows*cols)];
            memset(m_data, 0, sizeof(float) * (size_t) (rows * cols));
        }

        Matrix(const Matrix &s) : m_rows(s.m_rows), m_cols(s.m_cols) {
            py::print(this, std::to_string(m_rows) + "x" + std::to_string(m_cols) + " matrix");
            m_data = new float[(size_t) (m_rows * m_cols)];
            memcpy(m_data, s.m_data, sizeof(float) * (size_t) (m_rows * m_cols));
        }

        Matrix(Matrix &&s) noexcept : m_rows(s.m_rows), m_cols(s.m_cols), m_data(s.m_data) {
            py::print(this);
            s.m_rows = 0;
            s.m_cols = 0;
            s.m_data = nullptr;
        }

        ~Matrix() {
            py::print(std::to_string(m_rows) + "x" + std::to_string(m_cols) + " matrix");
            delete[] m_data;
        }

        Matrix &operator=(const Matrix &s) {
            if (this == &s) {
                return *this;
            }
            py::print(this, std::to_string(m_rows) + "x" + std::to_string(m_cols) + " matrix");
            delete[] m_data;
            m_rows = s.m_rows;
            m_cols = s.m_cols;
            m_data = new float[(size_t) (m_rows * m_cols)];
            memcpy(m_data, s.m_data, sizeof(float) * (size_t) (m_rows * m_cols));
            return *this;
        }

        Matrix &operator=(Matrix &&s) noexcept {
            py::print(this, std::to_string(m_rows) + "x" + std::to_string(m_cols) + " matrix");
            if (&s != this) {
                delete[] m_data;
                m_rows = s.m_rows; m_cols = s.m_cols; m_data = s.m_data;
                s.m_rows = 0; s.m_cols = 0; s.m_data = nullptr;
            }
            return *this;
        }

        float operator()(py::ssize_t i, py::ssize_t j) const {
            return m_data[(size_t) (i*m_cols + j)];
        }

        float &operator()(py::ssize_t i, py::ssize_t j) {
            return m_data[(size_t) (i*m_cols + j)];
        }

        float *data() { return m_data; }

        py::ssize_t rows() const { return m_rows; }
        py::ssize_t cols() const { return m_cols; }


    private:
        py::ssize_t m_rows;
        py::ssize_t m_cols;
        float *m_data;
};

class TickWrapper{
    public:
        TickWrapper():m_data(Matrix(2, 4)){
            for(size_t r = 0; r < m_data.rows(); ++r){
                for(size_t c = 0; c < m_data.cols(); ++c){
                    m_data(r, c) = r + c;
                }
            }
        }

        Matrix GetNdArray(){
            return m_data;
        }

        py::object GetDf(py::object& obj){
            py::object pandas = py::module::import("pandas");
            std::vector<std::string> columns_name{"uid", "score1", "score2", "score3"};
            py::list columns_name_list = py::cast(columns_name);
            py::object df = pandas.attr("DataFrame")(py::array_t<float>(
                {
                    m_data.rows(),
                    m_data.cols()},
                    {size_t(m_data.cols()) * sizeof(float), sizeof(float)},
                    m_data.data(),
                    obj),
                    "columns"_a = columns_name_list,
                    "copy"_a = false);
            df["symbol"] = "SHFE.cu2205";

            py::object origin_constructor = df.attr("_constructor_sliced");
            df.attr("_constructor_sliced") = py::cpp_function([=](py::args args, py::kwargs kwargs) {
                auto series = origin_constructor(*args, **kwargs);
                return series;
            });

            return df;
        }

        void RunOnce(){
            for(size_t r = 0; r < m_data.rows(); ++r){
                for(size_t c = 0; c < m_data.cols(); ++c){
                    m_data(r, c) = (r + c) * 10;
                }
            }
            py::print("update value", m_data);
        }

        Matrix m_data;
};

void binding_buffer_protocol(py::module_& m) {
    py::class_<Matrix>(m, "Matrix", py::buffer_protocol())
        .def(py::init<py::ssize_t, py::ssize_t>())
        /// Construct from a buffer
        .def(py::init([](const py::buffer &b) {
            py::buffer_info info = b.request();
            if (info.format != py::format_descriptor<float>::format() || info.ndim != 2)
                throw std::runtime_error("Incompatible buffer format!");

            auto v = new Matrix(info.shape[0], info.shape[1]);
            memcpy(v->data(), info.ptr, sizeof(float) * (size_t) (v->rows() * v->cols()));
            return v;
        }))

        .def("rows", &Matrix::rows)
        .def("cols", &Matrix::cols)

        /// Bare bones interface
        .def("__getitem__",
             [](const Matrix &m, std::pair<py::ssize_t, py::ssize_t> i) {
                 if (i.first >= m.rows() || i.second >= m.cols())
                     throw py::index_error();
                 return m(i.first, i.second);
             })
        .def("__setitem__",
             [](Matrix &m, std::pair<py::ssize_t, py::ssize_t> i, float v) {
                 if (i.first >= m.rows() || i.second >= m.cols())
                     throw py::index_error();
                 m(i.first, i.second) = v;
             })
        /// Provide buffer access
        .def_buffer([](Matrix &m) -> py::buffer_info {
            return py::buffer_info(
                m.data(),
                { m.rows(), m.cols() },
                { sizeof(float) * size_t(m.cols()),
                  sizeof(float) }
            );
        });

        py::class_<TickWrapper>(m, "Tick")
            .def(py::init<>())
            .def("get", &TickWrapper::GetNdArray)
            .def("get_df", [](py::object& obj){
                auto& api = obj.cast<TickWrapper&>();
                return api.GetDf(obj);
            })
            .def("wait_update", &TickWrapper::RunOnce);
}
