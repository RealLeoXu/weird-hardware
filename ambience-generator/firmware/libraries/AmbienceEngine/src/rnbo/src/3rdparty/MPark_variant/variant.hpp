// variant.hpp
//
// 原本这里放的是 mpark/variant（给不支持 C++17 std::variant 的老编译器用的
// polyfill）。ESP32 的 Arduino 工具链（arduino-esp32 3.3.11 自带的
// xtensa-esp-elf-g++ 14.2.0）实际是用 -std=gnu++2a（C++20）编译的，mpark
// 自己那套 SFINAE 重载匹配（专门为兼容老编译器写的 best_match trick）在
// GCC 14 + C++20 下编译不过（重载决议失败）。RNBO 代码里只用到了
// mpark::variant/monostate/get/holds_alternative/visit 这五个名字，C++17
// 的 std::variant 已经原生提供、语义完全一致，所以直接把 mpark:: 这几个
// 名字指向标准库实现，不再引入 mpark 那份 vendored 代码。
#pragma once

#include <variant>

namespace mpark {
    using std::variant;
    using std::monostate;
    using std::get;
    using std::holds_alternative;
    using std::visit;
    using std::in_place_type_t;
    using std::in_place_index_t;
    using std::bad_variant_access;
}
