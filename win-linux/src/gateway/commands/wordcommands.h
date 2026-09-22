#ifndef GATEWAY_COMMANDS_WORDCOMMANDS_H
#define GATEWAY_COMMANDS_WORDCOMMANDS_H

namespace Gateway::Commands
{
    // Registers this family's commands into AllowlistTable::Instance(). Called once
    // from GatewayServer startup, alongside RegisterCellCommands/RegisterSlideCommands/
    // RegisterPdfCommands as each family lands per the plan's build order.
    void RegisterWordCommands();
}

#endif // GATEWAY_COMMANDS_WORDCOMMANDS_H
