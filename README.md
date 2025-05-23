# 🐚 Mini Shell (Process-Based Shell)

This is a lightweight shell implemented in C that supports basic command execution and parallel processing using the `&` operator. The shell demonstrates low-level process handling using `fork()` and `execv()` and includes a custom error handler. The shell can be used in normal mode or batch mode (using a batch file).

## 🚀 Features

- Executes standard shell commands (e.g., `ls`, `echo`, etc.)

- Supports parallel execution using `&` operator.  
Example: `ls & pwd & echo hello`

- Handles the built-in `exit` command to terminate the shell, `cd` command to change directories, and `path` to modify search paths

- Handles both absolute and relative path

- Includes output redirection using `>` operator

- Graceful error messages for failed commands

- Supports input from batch file

## 📁 Structure

#### `main()`

- Displays a prompt: `prompt>`

- Reads a line of input using `getline`

- Calls `ParseCommand()` and then `ExecuteCommand()`

- Correctly frees dynamically allocated memory

#### `cmd* ParseCommand(char* inp, int* cmd_list_len)`

- Tokenizes the input by spaces

- Splits commands by `&` into individual processes

- Constructs a command list and passes it to `main()`

#### `ExecuteCommand(cmd* cmd_list, int cmd_list_len, char** paths)`

- Forks a child process for each command

- Executes each command using `execv` by checking the path

- Waits for all children to finish before returning

#### `ExitWithCode(int errCode)`

- Prints an error message and exits with appropriate code

## 🧠 Example Usage

```Bash
prompt> ls -l & echo Hello World & pwd
```

Expected output:

```
Hello World
<current working directory>
<listing of files>
prompt>
```

## 🛠 Compilation

Use `gcc` to compile:

```Bash
gcc -o shell shell.c
```

Run it:

##### In normal mode:

```Bash
./shell
```

##### In batch mode:

```Bash
./shell <batch-file>
```

## ⚠️ Notes

- This has only been tested on Linux (Ubuntu 24.04.2 LTS).

- This is a minimal implementation; advanced features like piping (`|`), some redirection like `<`, and job control are not included (output redirection is included using `>` operator).

- Commands are split only on spaces, `>`, and `&`, so quoted strings or complex syntax are not supported.

- Commands are executed in the order they appear but concurrently (not sequentially) when separated by `&`.