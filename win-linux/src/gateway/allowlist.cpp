#include "allowlist.h"

namespace Gateway
{
    AllowlistTable& AllowlistTable::Instance()
    {
        static AllowlistTable instance;
        return instance;
    }

    void AllowlistTable::Register(const QString& command, CommandSpec spec)
    {
        m_commands.emplace(command, std::move(spec));
    }

    const CommandSpec* AllowlistTable::Find(const QString& command) const
    {
        auto it = m_commands.find(command);
        return it == m_commands.end() ? nullptr : &it->second;
    }

    std::vector<QString> AllowlistTable::ListCommandNames() const
    {
        std::vector<QString> names;
        names.reserve(m_commands.size());
        for (const auto& entry : m_commands)
            names.push_back(entry.first);
        return names;
    }
}
