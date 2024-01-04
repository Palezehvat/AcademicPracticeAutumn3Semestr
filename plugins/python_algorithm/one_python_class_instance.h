#include <python3.11/Python.h>

class OnePythonClassInstance {
    public:
        static OnePythonClassInstance* get_instance() {
            static OnePythonClassInstance *instance = nullptr;
            if (!instance) {
                instance = new OnePythonClassInstance();
            }
            return instance;
        }
    private:
        OnePythonClassInstance () {
            Py_Initialize();
        }
        OnePythonClassInstance(const OnePythonClassInstance&) = delete;
        OnePythonClassInstance& operator = (const OnePythonClassInstance&) = delete;
        ~OnePythonClassInstance() {
            Py_Finalize();
        }

};