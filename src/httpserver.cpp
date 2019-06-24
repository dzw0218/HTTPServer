#include "socketepoll.h"
#include <iostream>
#include <signal.h>

void handle_sigpipe()
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, nullptr))
		return;
}

int main()
{
	return 0;
}
