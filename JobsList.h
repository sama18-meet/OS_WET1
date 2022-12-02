#ifndef JOBS_LIST_H_
#define JOBS_LIST_H_

#include <vector>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include<list>
#include<signal.h>
#include <string>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <fcntl.h>
#include <memory>
#include <bits/stdc++.h>
#include<time.h>

#include "Defines.h"


class JobsList {
 public:
  class JobEntry {
      public:
	int job_id;
	std::string cmd_line;
	pid_t pid;
	time_t starttime;
	time_t past_running_time;
	bool is_stopped;
      public:
	JobEntry() : job_id(INVALID_JOB_ID) {}; // invalid
	JobEntry(JobEntry const&) = default;
	JobEntry& operator=(JobEntry const&) = default;
	JobEntry(int job_id, std::string cmd_line, int pid, int starttime, time_t past_running_time = 0, bool is_stopped = false)
		: job_id(job_id), cmd_line(cmd_line), pid(pid), starttime(starttime), past_running_time(past_running_time), is_stopped(is_stopped) {}
	~JobEntry() = default;
	/**** print & kill methods ****/
	void printJobEntry();
	void killJob();
	/**** operator overloading ****/
	bool operator<(const JobEntry& j) const { return job_id < j.job_id; }
	bool operator>(const JobEntry& j) const { return job_id > j.job_id; }
	bool operator==(const JobEntry& j) const { return job_id == j.job_id; }
	/**** get methods ****/
	int getJobId() { return job_id; }
	std::string getCmdLine() { return cmd_line; }
	int getPid() { return pid; }
	time_t getStarttime() { return starttime; }
	time_t getPastRunningTime() { return past_running_time; }
	bool isStopped() { return is_stopped; }
	/**** set methods ****/
	void removeStoppedFlag() { is_stopped = false; }
	void setPastRunningTime(time_t time) { past_running_time = time; }
	void resetStarttime() { time(&starttime); }
  };
 public:
  std::vector<JobEntry> all_jobs;
  int max_jobid;
  int max_stopped_job_id;
  JobEntry fg_job;
 private:
  JobsList() : all_jobs(std::vector<JobEntry>()), max_jobid(0), max_stopped_job_id(0), fg_job(JobEntry()) {}
 public:
  JobsList(const JobsList&) = delete;
  void operator=(const JobsList&) = delete;
  ~JobsList() = default;
  static JobsList& getInstance() {
	static JobsList instance;
	return instance;
  }
  /**** jobs list functions ****/
  void addJob(std::string cmd_line, int pid, time_t past_running_time, bool is_stopped = false);
  void removeJobById(int jobId);
  void removeFinishedJobs();
  int getNumJobs();
  pid_t getPidByJobId(int jobId);
  JobEntry* getJobById(int jobId);
  int getMaxStoppedJobId();
  void printJobsList();
  /**** fg & bg manipulations ****/
  bool bringToFg(int job_id, int* err);
  bool resumeJobInBg(int job_id, int* err);
  pid_t stopFgProc();
  void setFgProc(pid_t pid, std::string cmd_line, time_t past_running_time);
  void rmFgProc() { fg_job = JobEntry(); };
  void killAllJobs();
};


#endif //JOBS_LIST_H_
