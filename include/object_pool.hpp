#pragma once

#include <span>
#include <utility>
#include <vector>

namespace horde
{
template <typename T>
class ObjectPool
{
public:
    ObjectPool() = default;
    ~ObjectPool() = default;

    void PreAllocate(size_t capacity)
    {
        m_Data.resize(capacity);
    }

    auto Acquire() -> T*
    {
        if (m_ActiveCount >= m_Data.size())
        {
            return nullptr;
        }
        return &m_Data[m_ActiveCount++];
    }

    void Release(size_t index)
    {
        if (index >= m_ActiveCount)
        {
            return;
        }
        --m_ActiveCount;
        if (index != m_ActiveCount)
        {
            std::swap(m_Data[index], m_Data[m_ActiveCount]);
        }
    }

    template <typename Predicate>
    void RemoveIf(Predicate pred)
    {
        for (size_t i = 0; i < m_ActiveCount;)
        {
            if (pred(m_Data[i]))
            {
                --m_ActiveCount;
                if (i != m_ActiveCount)
                {
                    std::swap(m_Data[i], m_Data[m_ActiveCount]);
                }
            }
            else
            {
                ++i;
            }
        }
    }

    [[nodiscard]] auto GetActive() -> std::span<T>
    {
        return std::span<T>{m_Data.data(), m_ActiveCount};
    }

    [[nodiscard]] auto GetActive() const -> std::span<const T>
    {
        return std::span<const T>{m_Data.data(), m_ActiveCount};
    }

    [[nodiscard]] auto GetCount() const -> size_t
    {
        return m_ActiveCount;
    }

    void Clear()
    {
        m_ActiveCount = 0;
    }

private:
    std::vector<T> m_Data;
    size_t m_ActiveCount{0};
};
}
