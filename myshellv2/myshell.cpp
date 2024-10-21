#include "myshell.h"

// 存储用户的输入的数组
char input_arr[BaseSize];
// 命令行参数表
char* global_argv[BaseSize];
// 参数个数
int global_argc;
// 环境变量表
char* global_env[BaseSize];
// 声明父进程的环境变量表
extern char** environ;
// 退出码
int exitCode;
// 自定义退出状态
enum exitStatus
{
    NORMAL, // 进程正常退出
    FAILED, // 正常退出但是没有完成任务
    INTERRUPT, // 异常退出 
    FILEOPENFAIL, // 文件打开失败
    FILENOTEXISTS // 文件不存在
};
// 重定向状态
enum redirStatus
{
    NONE,
    APPEND,
    TRUNCATE,
    INPUT 
};
// 重定向状态码
int redirCode;
// 文件名
char* filename;

// std::string getCwd()
// {
//     char current_wd[BaseSize];
//     getcwd(current_wd, BaseSize);
//     std::string s1(current_wd);
//     if(s1.size() == 1)
//     {
//         return s1;
//     }
//     std::string ret;
//     // 反向遍历字符串，直到遇到第一个/
//     auto rit = s1.rbegin();
//     while(rit != s1.rend())
//     {
//         if((*rit) == '/')
//             break;
//         ret.push_back((*rit));
//         ++rit;
//     }
// 
//     // 反转字符串
//     reverse(ret.begin(), ret.end()); 
//     
//     return ret;
// }

std::string getCwd()
{    
    char current_wd[BaseSize];
    getcwd(current_wd, BaseSize);
    std::string user(current_wd);
    // 如果是根目录，直接返回/
    if (user.size() == 1)
    {
        return user;
    }

    // 否则从末尾向前找倒数第一个/
    size_t pos = user.rfind("/");
    // 返回找到后的字符串进行截取
    return user.substr(pos + 1);
}

void printfCommandHint()
{
    // 存储当前工作路径
    // char current_wd[BaseSize];
    // 获取用户名
    char* user = getenv("USER");
    // 获取主机名
    char* hostname = getenv("HOSTNAME");
    // 获取当前工作路径
    // char* pwd = getenv("PWD");
    // getcwd(current_wd, BaseSize);
    // 打印信息
    printf("[%s@%s %s]%c ", user, hostname, getCwd().c_str(),
            strcmp(user, "root") == 0 ? '#' : '$');
}

char* getInput()
{
    // 读取输入
    fgets(input_arr, BaseSize, stdin);
    // 尽管可能没有输入指令。但是输入指令存在\n
    size_t sz = strlen(input_arr);
    // 消除\n
    input_arr[sz - 1] = '\0';
    // 如果没有输入则直接返回空
    if(sz == 0)
    {
        return nullptr;
    }

    return input_arr;
}

//char* getInput()
//{
//    std::cin.getline(input_arr, BaseSize);
//    if(strlen(input_arr) == 0)
//    {
//       return nullptr;
//    }
//    
//    return input_arr;
//}

void parseInput()
{
    // 清空上一次的命令行参数
    memset(global_argv, 0, BaseSize);
    global_argc = 0;
    // 重定向
    redirCode = NONE;
    filename = nullptr;

    // printf("command start: %s\n", input_arr);
    // 判断是否存在重定向符号
    size_t end = strlen(input_arr) - 1;
    while(end)
    {
        if(input_arr[end] == '>')
        {
            // 判断前面一个字符是否是>
            if(input_arr[end - 1] == '>')
            {
                redirCode = APPEND;
                // 切割字符串
                input_arr[end - 1] = 0;
                input_arr[end] = 0;
                // 获取文件名，重定向符的下一个位置
                filename = &input_arr[end+1];
                // 去除空格
                trim(filename);
                break;
            }
            else 
            {
                // 输出重定向
                redirCode = TRUNCATE;
                // 切割字符串
                input_arr[end] = 0;
                // 获取文件名
                filename = &input_arr[end+1];
                // 去除空格
                trim(filename);
                break;
            }
        }
        else if(input_arr[end] == '<')
        {
            // 输入重定向
            redirCode = INPUT;
            // 切割字符串
            input_arr[end] = 0;
            // 获取文件名
            filename = &input_arr[end+1];
            // 去除空格
            trim(filename);
            break;
        }
        else 
        {
            --end;
        }
    }
    
    // printf("redir: %d\n", redirCode);
    // printf("filename: %s\n", filename);
    // printf("command end: %s\n", input_arr);

    // 拆分读取的字符串
    for(char* ch = strtok(input_arr, " "); (bool)ch; ch = strtok(nullptr, " "))
    {
       global_argv[global_argc++] = ch; 
    }
}

void executeProgram()
{
    pid_t id = fork();
    if(id == 0)
    {
        // 判断是哪一种重定向
        if(redirCode == TRUNCATE)
        {
            // 输出重定向
            // 1. 打开文件
            // 确保文件不为空
            if(filename)
            {
                int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if(fd < 0)
                {
                    exit(FILEOPENFAIL);
                }
                // 2. 重定向stdout文件为新创文件
                dup2(fd, stdout->_fileno);
            }
            else 
            {
                exit(FILENOTEXISTS);
            }
        }
        else if(redirCode == APPEND)
        {
            if(filename)
            {
                int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
                if(fd < 0)
                {
                    exit(FILEOPENFAIL);
                }
                // 2. 重定向stdout文件为新创文件
                dup2(fd, stdout->_fileno);
            }
            else
            {
                exit(FILENOTEXISTS);
            }
        }
        else if(redirCode == INPUT)
        { 
            if(filename)
            {
                int fd = open(filename, O_RDONLY);
                if(fd < 0)
                {
                    exit(FILEOPENFAIL);
                }
                // 2. 重定向stdout文件为新创文件
                dup2(fd, stdin->_fileno);
            }
            else
            {
                exit(FILENOTEXISTS);
            }
        }
        // 重定向后恢复redirCode标记便于下次使用
        redirCode = NONE;
        // 程序替换
        execvpe(global_argv[0], global_argv, global_env);
        exit(0);
    }

    // 存储进程退出码
    int status = 0;
    // 父进程等待
    pid_t rid = waitpid(id, &status, 0);
    // 正常回收情况下
    if(rid > 0)
    {
        if(WIFEXITED(status))
        {
            // 正常退出码
            exitCode = WEXITSTATUS(status);
        }
        else if(WIFSIGNALED(status)) 
        {
            // 被信号终止
            exitCode = WTERMSIG(status);
        }
    }
}

bool checkIfBuiltInAndExecute()
{
   if(strcmp(global_argv[0], "cd") == 0)
   {
       // 判断是否有两个参数防止越界访问
       if(global_argc == 2)
       {
           if(strcmp(global_argv[1], "~") == 0)
           {
               std::string user(getenv("USER"));
               if(user == "root")
               {
                    chdir("/root");
               }
               else 
               {
                    chdir(getenv("HOME"));
               }
           }
           else 
           {
                chdir(global_argv[1]);
           }
           exitCode = NORMAL;
           return true;
       }
   }
   else if(strcmp(global_argv[0], "export") == 0)
   {
       if(global_argc == 2)
       {
           size_t i = 0;
           // 先找到环境变量表为空位置
           for(; global_env[i]; i++);
           // 添加新的环境变量，不考虑重复，理论上不会出现空间不足导致越界
           global_env[i] = (char*)malloc(sizeof(strlen(global_argv[1])));
           strncpy(global_env[i], global_argv[1], (strlen(global_argv[1]) + 1));
           global_env[++i] = nullptr;
           exitCode = NORMAL;
           return true;
       }
   }
   else if(strcmp(global_argv[0], "env") == 0)
   {
       for(size_t i = 0; global_env[i]; i++)
       {
           printf("%s\n", global_env[i]);
       }
       exitCode = NORMAL;
       return true;
   }
   else if(strcmp(global_argv[0], "echo") == 0)
   {
       // 获取第二个参数的第一个字符和第二个字符
       if(global_argv[1][0] == '$' && global_argv[1][1])
       {
           printf("%d\n", exitCode);
       }
       else 
       {
           // 直接输出字符串
           printf("%s\n", global_argv[1]);
       }
       // 将上一次的退出码置为0
       exitCode = NORMAL;
       return true;
   }
   exitCode = FAILED;
   return false;
}

void initShell()
{
    // 拷贝父进程的环境变量表
    for(size_t i = 0; environ[i]; i++)
    {
        global_env[i] = environ[i];
    }
    // 最后一个位置置为空
    global_env[strlen(*(environ))] = nullptr;
}

