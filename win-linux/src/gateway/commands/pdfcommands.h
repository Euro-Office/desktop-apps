#ifndef GATEWAY_COMMANDS_PDFCOMMANDS_H
#define GATEWAY_COMMANDS_PDFCOMMANDS_H

namespace Gateway::Commands
{
    // Registers this family's commands into AllowlistTable::Instance(). See
    // wordcommands.h for the registration-call convention.
    void RegisterPdfCommands();
}

#endif // GATEWAY_COMMANDS_PDFCOMMANDS_H
