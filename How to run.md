# tmlang

A programming language for describing **Turing machines**, with an interpreter and a debugger.

## The language at a glance
|Declaration | What it defines |
|---|---|
| `alphabet` | The tape symbols a machine may use |
| `state_set` | A set of states; sets can be combined with `\/` (union) and `\` (difference) |
| `transition_func` | The transition rules |
| `tm` | A machine: states, alphabet, start state, transition function, halting states |
| `tape` | The initial tape contents |
| `ctm` | A composite machine |
| `run` | Runs a machine on a tape |


## Building

 **CMake 3.16 or newer** and a **C++20 compiler**.

### Linux

```sh
./build.sh 
```

### Windows

On Windows use `build.bat`. 

## Usage

```sh
tmlang [script]
```
