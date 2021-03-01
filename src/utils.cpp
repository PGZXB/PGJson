//
// Created by 42025 on 2021/2/28.
//

#include <PGJson/utils.h>

pg::base::json::SizeType pg::base::json::getUTF8StringLength(const char *str) {
    SizeType i = 0, count = 0;
    char ch;

    for (; str[i]; ++count) {
        ch = str[i];
        if(ch & 0x80) {
            while(ch & 0x80){
                ch <<= 1;
                ++i;
            }
        } else ++i;
    }

    return count;
}