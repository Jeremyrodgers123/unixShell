//
//  main.cpp
//  homework2
//
//  Created by Jeremy Rodgers on 2/5/19.
//  Copyright © 2019 Jeremy Rodgers. All rights reserved.
//

#include <iostream>
#include "shelpers.hpp"



int main(int argc, const char * argv[]) {
    std::vector<Command> commands;
    std::string line;
    std::vector<std::string> tokens;
    //get line of input
    while(std::getline(std::cin, line)){
        std::vector<pid_t> pids; //stores child pids
        //tokenize and parse commands
        tokens = tokenize(line);
        commands = getCommands(tokens);
        if(tokens[0] =="exit"){
            break;
        }
        //loop through each command
        for( int i = 0; i < commands.size(); i++){
            Command command = commands[i];
            //std:: cout << "current command: " << command.exec << std::endl;
            //BUILTINS
            if(commands[0].exec == "cd"){
                const char *path;
                if(commands.at(0).argv.at(1) != nullptr){
                    path = commands.at(0).argv.at(1);
                }
                else{
                    path = getenv ("HOME");
                }
                int cdRet = chdir(path);
                std::cout << path << std::endl;
                if(cdRet < 0){
                     perror("error in cd function");
                }
               
                //hasError(cdRet, "error in cd function");
                continue;
            }
            //declare a environment variable with export
            if(commands[0].exec == "export"){
                std::string variableName = commands[0].argv[1];
                std::string variableValue = commands[0].argv[2];
                setenv(variableName.c_str(), variableValue.c_str(), false);
                continue;
            }
            //for each command, make a new process
            int pid = fork();
            hasError(pid, "A forking error occured!!!");
            if(pid == 0){
                //std::cout << "hello from the child process"<< getpid() <<"\n";
                int dupRet = dup2(command.fdStdin, 0);
                hasError(dupRet, "error occured changing standard input");

                int dup2Ret = dup2(command.fdStdout, 1);
                hasError(dup2Ret, "error occured changing standard output");
                //MURDERR all pipes that cause stupid hanging bug
                for(int j = 0; j < commands.size() ; j ++){
                    if(commands[j].fdStdin != 0){
                        close(commands[j].fdStdin);
                    }
                    if(commands[j].fdStdout != 1){
                        close(commands[j].fdStdout);
                    }
                }
                int execRet = execvp(command.argv.at(0), const_cast<char**> ( command.argv.data() ) );
                hasError(execRet, "something went wrong in exec system call");
                int closeRet = close(command.fdStdin);
                hasError(closeRet, "error closing file");
                if(commands.size() - 1 == i){
                   int closeOutRet = close(command.fdStdout);
                    hasError(closeOutRet, "error closing output file");
                }
                exit(0);
            }else{
                pids.push_back(pid);
                //std::cout << "hello from the parent process "<< getpid() << "\n";
            }
            //determine if error, child or parent process
        }
        
        for(int j = 0; j < commands.size() ; j ++){
            //close remaining pipes
            if(commands[j].fdStdin != 0){
                close(commands[j].fdStdin);
            }
            if(commands[j].fdStdout != 1){
                close(commands[j].fdStdout);
            }
        }
    
        for(int i = 0; i < pids.size(); i++){
            int waitRet =  waitpid(pids[i], NULL, 0);
            //hasError(waitRet, "error in wait function");
        }
    }
    return 0;
}
