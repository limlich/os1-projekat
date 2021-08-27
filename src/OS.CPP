#include "os.h"
#include "pcb.h"
#include "main.h"
#include "idle.h"
#include "thread.h"
#include "context.h"

PCB * volatile 	OS::runningPCB = NULL;
MainPCB * 		OS::mainPCB = NULL;
IdlePCB * 		OS::idlePCB = NULL;
SystemThread * 	OS::sysThread_ = NULL;
int 			OS::exitCode_ = 0;

int userMain(int argc, char* argv[]);

class SystemThread : public Thread
{
public:
	SystemThread(int argc, char* argv[])
		: Thread(defaultStackSize, defaultTimeSlice), argc_(argc), argv_(argv)
	{
		registerHandler(0, PCB::signalHandler0); // redistributed to all other threads
	}
	virtual ~SystemThread()
	{
		waitToComplete();
	}

	void run()
	{
		OS::exitCode_ = userMain(argc_, argv_);
	}

private:
	int argc_;
	char** argv_;
};

int OS::sysMain(int argc, char* argv[])
{
	mainPCB = new MainPCB();
	runningPCB = mainPCB;
	idlePCB = new IdlePCB();

	sysThread_ = new SystemThread(argc, argv);

	Context::init();

	sysThread_->start();
	delete sysThread_;

	Context::restore();

	delete mainPCB;
	delete idlePCB;

	return exitCode_;
}
