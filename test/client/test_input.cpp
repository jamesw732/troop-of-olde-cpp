#include <assert.h>
#include <iostream>

#include "client/input.hpp"
#include "shared/components.hpp"
#include "../helpers.hpp"

void test_input_buffer() {
    InputBuffer buffer;
    MovementInput front;
    MovementInput inp1{0, 0};
    MovementInput inp2{1, 0};
    MovementInput inp3{1, 1};
    MovementInput inp4{0, -1};

    buffer.flushUpTo(0);
    assertEquals(buffer.ack_tick, (uint16_t) -1);

    buffer.push(inp1);
    buffer.push(inp2);
    buffer.push(inp3);
    buffer.push(inp4);

    buffer.flushUpTo(0);
    front = buffer.buffer.front();
    assertEquals(front.x, 1);
    assertEquals(front.z, 0);
    assertEquals(buffer.ack_tick, 0);

    buffer.flushUpTo(0);
    front = buffer.buffer.front();
    assertEquals(front.x, 1);
    assertEquals(front.z, 0);
    assertEquals(buffer.ack_tick, 0);

    buffer.flushUpTo(2);
    front = buffer.buffer.front();
    assertEquals(front.x, 0);
    assertEquals(front.z, -1);
    assertEquals(buffer.ack_tick, 2);

    buffer.flushUpTo(3);
    assert(buffer.buffer.empty());
    assertEquals(buffer.ack_tick, 3);

    assertEquals(buffer.ack_tick, 3);
    buffer.flushUpTo(100); // Just make sure this doesn't crash the program
    std::cout << "Executed test_input_buffer without errors" << std::endl;
}

int main() {
    test_input_buffer();
}
