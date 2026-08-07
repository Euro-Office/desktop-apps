#ifndef GATEWAY_COMMANDS_SLIDECOMMANDS_H
#define GATEWAY_COMMANDS_SLIDECOMMANDS_H

namespace Gateway::Commands
{
    // Registers this family's commands into AllowlistTable::Instance(). See
    // wordcommands.h for the registration-call convention.
    void RegisterSlideCommands();
}

#endif // GATEWAY_COMMANDS_SLIDECOMMANDS_H
