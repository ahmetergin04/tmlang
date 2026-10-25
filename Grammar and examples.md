# TM Language — Grammar and Examples

Based on *ENS 491_2 – TM compiler and debugger* (the PDF). Keywords and
tokens follow the scanner (`include/token.h`, `src/scanner.cpp`). The
[Changes from the PDF](#changes-from-the-pdf) section at the end lists every
difference from the PDF and why it was made.

## Notation

| Form | Meaning |
|---|---|
| `rule ::= ...` | Defines `rule`. Rule names are lowercase `snake_case`. |
| `"x"` | A terminal: the literal text `x`. Only double quotes are used. |
| `a b` | `a` followed by `b` |
| `a \| b` | `a` or `b` |
| `( ... )` | Grouping |
| `( ... )*` | Zero or more times |
| `( ... )?` | Optional |
| `STRING`, `IDENTIFIER` | Token classes, defined in the lexical grammar |

---

## 1. Lexical grammar

### Keywords

Keywords are lowercase and case-sensitive (`TM` is an identifier, not a keyword).

| Keyword | Token | Meaning |
|---|---|---|
| `alphabet` | `ALPHABET` | Alphabet declaration |
| `state_set` | `STATE_SET` | State set declaration |
| `transition_func` | `TRANS_FUN` | Transition function declaration |
| `tm` | `TM` | Turing machine declaration |
| `ctm` | `CTM` | Composite Turing machine declaration |
| `tape` | `TAPE` | Tape declaration |
| `run` | `RUN` | Run statement |
| `head` | `HEAD` | The symbol under the tape head (used in conditions) |
| `in` | `IN` | Membership test (used in conditions) |
| `halt` | `HALT` | Halting state of a computing / semi-deciding TM |
| `halt_yes` | `HALT_YES` | Accepting halting state of a deciding TM |
| `halt_no` | `HALT_NO` | Rejecting halting state of a deciding TM |

### Operators and punctuation

| Lexeme | Token | Meaning |
|---|---|---|
| `(` `)` | `LEFT_PAREN` `RIGHT_PAREN` | Grouping, argument lists, tuples |
| `{` `}` | `LEFT_BRACE` `RIGHT_BRACE` | Set / table literals |
| `,` | `COMMA` | Separator |
| `;` | `SEMICOLON` | Ends every declaration and statement |
| `.` | `DOT` | Machine chaining in CTMs |
| `=` | `EQUAL` | Binds a name to a definition |
| `~` | `BLIP` | Left-end marker: always the first cell of a tape |
| `#` | `BLANK` | Blank symbol |
| `_` | `UNDERSCORE` | Wildcard |
| `>` | `START` | Entry row of a CTM |
| `->` | `RIGHT_ARROW` | Move head right |
| `<-` | `LEFT_ARROW` | Move head left |
| `<>` | `STAY` *(new, scanner does not produce it yet)* | Head stays in place |
| `\|-` | `COMPUTES` | Yields, in the `(state, symbol) \|- (...)` transition form |
| `==` | `EQUAL_EQUAL` | Equal |
| `!=` | `BANG_EQUAL` | Not equal |
| `&` | `AND` | Logical and |
| `\|` | `OR` | Logical or |
| `\/` | `UNION` | Set union |
| `\` | `DIFFERENCE` | Set difference |
| `!` | `BANG` | Not — reserved, not used by the grammar yet |
| `*` | `KLEENE_CLOSURE` | Kleene star — reserved, not used by the grammar yet |
| `::` | `SCOPE_RESOLUTION_OPERATOR` | Reserved, not used by the grammar yet |

Two-character operators are matched first (maximal munch): `->` before `>`,
`<-` and `<>` before `<`, `==` before `=`, `!=` before `!`, `|-` before `|`, `\/` before `\`.

### Token classes

```
STRING      ::= "\"" <any characters except "\""> "\""
IDENTIFIER  ::= letter ( letter | digit | "_" )*      // that is not a keyword
letter      ::= "a" ... "z" | "A" ... "Z"
digit       ::= "0" ... "9"
```

- **STRING** is a double-quoted literal. It may span lines and has no escape
  sequences. Its value is the text between the quotes. Tape symbols and state
  names are always strings: `"1"`, `"A"`, `"green"`.
- **IDENTIFIER** is a bare name used for declared things (alphabets, state
  sets, functions, machines, tapes, CTM row labels). It cannot start with `_`,
  because a lone `_` is the `UNDERSCORE` token. It cannot contain `-`.
- There are no number literals: write `"1"`, not `1`.

### Comments and whitespace

```
comment     ::= "//" <any characters up to the end of the line>
```

Spaces, tabs, carriage returns and newlines separate tokens and are otherwise
ignored. Block comments (`/* ... */`) are not supported.

---

## 2. Syntax grammar

### Program

```
program      ::= declaration* EOF

declaration  ::= alphabet_def
               | state_set_def
               | transition_function
               | turing_machine
               | tape_def
               | composite_tm
               | run
```

Every declaration ends with `";"`.

### Shared rules

```
symbol       ::= STRING | "#" | "~"
move         ::= "->" | "<-" | "<>"
halt_state   ::= "halt" | "halt_yes" | "halt_no"
```

- `symbol` is a tape symbol. `#` (blank) and `~` (blip) are not listed in any
  alphabet but are part of every tape, so transitions and conditions must be
  able to mention them.
- `~` may only be read, never written somewhere new: a transition that reads
  `~` must write `~` back and move `->`.

### Alphabet

```
alphabet      ::= "{" STRING ( "," STRING )* "}"
alphabet_def  ::= "alphabet" IDENTIFIER "=" alphabet ";"
```

### State set

```
state_set      ::= "{" STRING ( "," STRING )* "}"
state_set_expr ::= state_set_term ( ( "\/" | "\" ) state_set_term )*
state_set_term ::= IDENTIFIER | state_set
state_set_def  ::= "state_set" IDENTIFIER "=" state_set_expr ";"
```

Set operators have equal precedence and are evaluated left to right, so
`Q1 \ {"A"} \/ {"blank"}` means `(Q1 \ {"A"}) \/ {"blank"}`.

### Transition function

```
next_state          ::= STRING | halt_state
transition          ::= "(" STRING "," symbol "," next_state "," symbol "," move ")"
                      | "(" STRING "," symbol ")" "|-" "(" next_state "," symbol "," move ")"
transition_function ::= "transition_func" IDENTIFIER
                        "(" ( IDENTIFIER | state_set ) "," ( IDENTIFIER | alphabet ) ")"
                        "=" "{" transition ( "," transition )* "}" ";"
```

A transition has two equivalent forms. Both can be mixed in one function:

| Form | Example |
|---|---|
| `(state, read, next state, write, move)` | `("A", "1", "B", "2", ->)` |
| `(state, read) \|- (next state, write, move)` | `("C", "2") \|- ("B", "9", ->)` |

Both examples read: *in state A reading 1, go to state B, write 2, move right*
(and the same for the second with C, 2, B, 9).

The move is always written: `->` (right), `<-` (left) or `<>` (stay).

### Turing machine

```
final_state    ::= "{" "halt" "}"
                 | "{" "halt_yes" "," "halt_no" "}"
turing_machine ::= "tm" IDENTIFIER
                   "(" ( IDENTIFIER | state_set ) ","
                       ( IDENTIFIER | alphabet ) ","
                       STRING ","                        // start state
                       IDENTIFIER ","                    // transition function
                       final_state
                   ")" ";"
```

- `{halt}`: the machine **computes** a function or **semi-decides** a language.
  It stops only by entering `halt`.
- `{halt_yes, halt_no}`: the machine **decides** a language. It accepts by
  entering `halt_yes` and rejects by entering `halt_no`.

Halting states are not members of the state set. A machine's transitions may
only enter the halting states its `final_state` declares.

### Tape

```
tape_cell ::= STRING | "#"
tape_def  ::= "tape" IDENTIFIER "(" ( IDENTIFIER | alphabet ) ")"
              "=" "{" "~" ( "," tape_cell )* "}" ";"
```

Every tape starts with `~` (blip), the left-end marker; it is mandatory and
appears nowhere else. The other cells are symbols of the tape's alphabet or
`#` (blank). `{~}` is an empty tape.

### Composite Turing machine (CTM)

```
condition    ::= "_" | or_logical
or_logical   ::= and_logical ( "|" and_logical )*
and_logical  ::= equality ( "&" equality )*
equality     ::= "head" ( ( "==" | "!=" ) symbol
                        | "in" ( IDENTIFIER | alphabet ) )

next_tm      ::= next_tm_atom ( "." next_tm_atom )*
next_tm_atom ::= IDENTIFIER | symbol | halt_state

ctm_row      ::= "(" ( IDENTIFIER | ">" ) "," condition "," next_tm ")"
composite_tm ::= "ctm" IDENTIFIER "(" ( IDENTIFIER | alphabet | "_" ) ")"
                 "=" "{" ctm_row ( "," ctm_row )* "}" ";"
```

- **Row state**: `>` marks the entry row (exactly one per CTM). Other rows are
  named by a label such as `A`.
- **Row order**: rows with the same label are checked top to bottom, and the
  first row whose condition is true is taken. A `_` row is the fallback, so it
  comes last for its label (any row after it could never be reached). The `>`
  row always uses `_`.
- **Condition**: `_` is always true. Otherwise it tests the symbol under the
  head. `|` binds weaker than `&`, so `head == "1" | head == "2" & head != #`
  means `head == "1" | (head == "2" & head != #)`.
- **next_tm** is a chain run left to right. Each atom is one of:
  - a machine name, such as `R` or `L`;
  - a symbol, meaning "write this symbol" (never `~`);
  - a halting state;
  - a row label, meaning "go to that row" (as the final atom).
- **Parameter**: `_` means the CTM works over any alphabet.

### Run

```
run ::= "run" "(" IDENTIFIER "," IDENTIFIER ")" ";"
```

Runs a TM or CTM (first identifier) on a tape (second identifier).

---

## 3. Examples

```
// ---- Alphabets ----
alphabet numbers     = {"1", "2", "3", "9"};
alphabet numbers_sub = {"1", "2", "3"};
alphabet colors      = {"green", "blue"};

// ---- State sets ----
state_set Q1     = {"A", "B", "C", "D"};
state_set Q1_sub = Q1 \ {"A", "D"} \/ {"blank"};   // = {"B", "C", "blank"}
state_set Q2     = {"x", "y", "w", "p", "q"};

// ---- Transition functions ----

// Both forms can be mixed.
transition_func d1(Q1, numbers) = {
    ("C", ~,   "C",  ~,   ->),
    ("A", "1", "B",  "2", ->),
    ("C", "2") |- ("B", "9", ->),
    ("B", "3", halt, "3", <>)
};

transition_func d2(Q2, colors) = {
    ("x", ~,       "x",      ~,       ->),
    ("x", "green", halt_yes, "green", ->),
    ("x", "blue",  halt_no,  "blue",  ->)
};

// ---- Turing machines ----
tm M1(Q1, numbers, "C", d1, {halt});               // computes
tm M2(Q2, colors,  "x", d2, {halt_yes, halt_no});  // decides

// ---- Tapes ----
tape t1(numbers) = {~, "1", "2", "3", "3"};
tape t2(colors)  = {~, "green", #, "blue"};

// ---- Composite Turing machines ----

// Move right past every symbol in numbers_sub, then halt.
ctm skip_sub(numbers) = {
    (>, _, R.A),
    (A, head in numbers_sub, R.A),
    (A, _, halt)
};

// Erase leftwards (write blanks) until a blank is under the head.
ctm clean_up(_) = {
    (>, _, R.L.A),
    (A, head != #, #.L.A),
    (A, head == #, halt)
};

// ---- Runs ----
run(M1, t1);
run(M2, t2);
run(skip_sub, t1);
```

---

## Changes from the PDF

### Fixes to the grammar

1. **`final_state`.** The PDF rule `"{" "h" | "h_yes , h_no" "}"` split at the
   `|` into two halves with unmatched braces, and wrote `h_yes , h_no` as one
   token. It is now two explicit alternatives: `{halt}` for computing and
   semi-deciding machines, and `{halt_yes, halt_no}` for deciding machines.
   Because transitions must be able to enter these states, a transition's next
   state may now also be a `halt_state`.
2. **Blip, blank and `tape_def`.** The PDF rule
   `"{" # | STRING ( "," STRING )* "}"` split at the `|` the same way and left
   `#` unquoted. The PDF used `#` for the left-end marker; the marker is now `~`
   (blip) and `#` is the blank symbol. The rule is now
   `"{" "~" ( "," tape_cell )* "}"`, so every tape starts with `~` and `~`
   appears nowhere else. `~` and `#` are both `symbol`s, so transitions can
   read them and conditions can test for them.
3. **`equality`.** The PDF rule
   `STRING ( ( '!=' | '==' ) STRING )* || (STRING ('in' Alphabet))*` had five
   problems:
   - `||` is not a grammar operator. It is replaced with the normal `|`.
   - The left operand was `STRING`, but every example tests `head`, which is a
     keyword, not a `STRING`. The left operand is now always `"head"`.
   - `(...)*` allowed chains like `a == b == c`, which have no meaning, and let
     the `in` alternative match nothing. Exactly one comparison is now required.
   - `Alphabet` was the old keyword, used where the name of an alphabet belongs.
     It is now `IDENTIFIER | alphabet`, so either a declared name or a literal
     works.
   - The right side could not be `#` or `~`, so `head == #` (the most common
     test) could not be written. It is now a `symbol`.
4. **Rule names** are all lowercase. `Or_logical` and `And_logical` became
   `or_logical` and `and_logical`, which matches how `condition` refers to them.
5. **Notation.** Only double quotes are used. The single quotes, the curly
   quotes in the `turing_machine` rule, and the `<rule>` angle brackets were
   removed.
6. **Missing pieces:**
   - a. Added a top-level `program` rule.
   - b. Defined `STRING`, `IDENTIFIER` and comments, based on the scanner.
     Strings are double-quoted, so symbols and state names are written `"A"`,
     not `A`.
   - c. Every declaration, including `run`, now ends with `";"`.
7. **Keywords** match the scanner: `alphabet`, `state_set`, `transition_func`,
   `tm`, `ctm`, `tape`, `run`, `head`, `in`, `halt`, `halt_yes`, `halt_no`.
   - Logical and/or are `&` and `|`. The words `and`, `or` and `not` are not
     keywords.
   - `<>` is the "stay" move. The scanner needs a new `STAY` token for it.
   - `!`, `*` and `::` are reserved for future use.
8. **Transitions.** The PDF had a 4-tuple `(state, read, next, action)` where
   the action was either a move or a write. A transition now always writes and
   moves, and has two equivalent forms:
   `(state, read, next, write, move)` and
   `(state, read) |- (next, write, move)`. The move must always be written, and
   is `->` (right), `<-` (left) or `<>` (stay).

### Additions needed so the PDF's examples parse

All of these use tokens the scanner already produces.

- **State-set algebra** (`\/`, `\`). The PDF only had it in a comment, and
  `tests/test.tm` already uses it.
- **`>` and `_` in CTMs**: `>` as the entry row, `_` as the always-true
  condition, and `_` as "any alphabet" for the CTM parameter.
- **`next_tm` atoms**: a chain can contain symbols (`#.L.A`) and halting states
  (`halt`), not only identifiers.

### Example changes

- **Keywords and strings.** Keywords are lowercase, and symbols and states are
  quoted strings.
- **Duplicate declarations.** The PDF's two example blocks were merged, so no
  name is declared twice.
- **`d1`.** Uses the corrected transitions `("A","1","B","2",->)` and
  `("C","2") |- ("B","9",->)`. The PDF's `A` as the move in the second one was
  read as a move, written `->`. Two transitions were added so the machine can
  start on `~` and can halt.
- **`blank = %`.** Removed: `#` is the blank symbol, and the scanner rejects `%`.
- **`M1`.** It used `{A,B}` as final states; it now uses `{halt}`.
- **`M2`.** It had an empty argument and an undefined `d2`. `d2` is now
  defined, and `M2` is a deciding machine.
- **`t1`.** It was declared over the state set `Q1` and started with `#`; it is
  now over the alphabet `numbers` and starts with `~`.
- **CTM `R`.** Renamed to `skip_sub`, because `R` is also the move-right machine
  used in its own rows.
- **`clean-up`.** Renamed to `clean_up`, because identifiers cannot contain `-`.
  Its `-` condition was changed to `_`.
- **`Run(CTM, <tape>)`.** Replaced with real `run` statements.
