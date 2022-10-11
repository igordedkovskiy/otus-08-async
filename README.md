# otus homework #8 - async shared lib

## Howto
    Get handler by invoking connect function. Argument is a size of a block of commands.
    Invoking receive function transfers data. It's possible to transfer data command by command.
    Data format:
        "cmd1\ncmd2\n" is two commands: "cmd1" and "cmd2", size of data block is 10.
        If bulk_size = 2, then cmd1 and cmd2 is a block.
        A set of commands surrounded by curly brackets is a block.

### Output
    Set of files, where each file represents a block of commands.
    File name is:
        "bulk-${handler value}-${time, first command received}-${blocks cntr}.log"

## Example
    See main.cpp
