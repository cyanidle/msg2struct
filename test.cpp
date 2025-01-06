#include "msg2struct.hpp"

struct Msg {
    int a;
    float b;
    MSG_2_STRUCT(a, b);
};

struct Msg2 : Msg {
    msg2struct::String string;
    MSG_2_STRUCT_INHERIT(Msg, string);
};

struct Msg3 : Msg2 {
    msg2struct::Binary bin;
    MSG_2_STRUCT_INHERIT(Msg2, bin);
};

int main() {
    unsigned char buff[200];
    Msg3 msg;
    msg.a = -23;
    msg.b = 1.5;
    msg.string = {"hello", 5};
    msg.bin = {(const unsigned char*)"world", 5};
    Msg3 msgBack;
    msg2struct::OutIterator oit(buff, sizeof(buff));
    auto outOk = msg2struct::Dump(msg, oit);
    msg2struct::InIterator it(buff, sizeof(buff));
    auto ok = msg2struct::Parse(msgBack, it);
    return !(ok && outOk);
}
