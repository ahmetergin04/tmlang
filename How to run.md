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
tmlang [-printMachineTrace] [-printTransitionTrace] [script]
```

### Flags

| Flag | Effect |
|---|---|
| `-printMachineTrace` | After each `run`, prints a tree of every TM/CTM call: how each call ended, how many steps it took, and the tape after it returned. Built-in `R`/`L` and writes are not calls, so they are not shown. |

Example (`tests/test_ctm.tm`):

```
run(reset_nines, t_nums): halt after 20 steps
~ [#] "2" "1" "1"
`-- reset_nines (ctm): halt after 20 steps  |  ~ [#] "2" "1" "1"
    `-- bump_all (ctm): halt_yes after 10 steps  |  ~ # "2" "9" "9" [#]
        |-- Bump (tm): halt after 1 steps  |  ~ # ["2"] "9" "3"
        `-- Bump (tm): halt after 1 steps  |  ~ # "2" "9" ["9"]
```

`-printTransitionTrace` prints the same tree, plus one line per step under the call that made it, with the tape after that step. TM steps are written like transitions in the language; CTM steps are `R`, `L`, `write <symbol>` and `goto <label>: row <condition>` (the row that was picked). It includes everything `-printMachineTrace` shows. A run that hits the step limit prints 10000 step lines.

```
`-- go (ctm): halt after 3 steps  |  ~ [#]
    |-- L  |  [~]
    `-- M (tm): halt after 2 steps  |  ~ [#]
        |-- ("s", ~) |-- ("s", ~, ->)  [default ~ rule]  |  ~ [#]
        `-- ("s", #) |-- (halt, #, <>)  |  ~ [#]
```
