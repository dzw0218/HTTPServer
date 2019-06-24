#include "mutex.h"

namespace Pthread
{

MyMutex::MyMutex()
{
	pthread_mutex_init(&m_mutex, nullptr);
}

}
