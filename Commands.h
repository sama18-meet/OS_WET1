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
#include <sys/wait.h>
#include <iomanip>
#include <fcntl.h>
#include <memory>
#include <bits/stdc++.h>


#include "Defines.h"

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
public:
    ForegroundCommand(const char* cmd_line);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
private:
    bool invalid_args;
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



class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
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


#endif //SMASH_COMMAND_H_

