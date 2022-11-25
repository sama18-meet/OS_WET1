#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <limits.h>
#include "Commands.h"

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
    for (int i = 0; i < buffer.size(); ++i) {
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
    this->num_args = _parseCommandLine(cmd_line, this->args);
    this->Is_back_ground= false;
    string temp = string (args[num_args-1]);
    if (temp == "&" || temp.back() =='&')
    {
        this->Is_back_ground= true;
    }
    command_line = new char[string(cmd_line).length() + 1];
    strcpy(command_line, cmd_line);

}

Command::~Command() {
    for (int i=0; i<COMMAND_MAX_ARGS+1 ; ++i) {
        free(this->args[i]);
    }
}


///////////////////////////////////////////////////////////
// ChangePromptCommand methods implementation



ChangePromptCommand::ChangePromptCommand(const char* cmd_line) : Command(cmd_line) {
    if (num_args>1) {
        string temp = string(args[1]);
        if (temp == "&") {
            num_args--;
        }
        if (temp.back() == '&') {
            temp.pop_back();
        }
        new_prompt = temp;
    }

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
    string temp =args[1];
    if (temp.back()=='&')
    {
        temp.pop_back();
    }
    smash.setCwd(temp);
}

///////////////////////////////////////////////
// ExternalCommand implementation
///////////////////////////////////////////////
ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {
    std::stringstream ss(cmd_line);
    string buf;
    while (ss >> buf) {
        if (ContainsWildcards(buf)) {
            this->setComplexCommand(true);
        }
        if (buf=="sleep" || buf=="sleep&")
        {
            Sleep_Command= true;
        }
    }
}

void ExternalCommand::execute() {
    pid_t pid = fork();
    if (pid < 0) //that is for error
    {
        std::cerr<<"smash error: fork failed"<<std::endl;
        return;
    }
    else if (pid==0) //the son runs
    {
        if (this->Complex_Command || this->Sleep_Command) // use bash
        {
            setpgrp();
            _removeBackgroundSign(this->command_line); ///check if command line is corrupted
            const char* path_args[] = {"/bin/bash","-c",this->command_line, nullptr};
            if (execv(path_args[0], (char**)path_args) == -1)
            {
                perror("smash error: execv failed");
                return;
            }
        }
        else // using exec v
        {
            setpgrp();
            std::stringstream ss(this->command_line);
            string buff;
            string args;
            string path;

            ss >> path;

            while (ss>>buff)
            {
                args+=buff;
                args+=" ";
            }
            const char* filepath=path.c_str();
            const char* path_args[] = {args.c_str(),nullptr};
            if (args=="")
            {
                const char* path_args1[] = {nullptr};
                if (execv(filepath,(char**)path_args1) == -1)
                {
                    perror("smash error: execv failed");
                    return;
                }
            }
            else if (execv(filepath, (char**)path_args) == -1)
            {
                perror("smash error: execv failed");
                return;
            }

        }
    }
    else if (pid>0) ///that is the father, should wait for son and add to jop list if its a back ground
    {

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

    string cmd_s = _trim(cmd_line);
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" "));

    if(firstWord.back()== '&' ) {
        firstWord.pop_back();
    }

    if (firstWord.compare("chprompt") == 0) {
        return new ChangePromptCommand(cmd_s.c_str());
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_s.c_str());
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_s.c_str());
    }
    else if (firstWord.compare("cd") == 0 ) {
        return new ChangeDirCommand(cmd_s.c_str());
    }
    else
    {
        return new ExternalCommand(cmd_s.c_str());
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    if (cmd == nullptr) {
        return;
    }
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


