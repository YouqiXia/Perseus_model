#pragma once

namespace TimingModel {

    class TagEntry{
    friend class BaseCache;
    protected:
        uint64_t tag;
        uint64_t access_time;
        bool valid;

    public:
        TagEntry(){
            tag = 0;
            access_time = 0;
            valid = 0;
        }
        uint64_t getTag(){
            return tag;
        }
        bool isValid(){
            return valid;
        }
    };

    using setTags = std::vector<TagEntry>;
    using wayData = std::vector<uint8_t>;
} // namespace TimingModel