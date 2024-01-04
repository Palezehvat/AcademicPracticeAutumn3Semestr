#include <Python.h>
#include <iostream>
#include <smm-2ng-plugin.h>
#include <mutex>

namespace SMM {

class PythonAlgorithm : public AbstractAlgorithm {
    private:
        PyObject *class_instance;
    public:
	    void init(const nlohmann::json* args) override;
	    SMMValues run(const SMMValues& args) override;
        const char* getMetaData() const override;
        ~PythonAlgorithm() {
            if (class_instance != nullptr) {
                Py_DECREF(class_instance);
            }
        }
};

DECLARE_SMM_ALGORITHM(
    PythonAlgorithm,
    "Algorithm for working with Python",
    "Launches Python from C++",
    ALGO_PARAMS_LIST(),
    ALGO_PARAMS_LIST(
        ALGO_OUTPUT_PARAM(ready, "Ready", bool, "ready state"),
        ALGO_OUTPUT_PARAM(data, "Output", userdata_t, "bytes"),
        ALGO_OUTPUT_PARAM(counter, "Counter", uint32_t, "ticks")
    )
)

} //namespace 