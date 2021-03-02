//
// Created by 42025 on 2021/3/2.
//
#include <PGJson/String.h>
#include <PGJson/Allocator.h>
#include <PGJson/utils.h>

void pg::base::json::String::copyFrom(const pg::base::json::Char * str, pg::base::json::SizeType len) {
    if (str == nullptr) return;

    if (len == std::numeric_limits<decltype(length)>::max())
        len = pg::base::json::getCStringLength(str);

    // small-string, if the true-length <= String::SMALL_STRING_MAX_SIZE
    if (len <= String::SMALL_STRING_MAX_SIZE) {
        // release
        if (!usingSmall) PGJSON_FREE(data);

        usingSmall = true;
        std::memcpy(sData, str, len);
        sData[len] = 0;
        sLen = static_cast<std::uint8_t>(len);
        return;
    }

    // if the true-length is more than String::SMALL_STRING_MAX_SIZE
    // allocate memory and free old
    if (usingSmall) data = NULL;
    void * ptr = PGJSON_REALLOC(data, sizeof(Char) * (len + 1));

    // copy string from str to ptr
    std::memcpy(ptr, str, len);
    reinterpret_cast<Byte *>(ptr)[len] = 0;
    usingSmall = false;
    data = reinterpret_cast<Char*>(ptr);
    length = len;
}

void pg::base::json::String::destroy() {

    if (usingSmall) return;

    PGJSON_FREE(data);
}

bool pg::base::json::stringEquals(const Char * str, const String & str2) {
    if (str2.usingSmall)
        return stringEquals(str, reinterpret_cast<const Char *>(str2.sData));

    return stringEquals(str, str2.data);
}
