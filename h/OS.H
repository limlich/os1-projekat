#ifndef _os_h_
#define _os_h_

class PCB;
class MainPCB;
class IdlePCB;
class SystemThread;

class OS
{
public:
	static int sysMain(int argc, char* argv[]);

	static PCB * volatile runningPCB;
	static MainPCB * mainPCB;
	static IdlePCB * idlePCB;

private:
	friend class SystemThread;
	static SystemThread * sysThread_;
	static int exitCode_;
};

#endif
