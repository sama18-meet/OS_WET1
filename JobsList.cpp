#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <limits.h>
#include <algorithm>
#include <assert.h>

#include "Commands.h"
#include "JobsList.h"



///////////////////////////////////////////////////////////
////// JOB ENTRY
///////////////////////////////////////////////////////////

void JobsList::JobEntry::printJobEntry() {
	std::cout << "[" << job_id << "] " << cmd_line << " : " << pid << " ";
	if (!is_stopped) {
		time_t now;
		time(&now);
		time_t seconds_elapsed = difftime(now, starttime);
		time_t total_runtime = seconds_elapsed + past_running_time;
		std::cout << total_runtime << " secs" << std::endl;
	}
	else {
		std::cout << past_running_time << " secs (stopped)" << std::endl;
	}
}

void JobsList::JobEntry::killJob() {
	if (kill(pid, SIGKILL) == -1) {
		SmallShell::getInstance().syscallErrorHandler("kill");
	}
	else {
		std::cout << pid << ": " << cmd_line << std::endl;
	}
}

void JobsList::JobEntry::continueJob() {
	resetStarttime();
	setStopped(false);
	if (kill(pid, SIGCONT) == -1) {
		SmallShell::getInstance().syscallErrorHandler("kill");
	}
}


///////////////////////////////////////////////////////////
////// JOBS LIST
///////////////////////////////////////////////////////////


/**** jobs list functions ****/

void JobsList::addJob(std::string cmd_line, int pid, bool is_stopped, bool is_fg) {
	removeFinishedJobs();
	int job_id = max_jobid + 1;
	time_t starttime;
	time(&starttime); 
	JobEntry j(job_id, cmd_line, pid, starttime, 0, is_stopped, is_fg);
	all_jobs.push_back(j);
	max_jobid++;
}

void JobsList::removeJobById(int job_id) {
	std::vector<JobEntry>::iterator it = all_jobs.begin();
	int second_max_job_id = INVALID_JOB_ID;
	while(it != all_jobs.end()) {
	    if((*it).getJobId() == job_id) {
		it = all_jobs.erase(it);
	    }
	    else {
		second_max_job_id = (*it).getJobId() > second_max_job_id ? (*it).getJobId() : second_max_job_id;
		++it;
	    }
	}
	max_jobid = second_max_job_id == INVALID_JOB_ID ? 0 : second_max_job_id;
}

void JobsList::removeFinishedJobs() { // assumption: stopped jobs cannot be finished
	for (JobEntry j : all_jobs) {
		pid_t return_pid = waitpid(j.pid, nullptr, WNOHANG);
		if (return_pid == -1) { // no process with this pid (process terminated)
			removeJobById(j.job_id);
		} else if (return_pid == 0) {
			/* child is still running */
			continue;
		} else if (return_pid == j.pid) {
			/* child is finished. */
			removeJobById(j.job_id);
		}
	}
}

int JobsList::getNumJobs() {
	return all_jobs.size();
}

pid_t JobsList::getPidByJobId(int job_id) {
	for (JobEntry j : all_jobs) {
		if (j.getJobId() == job_id) {
			return j.getPid();
		}
	}
	return INVALID_JOB_ID;
}

JobsList::JobEntry* JobsList::getJobById(int job_id) {
	std::vector<JobEntry>::iterator it = all_jobs.begin();
	while(it != all_jobs.end()) {
	    if((*it).getJobId() == job_id) {
		return &(*it);
	    }
	    else {
		++it;
	    }
	}
	return nullptr;
}

JobsList::JobEntry* JobsList::getFgJob() {
	std::vector<JobEntry>::iterator it = all_jobs.begin();
	while(it != all_jobs.end()) {
	    if((*it).isFg()) {
		return &(*it);
	    }
	    else {
		++it;
	    }
	}
	return nullptr;
}

int JobsList::getMaxStoppedJobId() {
	int max = INVALID_JOB_ID;
	for (JobEntry j : all_jobs) {
		if (j.isStopped() & (j.getJobId() > max)) {
			max = j.getJobId();
		}
	}
	return max;
}

void JobsList::printJobsList() {
	removeFinishedJobs();
	std::sort(all_jobs.begin(), all_jobs.end());
	for (JobEntry j : all_jobs)
		j.printJobEntry();
}


/**** fg & bg manipulations ****/
bool JobsList::bringToFg(int job_id, int* err) {
	removeFinishedJobs();
	if (job_id == UNSPECIFIED_JOB_ID) {
		if (max_jobid == 0) {
			*err = JOBS_LIST_EMPTY;
			return false;
		}
		job_id = max_jobid;
	}
	pid_t pid = getPidByJobId(job_id);
	if (pid == INVALID_JOB_ID) { // no job with job_pid in jobsList
		*err = JOB_ID_NOT_EXIST;
		return false;
	}
	JobEntry* job = getJobById(job_id);  assert(job != nullptr); // job exists
	job->setFg(true);
	std::cout << job->getCmdLine() << " : " << job->getPid() << std::endl;
	if (job->isStopped()) {
		job->continueJob();
	}
	if (waitpid(pid, nullptr, WUNTRACED) == -1) {
		SmallShell::getInstance().syscallErrorHandler("waitpid");
		*err = FAILED_SYSCALL;
		return false;
	}
	removeFinishedJobs();
	return true;
}

bool JobsList::resumeJobInBg(int job_id, int* err) {
	if (job_id == UNSPECIFIED_JOB_ID) {
		job_id = getMaxStoppedJobId();
		if (job_id == INVALID_JOB_ID) {
			*err = NO_STOPPED_JOBS;
			return false;
		}
	}
	JobEntry* job = getJobById(job_id);
	if (job == nullptr) {
		*err = JOB_ID_NOT_EXIST;
		return false;
	}
	if (job->isStopped() == false) {
		*err = JOB_ALREADY_RUNNING;
		return false;
	}
	std::cout << job->getCmdLine() << " : " << job->getPid() << std::endl;
	job->continueJob();
	return true;
}

pid_t JobsList::stopFgProc() {
	removeFinishedJobs();
	JobEntry* fg_job = getFgJob();
	if (fg_job == nullptr)
		return NO_FG_JOB;
	pid_t pid = fg_job->getPid();
	time_t now;
	time(&now);
	fg_job->addToPastRunningTime(difftime(now, fg_job->getStarttime()));
	if (kill(pid, SIGSTOP) == -1) {
		SmallShell::getInstance().syscallErrorHandler("kill");
		return -1;
	}
	fg_job->setFg(false);
	fg_job->setStopped(true);
	return pid;
}

pid_t JobsList::killFgProc() {
	removeFinishedJobs();
	JobEntry* fg_job = getFgJob();
	if (fg_job == nullptr)
		return NO_FG_JOB;
	pid_t pid = fg_job->getPid();
	if (kill(pid, SIGKILL) == -1) {
		SmallShell::getInstance().syscallErrorHandler("kill");
		return -1;
	}
	removeJobById(fg_job->getJobId());
	return pid;
}


void JobsList::killAllJobs() {
	removeFinishedJobs();
	for (JobEntry j : all_jobs) {
		j.killJob();
	}
}



bool JobsList::sendSignal(int signum, int job_id) {
	pid_t pid = getPidByJobId(job_id);
	if (pid == INVALID_JOB_ID) {
		return false;
	}
	if (kill(pid, signum) == -1) {
		SmallShell::getInstance().syscallErrorHandler("kill");
	}
	else {
		std::cout << "signal number " << signum << " was sent to pid " << pid << std::endl;
	}
	return true;
}
