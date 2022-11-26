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
  char* args[COMMAND_MAX_ARGS + 1];
  char *command_line;
  int num_args;
  bool Is_back_ground;

 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};
/*

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};
 */
class ExternalCommand : public Command {
private:
    bool Complex_Command= false;
    bool Sleep_Command= false;
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
class ChangePromptCommand : public Command { // need to change this to inherit from Command instead of BuiltInCommand
 private:
  std::string new_prompt;
 public:
  ChangePromptCommand(const char* cmd_line);
  ~ChangePromptCommand() = default;
  void execute() override;
};


class ShowPidCommand : public Command {
 public:
  ShowPidCommand(const char* cmd_line) : Command(cmd_line) {};
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

class GetCurrDirCommand : public Command {
 public:
  GetCurrDirCommand(const char* cmd_line) : Command(cmd_line) {};
  virtual ~GetCurrDirCommand() = default;
  void execute() override;
};


class ChangeDirCommand : public Command {
 public:
  ChangeDirCommand(const char* cmd_line) : Command(cmd_line) {};
  virtual ~ChangeDirCommand() = default;
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


class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
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
