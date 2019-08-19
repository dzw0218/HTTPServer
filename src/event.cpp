#include "event.h"
#include "log.h"

namespace Event
{
//handler类定义
int Handler::register_event(int fd, EventType type)
{
    return EventProxy::instance()->register_event(fd, this, type);
}

int Handler::register_event(SocketInterface::SocketDest &socket, EventType type)
{
    return EventProxy::instance()->register_event(socket, this, type);
}

int Handler::shutdown_event(int fd)
{
    return EventProxy::instance()->shutdown_event(fd);
}

int Handler::shutdown_event(SocketInterface::SocketDest &socket)
{
    return EventProxy::instance()->shutdown_event(socket);
}

//MyObserver类的定义
MyObserver::MyObserver(Observer &observer, EventType type)
    : m_observer(observer),
    m_eventtype(type),
    m_refcount(1)
{
}

MyObserver::~MyObserver()
{
}

int MyObserver::increaseRef()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_refcount++;
}

int MyObserver::reduceRef()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_refcount--;
}

bool MyObserver::countRef()
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_refcount--;
    return (m_refcount == 0);
}

void MyObserver::release()
{
    delete this;
}

EventType MyObserver::getEventType()
{
    return m_eventtype;
}

const Observer* MyObserver::getHandle()
{
    return &m_observer;
}

void MyObserver::epoll_handlein(int fd)
{
    m_observer.epoll_handlein(fd);
}

void MyObserver::epoll_handleout(int fd)
{
    m_observer.epoll_handleout(fd);
}

void MyObserver::epoll_handleclose(int fd)
{
    m_observer.epoll_handleclose(fd);
}

void MyObserver::epoll_handleerr(int fd)
{
    m_observer.epoll_handleerr(fd);
}

//MyEvent实现
MyEvent::MyEvent(size_t event_max)
{
    m_epollfd = epoll_create(event_max);
    if(m_epollfd < 0)
        Logger::log(Logger::All, "epoll create failed.");

	Logger::log(Logger::Console, "Epoll create successfully");

    m_threadpool = ThreadPool::ThreadPoolProxy::instance();

    m_thread = std::thread(&MyEvent::eventwait, this);
    m_thread.detach();
}

MyEvent::~MyEvent()
{
    if(m_thread.joinable())
        m_thread.join();

    if(m_epollfd > 0)
        close(m_epollfd);
}

int MyEvent::register_event(int fd, Handler *handle, EventType type)
{
    if(fd == INVALID_SOCKET || handle == nullptr || m_epollfd == -1)
        return -1;

    struct epoll_event event;
    event.data.fd = fd;
    event.events = type;

    Exist ret = isExist(fd, type, handle);
	//std::cout << "[debug] fd is exist:" << ret << std::endl;
    if(ret == Existed)
        return 0;

    record(fd, type, handle);
    if(ret == HandleModify)
        return 0;

    int opt;
    if(ret == TypeModify || ret == Modify)
        opt = EPOLL_CTL_MOD;
    else if(ret == NotExist)
        opt = EPOLL_CTL_ADD;

    if(epoll_ctl(m_epollfd, opt, fd, &event) < 0)
    {
        detach(fd, true);
        return -1;
    }

    return 0;
}

int MyEvent::unregister_event(int fd)
{
    if(epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr) < 0)
        return -1;

    return detach(fd);
}

int MyEvent::shutdown_event(int fd)
{
    return ::shutdown(fd, SHUT_WR);
}

MyEvent::Exist MyEvent::isExist(int fd, EventType type, Handler *handler)
{
    std::unique_lock<std::mutex> lck(m_eventobserver_mutex);
    EventObserver_t::iterator iter = m_eventobserver.find(fd);
    if(iter == m_eventobserver.end())
        return NotExist;

    MyObserver &observer = *(iter->second);
    EventType oldType = observer.getEventType();
    const Observer *oldHandler = observer.getHandle();
    if(oldType != type && oldHandler != handler)
        return Modify;
    else if(oldType != type)
        return TypeModify;
    else if(oldHandler != handler)
        return HandleModify;

    return Existed;
}

int MyEvent::record(int fd, EventType type, Handler *handler)
{
	std::unique_lock<std::mutex> lck(m_eventobserver_mutex);

    MyObserver *observer = new MyObserver(*handler, type);

    m_eventobserver[fd] = observer;
    return 0;
}

int MyEvent::detach(int fd, bool release)
{
    std::unique_lock<std::mutex> lck(m_eventobserver_mutex);
    EventObserver_t::iterator iter = m_eventobserver.find(fd);
    if(iter == m_eventobserver.end())
        return -1;

    if(release)
        iter->second->release();

    m_eventobserver.erase(iter);
    return 0;
}

MyObserver* MyEvent::getObserver(int fd)
{
    std::unique_lock<std::mutex> lck(m_eventobserver_mutex);
    EventObserver_t::iterator iter = m_eventobserver.find(fd);
    if(iter == m_eventobserver.end())
        return nullptr;

    MyObserver *observer = m_eventobserver[fd];
    observer->increaseRef();
    return observer;
}

int MyEvent::pushTask(int fd, EventType type)
{
    std::unique_lock<std::mutex> lck(m_eventtype_mutex);
    EventType_t::iterator iter = m_eventtype.find(fd);
    if(iter == m_eventtype.end())
    {
        m_eventtype[fd] = type;
        return 0;
    }

    iter->second = (EventType)(iter->second | type);
    return 0;
}

int MyEvent::popTask(int &fd, EventType &type)
{
    std::unique_lock<std::mutex> lck(m_eventtype_mutex);
    EventType_t::iterator iter = m_eventtype.begin();
    if(iter == m_eventtype.end())
        return -1;

    fd = iter->first;
    type = iter->second;
    m_eventtype.erase(iter);
    return 0;
}

int MyEvent::cleartask(int fd)
{
    std::unique_lock<std::mutex> lck(m_eventtype_mutex);
    if(fd == -1)
    {
        m_eventtype.clear();
        return 0;
    }
    else if(fd > 0)
    {
        EventType_t::iterator iter = m_eventtype.find(fd);
        if(iter == m_eventtype.end())
        {
            return -1;
        }
        m_eventtype.erase(iter);
        return 0;
    }
    return -1;
}

size_t MyEvent::taskSize()
{
    std::unique_lock<std::mutex> lck(m_eventtype_mutex);
    return m_eventtype.size();
}

void MyEvent::taskHandle()
{
    int fd = 0;
    EventType type;
    if(popTask(fd, type) < 0)
        return;

    MyObserver *observer = getObserver(fd);
    if(observer == nullptr)
        return;

    if(type & ECLOSE)
    {
        cleartask(fd);
        observer->reduceRef();
    }
    else if(type & EERR)
        observer->epoll_handleerr(fd);
    else if(type & EIN)
        observer->epoll_handlein(fd);
    else if(type & EOUT)
        observer->epoll_handleout(fd);

    if(observer->countRef())
    {
        unregister_event(fd);
        observer->epoll_handleclose(fd);
        observer->release();
    }
}

void* MyEvent::eventwait(void *args)
{
    MyEvent &event = *(MyEvent*)args;
    if(event.m_epollfd == -1)
        return nullptr;

    while(true)
    {
        Logger::log(Logger::Console, "epoll wait is running...");
        int nEvent = epoll_wait(event.m_epollfd, &event.m_event[0], EventLen, -1);
        if(nEvent < 0 && errno == EINTR)
        {
            Logger::log(Logger::Console, "epoll is broken.");
            break;
        }

        for(int index = 0; index < nEvent; ++index)
        {
            int fd = event.m_event[index].data.fd;
            EventType type = static_cast<EventType>(event.m_event[index].events);

            if(event.pushTask(fd, type) == 0)
            {
                event.m_threadpool->pushTask(&event);
            }
        }
    }
}

EventProxy* EventProxy::instance()
{
    static EventProxy *eventproxy = nullptr;
    if(eventproxy == nullptr)
    {
        eventproxy = new EventProxy(EVENT_MAX);
    }
    return eventproxy;
}

EventProxy::EventProxy(int event_max)
{
    m_event = new MyEvent(event_max);
}

EventProxy::~EventProxy()
{
    if(m_event)
        delete m_event;
}

int EventProxy::register_event(int fd, Handler *handle, EventType type)
{
    m_event->register_event(fd, handle, type);
}

int EventProxy::register_event(SocketInterface::SocketDest &socket, Handler *handle, EventType type)
{
    register_event(socket.fd(), handle, type);
}

int EventProxy::shutdown_event(int fd)
{
    m_event->shutdown_event(fd);
}

int EventProxy::shutdown_event(SocketInterface::SocketDest &socket)
{
    m_event->shutdown_event(socket.fd());
}

} // namespace Event
