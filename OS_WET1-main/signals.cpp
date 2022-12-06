#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	std::cout << "smash: got ctrl-Z" << std::endl;
	pid_t fg_pid = JobsList::getInstance().stopFgProc();
	if (fg_pid == -1) return;
	std::cout << "smash: process " << fg_pid << " was stopped" << std::endl;
}

void ctrlCHandler(int sig_num) {
	std::cout << "smash: got ctrl-C" << std::endl;
	pid_t fg_pid = JobsList::getInstance().killFgProc();
	if (fg_pid == -1) return;
	std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

