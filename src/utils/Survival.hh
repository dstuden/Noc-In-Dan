#pragma once

#include "Inventory.hh"
#include "Stat.hh"
#include "Tomos/util/reflect/TReflectAttr.hh"

namespace NocInDan
{
    inline constexpr int g_kInventorySlots = 15;

    struct TOMOS_ANN( Tomos::Reflect::DataMeta{ "survival", "Survival" } ) Survival
    {
        Survival() : m_inventory( g_kInventorySlots ) {}

        Stat<float> m_health{ .m_minValue = 0.0f, .m_maxValue = 100.0f, .m_currentValue = 100.0f };
        Stat<float> m_hunger{ .m_minValue = 0.0f, .m_maxValue = 100.0f, .m_currentValue = 80.0f };
        Stat<float> m_sanity{ .m_minValue = 0.0f, .m_maxValue = 100.0f, .m_currentValue = 90.0f };

        Inventory m_inventory;
    };
}  // namespace NocInDan
