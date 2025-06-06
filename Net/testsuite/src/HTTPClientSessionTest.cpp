//
// HTTPClientSessionTest.cpp
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "HTTPClientSessionTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "HTTPTestServer.h"
#include <istream>
#include <ostream>
#include <sstream>


using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::StreamCopier;
using Poco::File;
using Poco::Path;


HTTPClientSessionTest::HTTPClientSessionTest(const std::string& name): CppUnit::TestCase(name)
{
}


HTTPClientSessionTest::~HTTPClientSessionTest()
{
}


void HTTPClientSessionTest::testGetSmall()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_GET, "/small");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::SMALL_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == HTTPTestServer::SMALL_BODY);
}


void HTTPClientSessionTest::testGetSmallUnix()
{
#if defined(POCO_HAS_UNIX_SOCKET)
#if POCO_OS == POCO_OS_ANDROID
	File socketFile("/data/local/tmp/SocketTest.sock");
#elif defined(POCO_OS_FAMILY_WINDOWS)
	File socketFile(Path::tempHome() + "SocketTest.sock");
#else
	File socketFile("/tmp/SocketTest.sock");
#endif // POCO_OS == POCO_OS_ANDROID
	if (socketFile.exists()) socketFile.remove();
	HTTPTestServer srv(socketFile.path());
	HTTPClientSession s(socketFile.path());
	HTTPRequest request(HTTPRequest::HTTP_GET, "/small");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue(response.getContentLength() == HTTPTestServer::SMALL_BODY.length());
	assertTrue(response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue(ostr.str() == HTTPTestServer::SMALL_BODY);
#endif // POCO_HAS_UNIX_SOCKET
}


void HTTPClientSessionTest::testGetLarge()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_GET, "/large");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::LARGE_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == HTTPTestServer::LARGE_BODY);
}


void HTTPClientSessionTest::testHead()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_HEAD, "/large");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::LARGE_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	assertTrue (StreamCopier::copyStream(rs, ostr) == 0);
}


void HTTPClientSessionTest::testPostSmallIdentity()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/echo");
	std::string body("this is a random request body\r\n0\r\n");
	request.setContentLength((int) body.length());
	s.sendRequest(request) << body;
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == body.length());
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == body);
}


void HTTPClientSessionTest::testPostLargeIdentity()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/echo");
	std::string body(8000, 'x');
	body.append("\r\n0\r\n");
	request.setContentLength((int) body.length());
	s.sendRequest(request) << body;
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == body.length());
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == body);
}


void HTTPClientSessionTest::testPostSmallChunked()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/echo");
	std::string body("this is a random request body");
	request.setChunkedTransferEncoding(true);
	s.sendRequest(request) << body;
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getChunkedTransferEncoding());
	assertTrue (response.getContentLength() == HTTPMessage::UNKNOWN_CONTENT_LENGTH);
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == body);
}


void HTTPClientSessionTest::testPostLargeChunked()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/echo");
	std::string body(16000, 'x');
	request.setChunkedTransferEncoding(true);
	std::ostream& os = s.sendRequest(request);
	os << body;
	os.flush();
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getChunkedTransferEncoding());
	assertTrue (response.getContentLength() == HTTPMessage::UNKNOWN_CONTENT_LENGTH);
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr, 16000);
	assertTrue (ostr.str() == body);
}


void HTTPClientSessionTest::testKeepAlive()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	s.setKeepAlive(true);
	HTTPRequest request(HTTPRequest::HTTP_HEAD, "/keepAlive", HTTPMessage::HTTP_1_1);
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs1 = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::SMALL_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	assertTrue (response.getKeepAlive());
	std::ostringstream ostr1;
	assertTrue (StreamCopier::copyStream(rs1, ostr1) == 0);

	request.setMethod(HTTPRequest::HTTP_GET);
	request.setURI("/small");
	s.sendRequest(request);
	std::istream& rs2 = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::SMALL_BODY.length());
	assertTrue (response.getKeepAlive());
	std::ostringstream ostr2;
	StreamCopier::copyStream(rs2, ostr2);
	assertTrue (ostr2.str() == HTTPTestServer::SMALL_BODY);

	request.setMethod(HTTPRequest::HTTP_GET);
	request.setURI("/large");
	s.sendRequest(request);
	std::istream& rs3 = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPMessage::UNKNOWN_CONTENT_LENGTH);
	assertTrue (response.getChunkedTransferEncoding());
	assertTrue (response.getKeepAlive());
	std::ostringstream ostr3;
	StreamCopier::copyStream(rs3, ostr3);
	assertTrue (ostr3.str() == HTTPTestServer::LARGE_BODY);

	request.setMethod(HTTPRequest::HTTP_HEAD);
	request.setURI("/large");
	s.sendRequest(request);
	std::istream& rs4= s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::LARGE_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	assertTrue (!response.getKeepAlive());
	std::ostringstream ostr4;
	assertTrue (StreamCopier::copyStream(rs4, ostr4) == 0);
}


void HTTPClientSessionTest::testTrailer()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	s.setKeepAlive(true);
	HTTPRequest request(HTTPRequest::HTTP_GET, "/trailer", HTTPMessage::HTTP_1_1);
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == HTTPTestServer::LARGE_BODY);
	assertTrue (!s.responseTrailer().empty());
	assertTrue (s.responseTrailer().get("Trailer-1") == "Value 1");
	assertTrue (s.responseTrailer().get("Trailer-2") == "Value 2");
}


void HTTPClientSessionTest::testProxy()
{
	HTTPTestServer srv;
	HTTPClientSession s("www.somehost.com");
	s.setProxy("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_GET, "/large");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::LARGE_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == HTTPTestServer::LARGE_BODY);
}


void HTTPClientSessionTest::testProxyAuth()
{
	HTTPTestServer srv;
	HTTPClientSession s("www.somehost.com");
	s.setProxy("127.0.0.1", srv.port());
	s.setProxyCredentials("user", "pass");
	HTTPRequest request(HTTPRequest::HTTP_GET, "/large");
	s.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getContentLength() == HTTPTestServer::LARGE_BODY.length());
	assertTrue (response.getContentType() == "text/plain");
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == HTTPTestServer::LARGE_BODY);
	std::string r = srv.lastRequest();
	assertTrue (r.find("Proxy-Authorization: Basic dXNlcjpwYXNz\r\n") != std::string::npos);
}


void HTTPClientSessionTest::testBypassProxy()
{
	HTTPClientSession::ProxyConfig proxyConfig;
	proxyConfig.host = "proxy.domain.com";
	proxyConfig.port = 80;
	proxyConfig.nonProxyHosts = "localhost|127\\.0\\.0\\.1";

	HTTPClientSession s1("127.0.0.1", 80);
	s1.setProxyConfig(proxyConfig);
	assertTrue (s1.bypassProxy());

	HTTPClientSession s2("127.0.0.1", 80);
	s2.setProxyConfig(proxyConfig);
	assertTrue (s2.bypassProxy());

	HTTPClientSession s3("www.appinf.com", 80);
	s3.setProxyConfig(proxyConfig);
	assertTrue (!s3.bypassProxy());
}


void HTTPClientSessionTest::testExpectContinue()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/expect");
	std::string body("this is a random request body\r\n0\r\n");
	request.setContentLength((int) body.length());
	request.setExpectContinue(true);
	s.sendRequest(request) << body;
	HTTPResponse response;
	assertTrue (s.peekResponse(response));
	assertTrue (response.getStatus() == HTTPResponse::HTTP_CONTINUE);
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getStatus() == HTTPResponse::HTTP_OK);
	assertTrue (response.getContentLength() == body.length());
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str() == body);
}


void HTTPClientSessionTest::testExpectContinueFail()
{
	HTTPTestServer srv;
	HTTPClientSession s("127.0.0.1", srv.port());
	HTTPRequest request(HTTPRequest::HTTP_POST, "/fail");
	std::string body("this is a random request body\r\n0\r\n");
	request.setContentLength((int) body.length());
	request.setExpectContinue(true);
	s.sendRequest(request) << body;
	HTTPResponse response;
	assertTrue (!s.peekResponse(response));
	assertTrue (response.getStatus() == HTTPResponse::HTTP_BAD_REQUEST);
	std::istream& rs = s.receiveResponse(response);
	assertTrue (response.getStatus() == HTTPResponse::HTTP_BAD_REQUEST);
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);
	assertTrue (ostr.str().empty());
}


void HTTPClientSessionTest::setUp()
{
}


void HTTPClientSessionTest::tearDown()
{
}


CppUnit::Test* HTTPClientSessionTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("HTTPClientSessionTest");

	CppUnit_addTest(pSuite, HTTPClientSessionTest, testGetSmall);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testGetSmallUnix);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testGetLarge);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testHead);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testPostSmallIdentity);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testPostLargeIdentity);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testPostSmallChunked);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testPostLargeChunked);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testKeepAlive);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testTrailer);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testProxy);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testProxyAuth);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testBypassProxy);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testExpectContinue);
	CppUnit_addTest(pSuite, HTTPClientSessionTest, testExpectContinueFail);

	return pSuite;
}
