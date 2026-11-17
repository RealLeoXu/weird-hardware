// RNBO.cpp
//
// Cycling '74 原版 RNBO SDK 的 unity-build 控制文件：先 #include "RNBO.h"
// 让所有类的完整声明先被处理一遍，再按固定顺序 #include 各个 .cpp 的实现，
// 全部作为同一个编译单元处理。这个顺序不是随便排的——比如 RNBO_PatcherState.cpp
// 排在前面，是因为后面某些文件（RNBO_PresetList.cpp 等）里用到
// std::make_unique<Preset>() 时需要 PatcherState 类已经是完整定义，不是
// forward declaration。之前把这 14 个 .cpp 当成各自独立的 Arduino 编译单元
// 编译，绕开了这个顺序保证，会在 RNBO_Presets.h 里报 "invalid use of
// incomplete type PatcherState"。
//
// 之前排除本文件、只单独编译那 14 个 .cpp，是为了避免"两边都编译，符号重复
// 定义"的链接错误——但正确的排除对象其实是反过来的：应该让 Arduino 只编译
// 这一个文件（本文件），那 14 个 .cpp 不能再被 Arduino 当独立编译单元发现，
// 所以在库里把它们改名成 .cpp.inc（内容完全不变，只是后缀名），本文件用
// #include 把它们的源码文本原样接进来——#include 不看后缀名，一样能用。
//
// RNBO_ExternalLoader.cpp 没有列在下面：那是给桌面版 Max/MSP 动态加载外部
// .dll/.so 模块用的，依赖 Max SDK 专属头文件、还需要 dlopen 之类的动态库
// 加载能力，ESP32 裸机环境完全用不上，已经整个移到
// AmbienceEngine/excluded_desktop_only_not_compiled/ 里了。

#include "RNBO.h"

#include "src/RNBO_CoreObject.cpp.inc"
#include "src/RNBO_DataBuffer.cpp.inc"
#include "src/RNBO_DataRefList.cpp.inc"
#include "src/RNBO_DynamicPatcherFactory.cpp.inc"
#include "src/RNBO_Engine.cpp.inc"
#include "src/RNBO_FileChangeWatcher.cpp.inc"
#include "src/RNBO_MaxClang.cpp.inc"
#include "src/RNBO_ParameterInterfaceAsync.cpp.inc"
#include "src/RNBO_ParameterInterfaceSync.cpp.inc"
#include "src/RNBO_PatcherFactory.cpp.inc"
#include "src/RNBO_PatcherState.cpp.inc"
#include "src/RNBO_PresetList.cpp.inc"
#include "src/RNBO_LoggerImpl.cpp.inc"
