#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

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
#include <iomanip>
#include <fcntl.h>
#include <memory>
#include <bits/stdc++.h>
#include <vector>
#include<time.h>



#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


#define INVALID_JOB_ID -1
#define UNSPECIFIED_JOB_ID -1
#define FG_JOB_ID 0
#define NO_FG_JOB -1

#define JOBS_LIST_EMPTY 1
#define JOB_ID_NOT_EXIST 2
#define NO_STOPPED_JOBS 3
#define JOB_ALREADY_RUNNING 4
#define FAILED_SYSCALL 5


#define STOPPED 1
#define CONTINUED 2
#define MOVED 3







class JobsList;
class SmallShell;

/******** BASE CLASSES ********/
class Command {
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
	if (*it == '-') ++it;
	if (it == s.end()) return false;
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }
    bool is_positive_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    //virtual void prepare();
    //virtual void cleanup();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
private:
    bool Complex_Command= false;
    bool Is_back_ground= false;
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

class PipeCommand : public Command {
    // TODO: Add your data members
    Command * Fisrtpipe;
    Command * Seconedpipe;
public:
    PipeCommand(const char* cmd_line) : Command::Command(cmd_line)
    {};
    virtual ~PipeCommand() {}
    void execute() override;
};

class SpecialPipeCommand : public Command {
    // TODO: Add your data members
    Command * Fisrtpipe;
    Command * Seconedpipe;
public:
    SpecialPipeCommand(const char* cmd_line) : Command::Command(cmd_line)
    {};
    virtual ~SpecialPipeCommand() {}
    void execute() override;
};



class RedirectionCommandOverRide : public Command {
    Command * FisrtCommand;
    std::string path;
    // TODO: Add your data members
public:
    explicit RedirectionCommandOverRide(const char* cmd_line): Command::Command(cmd_line)
    {};
    virtual ~RedirectionCommandOverRide() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class Appened : public Command {
    Command * FirstCommand;
    std::string path;
    // TODO: Add your data members
public:
    explicit Appened(const char* cmd_line): Command::Command(cmd_line)
    {};
    virtual ~Appened() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
/*

*/

/******** BUILTIN COMMANDS CLASSES ********/
class ChangePromptCommand : public BuiltInCommand {
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


class SetcoreCommand : public BuiltInCommand {
    int core_num;
    int job_num;
public:
    SetcoreCommand(const char* cmd_line);
    virtual ~SetcoreCommand() {}
    void execute() override;
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
    bool negative_job_id;
public:
    ForegroundCommand(const char* cmd_line);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
private:
    bool invalid_args;
    bool negative_job_id;
public:
    BackgroundCommand(const char* cmd_line);
    virtual ~BackgroundCommand() {}
    void execute() override;
};


class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line) {};
    virtual ~QuitCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
    virtual ~KillCommand() {}
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
    /**** prompt ****/
    std::string getPrompt();
    void setPrompt(std::string new_prompt);
    void resetPrompt();
    /**** pid ****/
    pid_t getPid() { return pid; };
    /**** cwd ****/
    std::string getCwd();
    void setCwd(std::string new_path);
    std::string getOldPwd() { return old_pwd; }
    /**** error handling ****/
    void syscallErrorHandler(std::string syscall_name);

};



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
	bool is_fg;
      public:
	JobEntry() : job_id(INVALID_JOB_ID) {}; // invalid
	JobEntry(JobEntry const&) = default;
	JobEntry& operator=(JobEntry const&) = default;
	JobEntry(int job_id, std::string cmd_line, int pid, int starttime, time_t past_running_time, bool is_stopped, bool is_fg)
		: job_id(job_id), cmd_line(cmd_line), pid(pid), starttime(starttime), past_running_time(past_running_time), is_stopped(is_stopped), is_fg(is_fg) {}
	~JobEntry() = default;
	/**** print & kill methods ****/
	void printJobEntry();
	void killJob();
	void continueJob();
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
	bool isFg() { return is_fg; }
	/**** set methods ****/
	void setFg(bool fg) { is_fg = fg; }
	void setStopped(bool stp) { is_stopped = stp; }
	void resetStarttime() { time(&starttime); }
	void addToPastRunningTime(int time) { past_running_time += time; }
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
  /**** jobs list functions ****/
  void addJob(std::string cmd_line, int pid, bool is_stopped, bool is_fg);
  void removeJobById(int jobId);
  void removeFinishedJobs();
  int getNumJobs();
  pid_t getPidByJobId(int jobId);
  JobEntry* getJobById(int jobId);
  JobEntry* getFgJob();
  int getMaxStoppedJobId();
  void printJobsList();
  /**** fg & bg manipulations ****/
  bool bringToFg(int job_id, int* err);
  bool resumeJobInBg(int job_id, int* err);

  pid_t stopFgProc();
  pid_t killFgProc();

  void killAllJobs();
  bool sendSignal(int signum, int job_id);
};




#endif //SMASH_COMMAND_H_

