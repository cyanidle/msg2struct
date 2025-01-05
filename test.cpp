#include "msg2struct.hpp"


struct Msg {
    int a;
    int b;
    MSG_2_STRUCT(a, b);
};

struct Msg2 : Msg {
    msg2struct::String string;
    MSG_2_STRUCT_INHERIT(Msg, string);
};

struct Msg3 : Msg2 {
    msg2struct::String string;
    MSG_2_STRUCT_INHERIT(Msg2, string);
};

int main() {
    unsigned char buff[] = {0x94, 0x7B, 0x14, 0xA5, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0xA5, 0x68, 0x65, 0x6C, 0x6C, 0x6F};
    auto it = msg2struct::Iterator(buff, sizeof(buff));
    it.Next();
    Msg3 msg;
    auto ok = msg2struct::Parse(msg, it);
    return !ok;
}
