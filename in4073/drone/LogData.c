#include "LogData.h"
#include "Command.h"
#include <stdlib.h>

//Authered by Yuxiang
struct LogData * LogData_create(){
    struct LogData * result = (struct LogData *)malloc(sizeof(struct LogData)); 
    return result;
}

struct Command * Command_make_LogData(struct LogData * data_entry)
{
    struct Command *cmd = (struct Command *)malloc(sizeof(struct Command));

    if (cmd)
    {
        cmd->type = LogInfo;       
        cmd->data = (void *) data_entry;
        return cmd;
    }
    free(cmd);
    return NULL;
}

void LogData_destroy(struct LogData * self){
    if(self){
        free(self);
    }
}