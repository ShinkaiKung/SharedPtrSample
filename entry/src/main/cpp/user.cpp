#include <string>
#include <memory>
#include "napi/native_api.h"
#include <node_api.h>

struct UserOption {
    std::string id;
};

class User {
  public:
    static std::shared_ptr<User> create(const UserOption &userOption) {
        std::shared_ptr<User> ptr(new User(userOption));
        return ptr;
    }

    std::string get_id() {
        return id;
    }

  public:
    User(const UserOption &userOption) : id(userOption.id) {
    }
    ~User() {
    }
    std::string id;
    std::string name;
};

void DestroyUser(napi_env env, void *finalize_data, void *hint) {
    // 通过 static_cast 转换 finalize_data 回到正确的类型
    std::shared_ptr<User> *obj = static_cast<std::shared_ptr<User> *>(finalize_data);
    //    // 手动调用智能指针的析构函数来释放资源
    obj->~shared_ptr();
}

static long long count = 0;

static napi_value CreateUser(napi_env env, napi_callback_info info) {
    napi_status status;

    size_t argc = 1;
    napi_value args[1] = {nullptr};

    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        napi_throw_error(env, nullptr, "Invalid number of arguments");
        return nullptr;
    }

    size_t str_size;
    status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &str_size);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get string length");
        return nullptr;
    }

    std::string str;
    str.resize(str_size);
    status = napi_get_value_string_utf8(env, args[0], &str[0], str_size + 1, &str_size);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get string");
        return nullptr;
    }

    napi_value userObject;
    status = napi_create_object(env, &userObject);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create object");
        return nullptr;
    }

    std::shared_ptr<User> myClassInstance = User::create({.id = str});

    status = napi_wrap(env, userObject, new std::shared_ptr<User>(myClassInstance), DestroyUser, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to wrap native object");
        return nullptr;
    }

    return userObject;
}

static napi_value GetId(napi_env env, napi_callback_info info) {
    napi_status status;

    size_t argc = 1;
    napi_value args[1] = {nullptr};

    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        napi_throw_error(env, nullptr, "Invalid number of arguments");
        return nullptr;
    }

    std::shared_ptr<User> *user = nullptr;
    napi_unwrap(env, args[0], reinterpret_cast<void **>(&user));

    auto id = user->get()->get_id();
    napi_value jsIdStr;
    status = napi_create_string_utf8(env, id.c_str(), NAPI_AUTO_LENGTH, &jsIdStr);
    if (status != napi_ok) {
        // 错误处理
        napi_throw_error(env, nullptr, "Unable to create string");
        return nullptr;
    }
    return jsIdStr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"createUser", nullptr, CreateUser, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getId", nullptr, GetId, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "user",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) {
    napi_module_register(&demoModule);
}
