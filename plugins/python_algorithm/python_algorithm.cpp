#include "python_algorithm.h"
#include "one_python_class_instance.h"

using namespace SMM;

namespace {

userdata_t get_userdata_from_string(std::string string_data) {
    size_t size = string_data.size();
    uint8_t* data = new uint8_t[size];
    std::memcpy(data, string_data.c_str(), size);
    userdata_t userdata{data, size};
    return userdata;
}

std::string get_string_from_userdata(userdata_t data) {
    return std::string(reinterpret_cast<char*>(data.first), data.second); 
}

PyObject* create_tuple_by_SMMValue(const SMMValue& value) {
    SMM::SMMValueType type = SMM::get_type(value);
    switch(type) {
        case SMMValueType::Unset:
            return Py_BuildValue("(sO)", SMM::to_string(type), Py_None);
        case SMMValueType::Bool:
            return Py_BuildValue("(si)", SMM::to_string(type), static_cast<int>(std::get<bool>(value)));
        case SMMValueType::UInt8:
            return  Py_BuildValue("(si)", SMM::to_string(type), static_cast<int>(std::get<uint8_t>(value)));
        case SMMValueType::UInt16:
            return  Py_BuildValue("(si)", SMM::to_string(type), static_cast<int>(std::get<uint16_t>(value)));
        case SMMValueType::UInt32:
            return  Py_BuildValue("(si)", SMM::to_string(type), static_cast<int>(std::get<uint32_t>(value)));
        case SMMValueType::UInt64:
            return  Py_BuildValue("(si)", SMM::to_string(type), static_cast<int>(std::get<uint64_t>(value)));
        case SMMValueType::Double:
            return Py_BuildValue("(sd)", SMM::to_string(type), std::get<double>(value));
        case SMMValueType::Userdata:
            return Py_BuildValue("(ss)", SMM::to_string(type), get_string_from_userdata(std::get<userdata_t>(value)));
    }
    throw std::runtime_error(std::string("Not reached"));
}

PyObject* get_dictionary_parameters(const SMMValues& args) {
    PyObject *dict = PyDict_New();
    for (const auto& [key, value] : args) {
        int result = PyDict_SetItemString(dict, key.c_str(), create_tuple_by_SMMValue(value));
        if (result < 0) {
            throw std::runtime_error(std::string("Problems build dictionary"));
        }
    }

    return dict;
}

SMMValue add_value(PyObject* value) {
    if (!PyTuple_Check(value)) {
        throw std::runtime_error(std::string("The python algorithm received incorrect value from algorithm"));
    }

    PyObject* string_type = PyTuple_GetItem(value, 0);
    PyObject* value_from_tuple = PyTuple_GetItem(value, 1);

    switch (type_from_string(static_cast<std::string>(PyBytes_AsString(PyUnicode_AsUTF8String(string_type))))) {
        case SMMValueType::Unset:
            return std::monostate{};
        case SMMValueType::Bool:
            return static_cast<bool>(PyObject_IsTrue(value_from_tuple));
        case SMMValueType::UInt8:
            return  static_cast<uint8_t>(PyLong_AsUnsignedLongMask(value_from_tuple));
        case SMMValueType::UInt16:
            return  static_cast<uint16_t>(PyLong_AsUnsignedLongMask(value_from_tuple));
        case SMMValueType::UInt32:
            return  static_cast<uint32_t>(PyLong_AsUnsignedLongMask(value_from_tuple));
        case SMMValueType::UInt64:
            return  static_cast<uint64_t>(PyLong_AsUnsignedLongMask(value_from_tuple));
        case SMMValueType::Double:
            return PyFloat_AsDouble(value_from_tuple);
        case SMMValueType::Userdata:
            return  get_userdata_from_string(static_cast<std::string>(PyBytes_AsString(PyUnicode_AsUTF8String(value_from_tuple))));
    }
    throw std::runtime_error(std::string("Reached"));
}

static SMMValues create_SMMValues(PyObject *dict) {
    
    SMMValues new_value;
    PyObject *keys = PyDict_Keys(dict);
    PyObject *values = PyDict_Values(dict);

    Py_ssize_t size = PyList_Size(keys);
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyList_GetItem(values, i);

        PyObject *key_string = PyObject_Str(key);
        const char *key_char_string = PyUnicode_AsUTF8(key_string);
        new_value[key_char_string] = add_value(value);
    }

    return new_value;
}

} //namespace

void PythonAlgorithm::init(const nlohmann::json* args) {
    std::string name_from_json = (*args)["name"];
    std::filesystem::path name = getAssetsFolder();
    name.append(name_from_json);
    size_t last_slash_pos = name.generic_string().find_last_of('/');
    std::string path_to_module = name.generic_string().substr(0, last_slash_pos);
    const std::string algo_name = (*args)["algo_name"];

    nlohmann::json new_json = {
        {"config", (*args)["config"]}
    };
    std::string json_string = new_json.dump();

    OnePythonClassInstance::get_instance();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(("sys.path.append('" + path_to_module + "')").c_str());
    
    PyObject *python_name = PyUnicode_DecodeFSDefault(name.filename().generic_string().c_str());
    PyObject *python_module = PyImport_Import(python_name);

    Py_DECREF(python_name); 
    if (python_module == NULL) {
        throw std::runtime_error(std::string("Python module not found"));
    }

    PyObject *python_dict = PyModule_GetDict(python_module);

    PyObject *python_class = PyDict_GetItemString(python_dict, algo_name.c_str());

    if (!(python_class != NULL && PyCallable_Check(python_class))) {
        throw std::runtime_error(std::string("Problems with work with Python class"));
    }

    Py_DECREF(python_module);
    Py_DECREF(python_dict); 

    PyObject *python_args = PyTuple_Pack(1, PyUnicode_FromString(json_string.c_str()));

    this->class_instance = PyObject_CallObject(python_class, python_args); 

    Py_DECREF(python_args);
}

SMMValues PythonAlgorithm::run(const SMMValues& args) {
    PyObject* in_params = get_dictionary_parameters(args);
    PyObject* python_method = PyObject_GetAttrString(class_instance, "run");
    if (!(PyCallable_Check(python_method))) {
        throw std::runtime_error(std::string("Problems with work with Python method"));
    }

    PyObject* out_params = PyObject_CallFunctionObjArgs(python_method, in_params, NULL);

    auto value = create_SMMValues(out_params);

    return value;
}