#pragma once

#include "BaseCache.hpp"


namespace TimingModel {


    class L2Cache: public BaseCache{
        public:
            class L2CacheParameterSet : public BaseCache::BaseCacheParameterSet
            {
            public:
                L2CacheParameterSet(sparta::TreeNode* n):
                    BaseCache::BaseCacheParameterSet(n)
                { }
                // PARAMETER(uint32_t, l2_test, 1, "l2 param test")
            };

            L2Cache(sparta::TreeNode* node, const L2CacheParameterSet* p);

            void makeResp(const MemAccInfoPtr& req);
            void makeCriticalResp(const MemAccInfoPtr& req);
            void makeNonCriticalResp(const MemAccInfoPtr& req);
            void mshrRecvResp(uint32_t id, uint32_t subindex);
        public:
            // uint32_t l2_test_;
    };

}//namespace TimingModel
