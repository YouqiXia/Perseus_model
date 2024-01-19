#pragma once

#include "BaseCache.hpp"


namespace TimingModel {


    class L1dCache: public BaseCache{
        public:
            class L1dCacheParameterSet : public BaseCache::BaseCacheParameterSet
            {
            public:
                L1dCacheParameterSet(sparta::TreeNode* n):
                    BaseCache::BaseCacheParameterSet(n)
                { }
                // PARAMETER(uint32_t, l1d_test, 1, "l1d param test")
            };

            L1dCache(sparta::TreeNode* node, const L1dCacheParameterSet* p);

            void makeResp(const MemAccInfoPtr& req);
            void makeCriticalResp(const MemAccInfoPtr& req);
            void makeNonCriticalResp(const MemAccInfoPtr& req);
            void mshrRecvResp(uint32_t id, uint32_t subindex);
        public:
            // uint32_t l1d_test_;
    };

}//namespace TimingModel
