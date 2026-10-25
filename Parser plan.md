# Parser Plan

A step-by-step plan for writing the parser for the grammar in
[Grammar and examples.md](Grammar%20and%20examples.md), following
*Crafting Interpreters* (recursive descent + Visitor pattern) in C++.

Each step lists the book chapter to read alongside it. Do the steps in order:
each one can be built and tested before the next.

---

## Step 0 — Fix the existing parser helpers
**Book:** ch. 6.2 *Recursive Descent Parsing*, ch. 6.3 *Syntax Errors*

What is already there is the book's helper set (`peek`, `previous`, `advance`,
`check`, `match`, `consume`, `isAtEnd`). Before building on it:

- [ ] `consume` does `throw error;`, which throws a pointer to the `error`
      function. It should be `throw Perror(peek(), message);`
      (book: `throw error(peek(), message);`).
- [ ] Add a constructor: `explicit parser(std::vector<token> tokens);`
      and make `tokens` / `current` private.
- [ ] Make `Perror` report *where* the error is, like the book's
      `Lox.error(Token, String)`: at end → `" at end"`, otherwise
      `" at '" + lexeme + "'"`. `report(line, where, message)` already exists
      in `error.h`.
- [ ] Add `void synchronize();` (book 6.3.3). Skip tokens until just after a
      `SEMICOLON`, or until the next token is a declaration keyword:
      `ALPHABET, STATE_SET, TRANS_FUN, TM, CTM, TAPE, RUN`.
- [ ] Add the public entry point: `std::vector<std::unique_ptr<Stmt>> parse();`

---

## Step 1 — AST node classes
**Book:** ch. 5 *Representing Code* (5.2 *Implementing Syntax Trees*,
5.3 *Working with Trees*), ch. 8.1 (Stmt classes)

The book generates its classes with `GenerateAst.java`. In C++, write them by
hand in an `include/ast.h` (or write a small generator script).

Base classes:

```cpp
struct Expr {
    virtual ~Expr() = default;
    virtual void accept(Visitor& visitor) = 0;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(Visitor& visitor) = 0;
};
```

Each node stores its children as `std::unique_ptr<...>` (or
`std::vector<std::unique_ptr<...>>`) and its leaves as `token`s. Keeping the
token, not just the string, keeps `line` for error messages later.

### Suggested nodes (mapped to the grammar)

**Expressions** — the only real expression trees are *set expressions* and
*conditions*. They work exactly like Lox's `Binary` / `Logical` / `Variable`:

| Node | Fields | Grammar rule | Lox equivalent |
|---|---|---|---|
| `Set_Literal` | `std::vector<token> items` | `alphabet`, `state_set` | `Literal` |
| `Set_Ref` | `token name` | `IDENTIFIER` inside a set expression | `Variable` |
| `Set_Binary` | `left`, `token op` (`UNION`/`DIFFERENCE`), `right` | `state_set_expr` | `Binary` |
| `WildCard` | `token underscore` | `"_"` condition | `Literal` |
| `Logical` | `left`, `token op` (`OR`/`AND`), `right` | `or_logical`, `and_logical` | `Logical` |
| `Equality` | `token op` (`EQUAL_EQUAL`/`BANG_EQUAL`), `token symbol` | `"head" ("=="\|"!=") symbol` | `Binary` |
| `Membership` | `token in`, `std::unique_ptr<Expr> set` (`Set_Ref` or `Set_Literal`) | `"head" "in" ...` | — |

> One `Logical` node with an operator token replaces your separate
> `Or_logic` / `And_logic` classes — this is what the book does (ch. 9.3).
> `Condition` then disappears as a class: a condition is just an `Expr`.

#### Expression examples

Each example shows the source text, then the tree the parser builds from it.

**`Set_Literal`** — a set written out in braces.
```
{"A", "B", "C"}

Set_Literal
└─ items: "A", "B", "C"
```

**`Set_Ref`** — the name of a set declared earlier.
```
Q1                      // e.g. in: state_set Q2 = Q1;

Set_Ref
└─ name: Q1
```

**`Set_Binary`** — `\/` or `\` between two sets. Left-associative, so the
first operator ends up deepest in the tree.
```
Q1 \ {"A", "D"} \/ {"blank"}

Set_Binary  op: \/
├─ left:  Set_Binary  op: \
│         ├─ left:  Set_Ref  Q1
│         └─ right: Set_Literal  "A", "D"
└─ right: Set_Literal  "blank"
```

**`WildCard`** — the always-true condition.
```
(A, _, halt)            // the "_" in this CTM row

WildCard
└─ underscore: _
```

**`Equality`** — compares the head symbol with one symbol.
```
head != #

Equality
├─ op:     !=
└─ symbol: #
```

**`Membership`** — tests whether the head symbol is in a set.
```
head in numbers_sub

Membership
├─ in:  in
└─ set: Set_Ref  numbers_sub
```

**`Logical`** — `|` or `&` between two conditions. `&` binds tighter than
`|`, so it ends up deeper in the tree.
```
head == "1" | head == "2" & head != #

Logical  op: |
├─ left:  Equality  == "1"
└─ right: Logical  op: &
          ├─ left:  Equality  == "2"
          └─ right: Equality  != #
```

**Parts of declarations** — these are plain data, not expressions. They do not
need `accept`; the statement that owns them can be visited instead:

| Struct | Fields | Grammar rule |
|---|---|---|
| `Transition` | `token state, read, next, write, move` | both `transition` forms |
| `Final_state` | `std::vector<token> halts` (1 or 2 tokens) | `final_state` |
| `Ctm_row` | `token label` (IDENTIFIER or `>`), `std::unique_ptr<Expr> condition`, `std::vector<token> next_tm` | `ctm_row`, `next_tm` |

> Both transition forms produce the same `Transition` struct; the parser
> erases the difference. The rest of the compiler never needs to know which
> form was written.

#### Examples of the data structs

**`Transition`** — both forms give the same struct.
```
("C", "2", "B", "9", ->)
("C", "2") |- ("B", "9", ->)

Transition
├─ state: "C"
├─ read:  "2"
├─ next:  "B"
├─ write: "9"
└─ move:  ->
```

**`Final_state`** — one halt state (computing) or two (deciding).
```
{halt}                  →  Final_state  halts: [halt]
{halt_yes, halt_no}     →  Final_state  halts: [halt_yes, halt_no]
```

**`Ctm_row`** — one row of a CTM table.
```
(A, head in numbers_sub, R.A)

Ctm_row
├─ label:     A
├─ condition: Membership
│             └─ set: Set_Ref  numbers_sub
└─ next_tm:   [R, A]
```
```
(>, _, R.L.A)

Ctm_row
├─ label:     >
├─ condition: WildCard
└─ next_tm:   [R, L, A]
```
```
(A, head != "4", "1".R.A)

Ctm_row
├─ label:     A
├─ condition: Equality  != "4"
└─ next_tm:   ["1", R, A]
```
```
(A, head == "1" | head == "2" & head != #, R.A)

Ctm_row
├─ label:     A
├─ condition: Logical  op: |
│             ├─ left:  Equality  == "1"
│             └─ right: Logical  op: &
│                       ├─ left:  Equality  == "2"
│                       └─ right: Equality  != #
└─ next_tm:   [R, A]
```

> `condition` is a `std::unique_ptr<Expr>`, so it can hold any condition
> node: `WildCard`, `Equality`, `Membership` or `Logical` (which combines the
> others with `&` / `|`).

**Statements** (one per declaration):

| Node | Fields |
|---|---|
| `Alphabet_def` | `token name`, `std::unique_ptr<Expr> symbols` |
| `State_set_def` | `token name`, `std::unique_ptr<Expr> states` |
| `Transition_fun` | `token name`, `states`, `alphabet` (each an `Expr`: `Set_Ref` or `Set_Literal`), `std::vector<Transition>` |
| `Turing_machine` | `token name`, `states`, `alphabet`, `token start`, `token func`, `Final_state` |
| `Tape_def` | `token name`, `alphabet`, `std::vector<token> cells` |
| `Composite_tm` | `token name`, `alphabet` (or null for `_`), `std::vector<Ctm_row>` |
| `Run` | `token machine`, `token tape` |

#### Statement examples

**`Alphabet_def`**
```
alphabet colors = {"green", "blue"};

Alphabet_def
├─ name:    colors
└─ symbols: Set_Literal  "green", "blue"
```

**`State_set_def`**
```
state_set Q1_sub = Q1 \ {"A", "D"};

State_set_def
├─ name:   Q1_sub
└─ states: Set_Binary  op: \
           ├─ left:  Set_Ref  Q1
           └─ right: Set_Literal  "A", "D"
```

**`Transition_fun`**
```
transition_func d1(Q1, numbers) = {
    ("A", "1", "B", "2", ->),
    ("C", "2") |- ("B", "9", ->)
};

Transition_fun
├─ name:        d1
├─ states:      Set_Ref  Q1
├─ alphabet:    Set_Ref  numbers
└─ transitions: [ Transition("A", "1", "B", "2", ->),
                  Transition("C", "2", "B", "9", ->) ]
```

**`Turing_machine`**
```
tm M1(Q1, numbers, "C", d1, {halt});

Turing_machine
├─ name:        M1
├─ states:      Set_Ref  Q1
├─ alphabet:    Set_Ref  numbers
├─ start:       "C"
├─ func:        d1
└─ final_state: Final_state  halts: [halt]
```

**`Tape_def`**
```
tape t1(numbers) = {~, "1", #, "2"};

Tape_def
├─ name:     t1
├─ alphabet: Set_Ref  numbers
└─ cells:    [~, "1", #, "2"]
```

**`Composite_tm`**
```
ctm skip_sub(numbers) = {
    (>, _, R.A),
    (A, head in numbers_sub, R.A),
    (A, _, halt) //like a default case
};

Composite_tm
├─ name:     skip_sub
├─ alphabet: Set_Ref  numbers          // null for ctm clean_up(_)
└─ rows:     [ Ctm_row(>, WildCard,                    [R, A]),
               Ctm_row(A, Membership(Set_Ref numbers_sub), [R, A]),
               Ctm_row(A, WildCard,                    [halt]) ]
```

**`Run`**
```
run(M1, t1);

Run
├─ machine: M1
└─ tape:    t1
```

- [ ] Update `visitor.h` to match the final list of node classes.
- [ ] Visitor methods return `void`. The book's visitors return values
      (`R visitBinaryExpr(...)`); in C++ the simplest replacement is to store
      the result in a member of the visitor (e.g. a `std::ostringstream` in the
      printer).

---

## Step 2 — AST printer
**Book:** ch. 5.4 *A (Not Very) Pretty Printer*

- [ ] Write `class ast_printer : public Visitor` that prints the tree in a
      Lisp-like form, e.g. `(alphabet numbers {"1" "2"})`,
      `(or (== head "1") (in head numbers_sub))`.
- [ ] Build a few nodes by hand in `main` and print them, before the parser
      exists — this tests Step 1 on its own (the book does exactly this in
      5.4).

---

## Step 3 — Program and declaration dispatch
**Book:** ch. 8.1 *Statements*, ch. 8.2.1 *Variable declarations*
(`declaration()` + `synchronize()`)

```
parse()        → while (!isAtEnd()) add declaration()
declaration()  → try { switch on keyword → *_def() } catch (ParseError) { synchronize(); return nullptr; }
```

- [ ] `parse()` collects statements and skips `nullptr`s.
- [ ] `declaration()` uses `match(...)` on the keyword and calls the matching
      function below. Anything else → `throw Perror(peek(), "Expect declaration.")`.
- [ ] Every declaration function ends with
      `consume(SEMICOLON, "Expect ';' after ... declaration.")`.

---

## Step 4 — Small helper functions
**Book:** ch. 6.2 (`primary()` is the model for these)

These are reused by several declarations. Write them first:

| Function | Grammar | Returns |
|---|---|---|
| `string_set()` | `"{" STRING ( "," STRING )* "}"` | `std::unique_ptr<Expr>` (`Set_Literal`) |
| `set_or_ref()` | `IDENTIFIER \| string_set` | `std::unique_ptr<Expr>` (`Set_Ref` or `Set_Literal`) |
| `symbol()` | `STRING \| "#" \| "~"` | `token` |
| `move()` | `"->" \| "<-" \| "<>"` | `token` |
| `next_state()` | `STRING \| "halt" \| "halt_yes" \| "halt_no"` | `token` |

`symbol()`, `move()` and `next_state()` all follow the same pattern:
`if (match({...})) return previous(); throw Perror(peek(), "Expect ...");`

---

## Step 5 — Declarations (easiest first)

### 5.1 `alphabet_def` and `run`
**Book:** ch. 8.2.1

- [ ] `alphabet_def()`: `consume(IDENTIFIER)`, `consume(EQUAL)`, `string_set()`, `;`
- [ ] `run_stmt()`: `( IDENTIFIER , IDENTIFIER ) ;`

### 5.2 `state_set_def` — your first binary expression
**Book:** ch. 6.2, the `term()` / `factor()` loop

```
state_set_expr → set_or_ref ( ( "\/" | "\" ) set_or_ref )*
```

- [ ] Same loop as the book's `term()`:
      `expr = set_or_ref(); while (match({UNION, DIFFERENCE})) { op = previous(); right = set_or_ref(); expr = Set_Binary(expr, op, right); }`
- [ ] This is left-associative automatically, as the grammar requires.

### 5.3 `tape_def`
- [ ] `( set_or_ref ) = {`, then `consume(BLIP, "Tape must start with '~'.")`,
      then `( "," (STRING | "#") )*`, then `} ;`

### 5.4 `turing_machine`
- [ ] Arguments in order: `set_or_ref , set_or_ref , STRING , IDENTIFIER , final_state`
- [ ] `final_state()`: after `{`, if `match(HALT)` → expect `}`;
      else if `match(HALT_YES)` → expect `,` `HALT_NO` `}`;
      else error `"Expect 'halt' or 'halt_yes, halt_no'."`

### 5.5 `transition_function` — the two transition forms
- [ ] `transition()`: parse `( STRING , symbol`, then look at the next token:
  - `COMMA` → 5-tuple form: `next_state , symbol , move )`
  - `RIGHT_PAREN` → `|-` form: `) |- ( next_state , symbol , move )`
  - anything else → error
- [ ] This is one token of lookahead (`check(COMMA)`), so no backtracking is
      needed.

### 5.6 `composite_tm` — conditions are the precedence chain
**Book:** ch. 6.2 (precedence levels), ch. 9.3 *Logical Operators*

```
condition   → "_" | or_logical
or_logical  → and_logical ( "|" and_logical )*
and_logical → equality ( "&" equality )*
equality    → "head" ( ( "==" | "!=" ) symbol | "in" set_or_ref )
```

- [ ] `condition()`: `if (match({UNDERSCORE})) return WildCard;` else `or_logical()`.
- [ ] `or_logical()` / `and_logical()`: the same loop as 5.2, producing
      `Logical` nodes (book's `or()` / `and()` in 9.3).
- [ ] `equality()`: `consume(HEAD)`, then `==`/`!=` + `symbol()` →
      `Equality`, or `in` + `set_or_ref()` → `Membership`.
- [ ] `ctm_row()`: `( (IDENTIFIER | START) , condition , next_tm )`
- [ ] `next_tm()`: one atom, then `( "." atom )*`. An atom is `IDENTIFIER`,
      `symbol()` or a halt state.
- [ ] CTM parameter: `IDENTIFIER | string_set | UNDERSCORE`.

---

## Step 6 — Wire it up and test
**Book:** ch. 6.4 *Wiring up the Parser*

- [ ] In `tm.cpp` `run()`: scanner → `parser(tokens).parse()` → if
      `had_error` stop, else print every statement with `ast_printer`.
- [ ] Run the three test files. Expected results:
  - `test.tm` — error at the end (`transition_func` is incomplete), and the
    three declarations before it still print (shows `synchronize` works).
  - `test2.tm` — error at `std::left` (no grammar rule yet, as decided).
  - `test3.tm` — parses fully.
- [ ] Add error tests: missing `;`, tape without `~`, `{halt_yes}` alone,
      transition with 4 elements, condition without `head`. Each should give
      one clear error, and the parser should continue to the next
      declaration.
- [ ] Optional: register the `.tm` files with `add_test(...)` in
      `CMakeLists.txt` so `ctest` actually runs something (it currently has no
      tests).

---

## Not in the parser (later, in the resolver — book ch. 11)

The grammar cannot express these rules. Leave them out of the parser and check
them in a separate pass over the AST:

- Names are declared before use, and have the right kind (e.g. a TM's 4th
  argument is a `transition_func`).
- Transition states are in the state set; symbols are in the alphabet,
  `#` or `~`.
- No two transitions with the same `(state, read)`.
- A transition that reads `~` writes `~` and moves `->`; `~` is never written
  anywhere else.
- A TM's transitions only enter the halt states its `final_state` declares.
- Tape cells are in the tape's alphabet or `#`.
- A CTM has exactly one `>` row, which uses `_`; a `_` row is the last row for
  its label; every jump label exists.
