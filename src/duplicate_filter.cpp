#include <qrMqtt/duplicate_filter.hpp>

namespace qrmqtt
{
DuplicateFilter::DuplicateFilter(int cooldown_seconds)
  : cooldown_(cooldown_seconds)
{
}

bool DuplicateFilter::accept(const std::string &payload)
{
    const auto now = std::chrono::steady_clock::now();
    const auto found = seen_.find(payload);
    if (found != seen_.end() && now - found->second < cooldown_)
    {
        return false;
    }
    seen_[payload] = now;
    for (auto item = seen_.begin(); item != seen_.end();)
    {
        if (now - item->second >= cooldown_)
        {
            item = seen_.erase(item);
        }
        else
        {
            ++item;
        }
    }
    return true;
}
}
