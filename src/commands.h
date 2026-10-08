#ifndef COMMANDS_H
#define COMMANDS_H

#define COMMAND_MAX 128

typedef void (*command_function)(void);

typedef struct
{
    const char* name;
    const char* description;
    command_function function;
} Command;

void execute_command(const char* command);
int string_equals(const char* a, const char* b);

#endif
