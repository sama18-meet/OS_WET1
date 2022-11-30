#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>
#include <iostream>
#include <vector>
#include<time.h>
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

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


class Command {
// TODO: Add your data members
 protected:
  char* cmd_line;
  char* args[COMMAND_MAX_ARGS + 1];
  int num_args;

 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  char* getCmdLine() { return cmd_line; }
  bool is_number(const std::string& s)
  {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
  }

  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() = default;
};
class ExternalCommand : public Command {
private:
    bool Complex_Command= false;
    bool Sleep_Command= false;
    bool Is_back_ground;
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
    void setComplexCommand(bool complexCommand) {
        Complex_Command = complexCommand;
    }

    bool isComplexCommand() const {
        return Complex_Command;
    }

};

/*
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};
*/

class SmallShell;
class ChangePromptCommand : public BuiltInCommand { // need to change this to inherit from Command instead of BuiltInCommand
 private:
  char* new_prompt;
 public:
  ChangePromptCommand(const char* cmd_line);
  ~ChangePromptCommand() = default;
  void execute() override;
};


class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
  virtual ~GetCurrDirCommand() = default;
  void execute() override;
};


class ChangeDirCommand : public BuiltInCommand {
 public:
  ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
  virtual ~ChangeDirCommand() = default;
  void execute() override;
};




class JobsList {
 public:
  class JobEntry {
      public:
	int job_id;
	std::string cmd_line;
	pid_t pid;
	time_t starttime;
	bool is_stopped;
      public:
	JobEntry() : job_id(-1) {}; // invalid
	JobEntry(JobEntry const&) = default;
	JobEntry& operator=(JobEntry const&) = default;
	JobEntry(int job_id, std::string cmd_line, int pid, int starttime, bool is_stopped = false)
		: job_id(job_id), cmd_line(cmd_line), pid(pid), starttime(starttime), is_stopped(is_stopped) {}
	~JobEntry() = default;
	void printJobEntry();
	bool operator<(const JobEntry& j) const { return job_id < j.job_id; }
	//bool operator>(const JobEntry& j) const { return job_id > j.job_id; }
	//bool operator==(const JobEntry& j) const { return job_id == j.job_id; }
	int getJobId() { return job_id; }
	int getPid() { return pid; }
	bool isStopped() { return is_stopped; }
	std::string getCmdLine() { return cmd_line; }
	void removeStoppedFlag() { is_stopped = false; }


  };
 public:
  std::vector<JobEntry> all_jobs;
  int max_jobid;
  int max_stopped_job_id;
 private:
  JobsList() : all_jobs(std::vector<JobEntry>()), max_jobid(0), max_stopped_job_id(0) {}
 public:
  JobsList(const JobsList&) = delete;
  void operator=(const JobsList&) = delete;
  ~JobsList() = default;
  static JobsList& getInstance() {
	static JobsList instance;
	return instance;
  }
  void addJob(std::string cmd_line, int pid, bool is_stopped = false);
  void printJobsList();
  //void killAllJobs();
  void removeFinishedJobs();
  pid_t getPidByJobId(int jobId);
  JobEntry getJobById(int jobId);
  void removeJobById(int jobId);
  //JobEntry& getLastJob(int* lastJobId);
  //JobEntry& getLastStoppedJob(int *jobId);
  bool bringToFg(int job_id, int* err);
  bool resumeJobInBg(int job_id, int* err);
  int getMaxStoppedJobId();
};

class JobsCommand : public BuiltInCommand {
 public:
  JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 private:
  bool invalid_args;
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 public:
  BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
  virtual ~BackgroundCommand() {}
  void execute() override;
};


/*
class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};



class TimeoutCommand : public BuiltInCommand {
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class FareCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  FareCommand(const char* cmd_line);
  virtual ~FareCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};
*/

class SmallShell { // singelton
 private:
  std::string prompt;
  pid_t pid;
  std::string old_pwd;
 private:
  SmallShell();
 public:
  SmallShell(SmallShell const&)      = delete;
  void operator=(SmallShell const&)  = delete;
  ~SmallShell() = default;
  static SmallShell& getInstance()
  {
    static SmallShell instance;
    return instance;
  }
  Command *CreateCommand(const char* cmd_line);
  void executeCommand(const char* cmd_line);
  // prompt
  std::string getPrompt();
  void setPrompt(std::string new_prompt);
  void resetPrompt();
  // pid
  pid_t getPid() { return pid; };
  // cwd
  std::string getCwd();
  void setCwd(std::string new_path);
  std::string getOldPwd() { return old_pwd; }
  // error handling
  void syscallErrorHandler(std::string syscall_name);
  
};

#endif //SMASH_COMMAND_H_
