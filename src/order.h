#pragma once
#include "types.h"
using namespace std;

struct Order {
    U64 orderId;
    U64 participantId; // ignored for now. Will implement in the future once basic implementation is done
    Side side;
    int price; // in cents - 10050 = $100.50
    U64 quantity;
    U64 timestamp;

    Order(U64 order_id, U64 participant_id, Side s, int p, U64 q, U64 t)
    : orderId(order_id), participantId(participant_id), side(s), price(p), quantity(q), timestamp(t) {}
};