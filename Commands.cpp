#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <ctime>
#include <sys/stat.h>
#include "Commands.h"
#include "JobsList.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s) // remove leading whitespace
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) // remove trailing whitespace
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) // remove leading and trailing whitespace
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

bool ContainsWildcards(string buffer)
{
    if (buffer=="*" || buffer=="?"){
        return true;
    }
    for (unsigned int i = 0; i < buffer.size(); ++i) {
        if (buffer[i]=='*' || buffer[i]=='?'){
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// Command methods implementation
///////////////////////////////////////////////////////////



Command::Command(const char* cmd_line) {
	this->cmd_line = (char*)cmd_line;
	this->num_args = _parseCommandLine(cmd_line, this->args);
}

Command::~Command() {
	for (int i=0; i<COMMAND_MAX_ARGS+1 ; ++i) {
		free(this->args[i]);
	}
}


BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line) {
	_removeBackgroundSign(this->cmd_line);
	_removeBackgroundSign(this->args[num_args-1]);
}

///////////////////////////////////////////////////////////
// ChangePromptCommand methods implementation


ChangePromptCommand::ChangePromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
	new_prompt = args[1];
}

void ChangePromptCommand::execute() {
	SmallShell& smash = SmallShell::getInstance();
	if (num_args == 1) {
		smash.resetPrompt();
	}
	else {
		smash.setPrompt(new_prompt);
	}
}


///////////////////////////////////////////////////////////
// ShowPidCommand methods implementation



void ShowPidCommand::execute() {
	SmallShell& smash = SmallShell::getInstance();
	std::cout << "smash pid is " << smash.getPid() << std::endl;
}

///////////////////////////////////////////////////////////
// GetCurrDir methods implementation


void GetCurrDirCommand::execute() {
	std::cout << SmallShell::getInstance().getCwd() << std::endl;
}


///////////////////////////////////////////////////////////
// ChangeCurrDir methods implementation


void ChangeDirCommand::execute() {
	SmallShell& smash = SmallShell::getInstance();
	if (num_args != 2) {
		std::cerr << "smash error: cd: too many arguments" << std::endl;
		return;
	}
	if (string(args[1]).compare("-") == 0) {
		if (smash.getOldPwd().compare("") == 0) {
			std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
			return;
		}
		smash.setCwd(smash.getOldPwd());
		return;
	}
	smash.setCwd(string(args[1]));
}



///////////////////////////////////////////////
// ExternalCommand implementation
///////////////////////////////////////////////
ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {
	string temp = string (args[num_args-1]);
	if (temp == "&" || temp.back() =='&')
	{
		this->Is_back_ground= true;
	}
	this->cmd_line = new char[string(cmd_line).length() + 1];
	strcpy(this->cmd_line, cmd_line);
	std::stringstream ss(cmd_line);
	string buf;
	while (ss >> buf) {
        if (ContainsWildcards(buf)) {
		this->setComplexCommand(true);
        }
    }
}

void ExternalCommand::execute() {
    _removeBackgroundSign(this->cmd_line); ///check if command line is corrupted
    pid_t pid = fork();
    if (pid < 0) //that is for error
    {
        std::cerr<<"smash error: fork failed"<<std::endl;
        return;
    }
    else if (pid==0) //the son runs
    {
        if (this->Complex_Command ) // use bash
        {
            setpgrp();

            const char *path_args[] = {"/bin/bash", "-c", this->cmd_line, nullptr};
            if (execv(path_args[0], (char **) path_args) == -1) {
                perror("smash error: execv failed");
                return;
            }
        } else // using exec v
        {
            setpgrp();
            std::stringstream ss(this->cmd_line);
            string buff;
            string args;
            string path;

            ss >> path;

            while (ss >> buff) {
                args += buff;
                args += " ";
            }
            if (args.size()>1)
            {
                args.pop_back();
            }
            const char *filepath = path.c_str();
            if (args == "") {
                const char *path_args[] = {path.c_str(), nullptr, nullptr};
                if (execvp(filepath, (char **) path_args) == -1) {
                    perror("smash error: execvp failed");
                    return;
                }
            } else {
                const char *path_args[] = {path.c_str(), args.c_str(), nullptr};
                if (execvp(filepath, (char **) path_args) == -1) {
                    perror("smash error: execvp failed");
                    return;
                }
            }
        }
    }
    else if (pid>0) ///that is the father, should wait for son and add to jop list if its a back ground
    {
if (!this->Is_back_ground) ///father should wait
        {
	    JobsList::getInstance().setFgProc(pid, cmd_line, 0);
            if (waitpid(pid, nullptr, WUNTRACED) == -1)
            {
                perror("smash error: waitpid failed");
                return;
            }
	    JobsList::getInstance().rmFgProc();
        }
        else
        {
		JobsList::getInstance().addJob(this->cmd_line, pid, 0, false);
		return;
        }
    }
}



void JobsCommand::execute() {
	JobsList::getInstance().printJobsList();
}

ForegroundCommand::ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
	invalid_args = false;
	if (num_args > 2)
		invalid_args = true;
	if (num_args == 2) {
		if (!is_number(args[1])) {
			invalid_args = true;
		}
	}
}

void ForegroundCommand::execute() {
	if (invalid_args) {
		std::cerr << "smash error: fg: invalid arguments" << std::endl;
	}
	int job_id = num_args == 2 ? std::stoi(args[1]) : UNSPECIFIED_JOB_ID;
	int err;
	bool success = JobsList::getInstance().bringToFg(job_id, &err);
	if (!success) {
		if (err == JOBS_LIST_EMPTY)
			std::cerr << "smash error: fg: jobs list is empty" << std::endl;
		else if (err == JOB_ID_NOT_EXIST)
			std::cerr << "smash error: fg: job-id " << job_id << "does not exist" << std::endl;
	}

}

BackgroundCommand::BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
	invalid_args = false;
	if (num_args > 2)
		invalid_args = true;
	if (num_args == 2) {
		if (!is_number(args[1])) {
			invalid_args = true;
		}
	}
}




void BackgroundCommand::execute() {
	if (invalid_args) {
		std::cerr << "smash error: bg: invalid arguments" << std::endl;
	}
	int job_id = num_args == 2 ? std::stoi(args[1]) : UNSPECIFIED_JOB_ID;
	int err;
	bool success = JobsList::getInstance().resumeJobInBg(job_id, &err);
	if (!success) {
		if (err == NO_STOPPED_JOBS)
			std::cout << "smash error: bg: there is no stopped jobs to resume" << std::endl;
		if (err == JOB_ID_NOT_EXIST)
			std::cout << "smash error: bg: job-id " << job_id << " does not exist" << std::endl;
		if (err == JOB_ALREADY_RUNNING)
			std::cout << "smash error: bg: job-id " << job_id << " is already running in the background" << std::endl;
	}
}


///////////////////////////////////////////////////////////
// SmallShell methods implementation
///////////////////////////////////////////////////////////

SmallShell::SmallShell() {
	prompt = "smash> ";
	pid = getpid();
	old_pwd = "";
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));

    if (cmd_s=="")
    {
        return nullptr;
    }

    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" "));


    std::stringstream ss ((_trim(string(cmd_line))));
    string buffer;
    while (ss >> buffer)
    {
        if (buffer=="|")
        {
            return new PipeCommand(cmd_line);
        }
        else if (buffer=="|&")
        {
            return new SpecialPipeCommand(cmd_line);
        }

        else if (buffer == ">") {
            return new RedirectionCommandOverRide(cmd_line);
        }
        else if (buffer == ">>"){
            return new Appened(cmd_line);
        }
    }


  if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&")==0) {
    return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0 ) {
    return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("setcore") == 0 || firstWord.compare("setcore&") == 0 ) {
      return new SetcoreCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0) {
    return new ForegroundCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0) {
    return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0 ) {
      return new QuitCommand(cmd_line);
  }
  else if (firstWord.compare("kill") == 0 || firstWord.compare("kill&") == 0 ) {
      return new KillCommand(cmd_line);
  }

  else {
    return new ExternalCommand(cmd_s.c_str());
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  Command* cmd = CreateCommand(cmd_line);
  if (cmd == nullptr) {
	return;
  }
  JobsList::getInstance().removeFinishedJobs();
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

///// prompt /////

std::string SmallShell::getPrompt() {
	return this->prompt;
}
void SmallShell::setPrompt(std::string new_prompt) {
	this->prompt = new_prompt + "> ";
}

void SmallShell::resetPrompt() {
	prompt = "smash> ";
}

///// cwd /////
std::string SmallShell::getCwd() {
	char buff[PATH_MAX];
	if (getcwd(buff, sizeof(buff)) == nullptr) {
		return "error";
	}
	std::string cwd(buff);
	return cwd;
}

void SmallShell::setCwd(std::string new_path) {
	std::string curr_pwd = getCwd();
	int rc = chdir(new_path.c_str());
	if (rc < 0) {
		syscallErrorHandler("chdir");
		return;
	}
	old_pwd = curr_pwd;

}

///// error handler /////

void SmallShell::syscallErrorHandler(std::string syscall_name) {
	std::string error_msg = "smash error: " + syscall_name + " failed";
	perror(error_msg.c_str());
}


void PipeCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    _removeBackgroundSign(this->cmd_line); ///check if command line is corrupted
    string command1;
    string command2;

    std::stringstream ss(this->cmd_line);
    bool found = false;
    string buffer;
    while (ss >> buffer) {
        if (buffer == "|"){
            found = true;
        }
        else if(!found){
            command1 += buffer + " ";
        }
        else if (buffer != "|" && found){
            command2 += buffer + " ";
        }
    }
    Fisrtpipe = smash.getInstance().CreateCommand(command1.c_str());
    Seconedpipe =smash.getInstance().CreateCommand(command2.c_str());

    int fd[2];
    pipe(fd);
    int pid_1= fork();
    if(pid_1 == -1){
        perror("smash error: fork failed");
    }
    if(pid_1 == 0){
        setpgrp();
        if (dup2(fd[1], 1) == -1) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        Fisrtpipe->execute();
        exit(0);
    }

    int pid_2= fork();

    if(pid_2 == -1){
        perror("smash error: fork failed");
    }
    else if (pid_2 == 0) {
        setpgrp();
        if (dup2(fd[0], 0) == -1) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        Seconedpipe->execute();
        exit(0);
    }
    else if(pid_1 !=0 && pid_2 != 0) {
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
            if (close(fd[1]) == -1) {
                perror("smash error: close failed");
                return;
            }
        if (waitpid(pid_1, nullptr, WNOHANG) == -1)
        {
            perror("smash error: waitpid failed");
            return;
        }
        if (waitpid(pid_2, nullptr, WUNTRACED) == -1)
        {
            perror("smash error: waitpid failed");
            return;
        }
    }
}

void SpecialPipeCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    _removeBackgroundSign(this->cmd_line); ///check if command line is corrupted
    string command1;
    string command2;

    std::stringstream ss(this->cmd_line);
    bool found = false;
    string buffer;
    while (ss >> buffer) {
        if (buffer == "|&"){
            found = true;
        }
        if(!found){
            command1 +=  buffer+" " ;
        }
        else if (buffer != "|&" && found){
            command2 +=buffer+" " ;
        }
    }
    Fisrtpipe = smash.getInstance().CreateCommand(command1.c_str());
    Seconedpipe =smash.getInstance().CreateCommand(command2.c_str());

    int fd[2];
    pipe(fd);
    int pid_1= fork();
    if(pid_1 == -1){
        perror("smash error: fork failed");
    }
    if(pid_1 == 0){
        setpgrp();
        if (dup2(fd[1], 2) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        Fisrtpipe->execute();
        exit(1);
    }
    waitpid(pid_1,NULL,0);

    int pid_2= fork();
    if(pid_2 == -1){
        perror("smash error: fork failed");
    }
    if (pid_2 == 0) {
        setpgrp();
        if (dup2(fd[0], 0) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        Seconedpipe->execute();
        exit(1);
    }
    if(pid_1 !=0 && pid_2 != 0) {
        close(fd[0]);
        close(fd[1]);
        waitpid(pid_2,NULL,0);
    }
}

void RedirectionCommandOverRide::execute() {
    string command;
    std::stringstream ss(cmd_line);
    bool found = false;
    string buffer;
    while (ss >> buffer) {
        if (buffer == ">") {
            found = true;
        }
        if (!found) {
            command += " " + buffer;
        } else if (buffer != ">") {
            path += buffer;
        }
    }
    FisrtCommand = SmallShell::getInstance().CreateCommand(command.c_str());

   ///////here the parameters are on
    JobsList::getInstance().removeFinishedJobs();

    int out = dup(1);
    if (out == -1) {
        perror("smash error: dup failed");
        return;
    }
    if (close(1) == -1) {
        perror("smash error: close failed");
        return;
    }

    int p1 = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0655);

    if (p1 <= -1) {
        perror("smash error: open failed");
        if (dup2(out, 1) == -1) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(out) == -1)
            perror("smash error: close failed");
        return;
    }

    FisrtCommand->execute();

    if (dup2(out, 1) == -1) {
        perror("smash error: dup2 failed");
        return;
    }
    if (close(out) == -1) {
        perror("smash error: close failed");
        return;
    }
}

void Appened::execute() {}

SetcoreCommand::SetcoreCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

    std::stringstream ss(cmd_line);
    std::string buffer;
    stringstream buffer2;

    int i=0;
    while (ss >> buffer)
    {
        if(i==1)
        {
            buffer2 << buffer;
            buffer2 >> this->job_num;
        }
        else if (i==2)
        {
            buffer2 << buffer;
            buffer2 >> this->core_num;
        }
        else {
            i++;
        }
    }
}

void SetcoreCommand::execute() {
    if (this->num_args>2)
    {
        perror("smash error: fare: invalid arguments");
        return;
    }

    cpu_set_t my_set;        /* Define my cpu_set bit mask. */
    CPU_ZERO(&my_set);  // set all to zero
    CPU_SET(this->core_num,&my_set);
    pid_t pid =  JobsList::getInstance().getPidByJobId(this->job_num);
    if (pid<0)
    {
        std::cerr <<"smash error: setcore: job-id "<< this->job_num <<" does not exist" << std::endl;
    }
   if (sched_setaffinity(pid, sizeof(cpu_set_t), &my_set) == -1)
   {
       if (errno ==EINVAL || errno == EPERM   )
       {
           perror("smash error: fare: invalid core number");
       }
       else
           perror("smash error: fare: invalid arguments");
   }

}


void QuitCommand::execute() {
    if (this->num_args==1) {
        exit(0);
    }
    else if (string(this->args[1]) != "kill") {
	exit(0);
    }
    else {
	std::cout << "smash: sending SIGKILL signal to " << JobsList::getInstance().getNumJobs() << " jobs:" << std::endl;
	JobsList::getInstance().killAllJobs();
	exit(0);
    }
}

void KillCommand::execute() {
	if (num_args != 3) {
		std::cout << "smash error: kill: invalid arguments" << std::endl;
		return;
	}
	if ((args[1][0] != '-') || !is_number(args[1]+1) || !is_number(args[2])) {
		std::cout << "smash error: kill: invalid arguments" << std::endl;
		return;
	}
	bool found_job_id = JobsList::getInstance().sendSignal(std::stoi(args[1]+1), std::stoi(args[2]));
	if (!found_job_id) {
		std::cout << "smash error: kill: job-id " << args[2] << " does not exsist" << std::endl;
	}
	return;
}
