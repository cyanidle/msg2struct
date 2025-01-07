#pragma once
#include "msg2struct.hpp"

struct Msg {
    int a;
    int ra;
    float b;
    bool c;
    MSG_2_STRUCT(a, ra, b, c);
};

struct Msg2 : Msg {
    msg2struct::String string;
    MSG_2_STRUCT_INHERIT(Msg, string);
};

struct Msg3 : Msg2 {
    msg2struct::Binary bin;
    MSG_2_STRUCT_INHERIT(Msg2, bin);
};

// TODO: TAGS in struct (first value of Array or special field in object to identify type + let user check it)
// Simpler: We send a PAIR of msgpacks instead of single object. Iter API for that

int main() {
    unsigned char buff[200];
    Msg3 msg;
    msg.a = -23;
    msg.ra = 0xc0;
    msg.b = 1.5;
    msg.c = true;
    msg.string = {"hello", 5};
    msg.bin = {(const unsigned char*)"world", 5};
    Msg3 msgBack;
    msg2struct::OutIterator oit(buff, sizeof(buff));
    auto outOk = msg2struct::Dump(msg, oit);
    msg2struct::InIterator it(buff, oit.Written());
    auto ok = msg2struct::Parse(msgBack, it);
    return !(ok && outOk);
}
