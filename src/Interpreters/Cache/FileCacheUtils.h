#pragma once

namespace FileCacheUtils
{

static size_t roundDownToMultiple(size_t num, size_t multiple)
{
    if (!multiple)
        return num;
    return (num / multiple) * multiple;
}

static size_t roundUpToMultiple(size_t num, size_t multiple)
{
    if (!multiple)
        return num;

    /// Check for potential overflow in: num + multiple - 1. This happens when
    /// num + multiple - 1 > SIZE_MAX (max value of size_t)
    if (num > SIZE_MAX - multiple + 1)
    {
        /// Return SIZE_MAX as we can't round up further.
        return SIZE_MAX;
    }

    return roundDownToMultiple(num + multiple - 1, multiple);
}

}
