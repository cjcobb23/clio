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

    void
    invalidate(std::vector<ripple::uint256> const& keys)
    {
        for (auto key : keys)
        {
            std::unique_lock lck{mtx_};
            if (!cache_.contains(key))
                continue;

            auto& [it, obj] = cache_[key];
            queue_.erase(it);
            cache_.erase(key);
        }
    }

    std::optional<boost::json::object>
    get(ripple::uint256 const& key)
    {
        std::unique_lock lck{mtx_};
        if (!cache_.contains(key))
            return {};

        auto& [it, obj] = cache_[key];
        queue_.erase(it);
        queue_.push_front(obj);
        cache_[key].it = queue_.front();
        return obj;
    }

    void
    put(ripple::uint256 const& key, boost::json::object const& val)
    {
        std::unique_lock lck{mtx_};
        if (cache_.size() >= maxSize)
        {
            auto& key = queue_.back();
            cache_.erase(key);
            queue_.pop_back();
        }

        queue_.push_front(key);
        cache_[key] = {queue_.front(), obj};
    }
}
