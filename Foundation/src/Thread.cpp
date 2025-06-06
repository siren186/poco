//
// Thread.cpp
//
// Library: Foundation
// Package: Threading
// Module:  Thread
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/Exception.h"
#include "Poco/ThreadLocal.h"
#include "Poco/AtomicCounter.h"
#include <sstream>


#if defined(POCO_OS_FAMILY_WINDOWS)
#include "Thread_WIN32.cpp"
#elif defined(POCO_VXWORKS)
#include "Thread_VX.cpp"
#else
#include "Thread_POSIX.cpp"
#endif


namespace Poco {


namespace {


class RunnableHolder: public Runnable
{
public:
	RunnableHolder(Runnable& target):
		_target(target)
	{
	}

	~RunnableHolder()
	{
	}

	void run()
	{
		_target.run();
	}

private:
	Runnable& _target;
};


class CallableHolder: public Runnable
{
public:
	CallableHolder(Thread::Callable callable, void* pData):
		_callable(callable),
		_pData(pData)
	{
	}

	~CallableHolder() override = default;

	void run() override
	{
		_callable(_pData);
	}

private:
	Thread::Callable _callable;
	void* _pData;
};


} // namespace


Thread::Thread(uint32_t sigMask):
	_id(uniqueId()),
	_pTLS(nullptr),
	_event(Event::EVENT_AUTORESET),
	_interruptionRequested(false)
{
	setNameImpl(makeName());
#if defined(POCO_OS_FAMILY_UNIX)
	setSignalMaskImpl(sigMask);
#endif
}


Thread::Thread(const std::string& name, uint32_t sigMask):
	_id(uniqueId()),
	_pTLS(nullptr),
	_event(Event::EVENT_AUTORESET),
	_interruptionRequested(false)
{
	setNameImpl(name);
#if defined(POCO_OS_FAMILY_UNIX)
	setSignalMaskImpl(sigMask);
#endif
}


Thread::~Thread()
{
	delete _pTLS;
}


void Thread::setPriority(Priority prio)
{
	setPriorityImpl(prio);
}


Thread::Priority Thread::getPriority() const
{
	return Priority(getPriorityImpl());
}


void Thread::start(Runnable& target)
{
	startImpl(new RunnableHolder(target));
}


void Thread::start(Poco::SharedPtr<Runnable> pTarget)
{
	startImpl(pTarget);
}


void Thread::start(Callable target, void* pData)
{
	startImpl(new CallableHolder(target, pData));
}


void Thread::join()
{
	joinImpl();
}


void Thread::join(long milliseconds)
{
	if (!joinImpl(milliseconds))
		throw TimeoutException();
}


bool Thread::tryJoin(long milliseconds)
{
	return joinImpl(milliseconds);
}


bool Thread::trySleep(long milliseconds)
{
	Thread* pT = Thread::current();
	poco_check_ptr(pT);
	return !(pT->_event.tryWait(milliseconds));
}


void Thread::wakeUp()
{
	_event.set();
}


bool Thread::isInterrupted()
{
	return _interruptionRequested.load(std::memory_order_relaxed);
}


void Thread::checkInterrupted()
{
	bool expected = true;
	if (_interruptionRequested.compare_exchange_strong(expected, false))
	{
		throw Poco::ThreadInterruptedException("Thread interrupted");
	}
}


void Thread::interrupt()
{
	_interruptionRequested.store(true, std::memory_order_relaxed);
	wakeUp();
}


void Thread::clearInterrupt()
{
	_interruptionRequested.store(false, std::memory_order_relaxed);
}


ThreadLocalStorage& Thread::tls()
{
	if (!_pTLS)
		_pTLS = new ThreadLocalStorage;
	return *_pTLS;
}


void Thread::clearTLS()
{
	if (_pTLS)
	{
		delete _pTLS;
		_pTLS = nullptr;
	}
}


std::string Thread::makeName()
{
	std::ostringstream name;
	name << '#' << _id;
	return name.str();
}


int Thread::uniqueId()
{
	static Poco::AtomicCounter counter;
	return ++counter;
}


void Thread::setName(const std::string& name)
{
	setNameImpl(name);
}


} // namespace Poco
