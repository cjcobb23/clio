class JSONCache
{
    struct CacheEntry
    {
        std::deque<ripple::uint256>::iterator iter;
        boost::json::object obj;
    };
    std::unordered_map<ripple::uint256, CacheEntry> cache_;
    std::deque<ripple::uint256> queue_;
    std::shared_mutex mtx_;

    class JSONCache(size_t maxSize)
    {
        cache_.reserve(maxSize);
    }

    bool
    contains(ripple::uint256 const& key)
    {
        std::shared_lock lck{mtx_};
        return cache_.contains(key);
    }

    void
    invalidate(std::vector<ripple::uint256> const& keys)
    {
        for (auto key : keys)
        {
            if (!contains(key))
                continue;

            std::unique_lock lck{mtx_};
            auto& [it, obj] = cache_[key];
            queue_.erase(it);
            cache_.erase(key);
        }
    }

    std::optional<boost::json::object>
    get(ripple::uint256 const& key)
    {
        if (!contains(key))
            return {};

        std::shared_lock lck{mtx_};
        auto& [it, obj] = cache_[key];
        queue_.erase(it);
        queue_.push_front(obj);
        cache_[key].it = queue_.front();
        return obj;
    }

    size_t
    size()
    {
        std::shared_lock lck{mtx_};
        return cache_.size();
    }

    void
    put(ripple::uint256 const& key, boost::json::object const& val)
    {
        if (size() >= maxSize)
        {
            std::unique_lock lck{mtx_};
            auto& it = queue_.back();
            cache_.erase(*it);
            queue_.pop_back();
        }

        std::unique_lock lck{mtx_};
        queue_.push_front(key);
        cache_[key] = {queue_.front(), obj};
    }
}
