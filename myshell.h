#pragma once 
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>

const int BaseSize = 1024;

// 打印控制台内容
void printfCommandHint();

// 获取用户输入
char* getInput();

// 拆分用户输入
void parseInput();

// 执行程序
void executeProgram();

// 判断内建命令并执行
bool checkIfBuiltInAndExecute();

// 导入环境变量
void initShell();

