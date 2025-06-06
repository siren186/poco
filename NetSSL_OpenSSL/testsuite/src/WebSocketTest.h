//
// WebSocketTest.h
//
// Definition of the WebSocketTest class.
//
// Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef WebSocketTest_INCLUDED
#define WebSocketTest_INCLUDED


#include "Poco/Net/Net.h"
#include "CppUnit/TestCase.h"


class WebSocketTest: public CppUnit::TestCase
{
public:
	WebSocketTest(const std::string& name);
	~WebSocketTest() override;

	void testWebSocketTimeout();
	void testWebSocket();
	void testWebSocketLarge();
	void testWebSocketNB();

	void setUp() override;
	void tearDown() override;

	static CppUnit::Test* suite();

private:
};


#endif // WebSocketTest_INCLUDED
