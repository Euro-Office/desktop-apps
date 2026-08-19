#ifndef GATEWAY_COMMANDS_CELLCOMMANDS_H
#define GATEWAY_COMMANDS_CELLCOMMANDS_H

namespace Gateway::Commands
{
    // Registers this family's commands into AllowlistTable::Instance(). See
    // wordcommands.h for the registration-call convention.
    void RegisterCellCommands();
}

#endif // GATEWAY_COMMANDS_CELLCOMMANDS_H
