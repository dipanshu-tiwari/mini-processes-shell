# 🐚 Mini Shell (Process-Based Shell)

This is a lightweight shell implemented in C that supports basic command execution and parallel processing using the `&` operator. The shell demonstrates low-level process handling using `fork()` and `execvp()` and includes a custom error handler.

***

## 🚀 Features

- Executes standard shell commands (e.g., `ls`, `echo`, etc.)

- Supports parallel execution using `&` operator.  
Example: `ls & pwd & echo hello`

- Handles the built-in `exit` command to terminate the shell

- Graceful error messages for failed commands

***

## 📁 Structure

#### `main()`

- Displays a prompt: `prompt>`

- Reads a line of input using `getline`

- Tokenizes the input by spaces

- Splits commands by `&` into individual processes

- Constructs a command list and passes it to `ExecuteCommand()`

- Correctly frees dynamically allocated memory

***

#### `ParseCommand(cmd* cmd_list, int cmd_list_len)`

- Forks a child process for each command

- Executes each command using `execvp`

- Waits for all children to finish before returning

***

#### `ExitWithCode(int errCode)`

- Prints an error message and exits with appropriate code

***

## 🧠 Example Usage

```Bash
prompt> ls -l & echo Hello & pwd
```

Expected output:

```
Hello
<current working directory>
<listing of files>
prompt>
```

***

## 🛠 Compilation

Use `gcc` to compile:

```Bash
gcc -o shell shell.c
```

Run it:

```Bash
./mini-shell
```

***

## ⚠️ Notes

- This is a minimal implementation; advanced features like piping (`|`), redirection (`>`, `<`), and job control are not included.

- Commands are split only on spaces and `&`, so quoted strings or complex syntax are not supported.

- Commands are executed in the order they appear but concurrently (not sequentially) when separated by `&`.

***

## 📄 License

Feel free to use or adapt this for educational or personal use. Attribution appreciated but not required.