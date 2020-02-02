/*
 * TennisMachineTest.cpp
 *
 *  Created on: 02.02.2020
 *      Author: mj
 */

#include "TestSupport.h"
#include "TennisMachine.h"

#define TESTPR(m,v) printf(m, v)

TennisMachine createSUT() {
    TennisMachine sut;
    TennisMachine::Client client;
    client.print = [](FONT_ATTRIB a, String m)
    {   TESTPR("DISPLAY: '%s'\n", m.c_str());};
    client.send = [](String m)
    {   TESTPR("> '%s'\n", m.c_str());};
    sut.setClient(client);
    return sut;
}

void test_TennisMachine_1()
{
    TennisMachine sut = createSUT();
    WordBuffer buf;

    sut.start();
    assertEquals("initial", "initial", sut.getState());

    buf.addWord("cq");
    sut.onMessageTransmit(buf);
    assertEquals("end", "end", sut.getState());
}
