//
// Created by yzhang on 1/18/24.
//

#pragma once

namespace utils {
    template<class category, class T>
    struct IteratorTraits {
        using difference_type = long;
        using value_type      = T;
        using pointer         = const T*;
        using reference       = const T&;
        using iterator_category = category;
    };
}
