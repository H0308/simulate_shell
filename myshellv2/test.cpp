#include "myshell.h"
using namespace std;

extern char* global_argv[BaseSize];
extern int global_argc;

int main()
{
    initShell();
    while(true)
    {
        // 打印命令提示信息
        printfCommandHint();
        // cout << endl;
        // 获取用户输入的信息
        char* ret = nullptr;
        if(!(bool)(ret = getInput()))
        {
            continue;
        }
        parseInput();

        //cout << global_argc << endl;
        //for (int i = 0; i < global_argc; i++)
        //{
        //    printf("%s\n", global_argv[i]);
        //}
        // 判断是否是内建命令
        if(checkIfBuiltInAndExecute())
        {
            continue;
        }
        // 否则执行外部命令
        executeProgram();
    }

    return 0;
}
