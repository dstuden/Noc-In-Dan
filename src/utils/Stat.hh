#pragma once

#include <type_traits>

#include "Tomos/util/reflect/TReflectAttr.hh"

namespace NocInDan
{
    template<typename T>
        requires std::is_arithmetic_v<T>
    struct TOMOS_ANN( Tomos::Reflect::DataMeta{ "stat", "Stat" } ) Stat
    {
        T m_minValue     = 0;
        T m_maxValue     = 100;
        T m_currentValue = 100;
    };
}  // namespace NocInDan
