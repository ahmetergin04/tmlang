// Interpreter tests: sets, transition functions, TMs, tapes and runs.
// CTMs are not interpreted yet, so none are declared here.
// Expected output of each run is written above it.

// ---- Alphabets (literal only) ----
alphabet bits   = {"0", "1"};
alphabet colors = {"green", "blue"};

// ---- State sets: literal, reference, difference, union, chain ----
state_set Base        = {"start", "flip", "back", "done", "junk"};
state_set Flip_states = Base \ {"junk"};                        // {"back","done","flip","start"}
state_set Extra       = {"s1", "s2"};
state_set All         = Flip_states \/ Extra;                    // union of two refs
state_set Chain       = Base \ {"junk", "done"} \/ {"done"};     // left to right: == Flip_states
state_set Copy        = Chain;                                   // plain reference

// ---- Transition functions ----

// Flips every bit, walks back to ~ and halts on cell 1.
// Uses the chained set (if the chain is evaluated wrong, "done" is missing
// and this declaration fails) and an explicit ~ transition that overrides
// the default rule.
transition_func d_flip(Chain, bits) = {
    ("start", #)   |-- ("flip", #,   ->),
    ("flip",  "0") |-- ("flip", "1", ->),
    ("flip",  "1") |-- ("flip", "0", ->),
    ("flip",  #)   |-- ("back", #,   <-),
    ("back",  "0") |-- ("back", "0", <-),
    ("back",  "1") |-- ("back", "1", <-),
    ("back",  #)   |-- ("back", #,   <-),
    ("back",  ~)   |-- ("done", ~,   ->),
    ("done",  #)   |-- (halt,   #,   <>)
};

// Moves left onto ~ with no ~ transition: the default rule keeps state "s2"
// and moves right, then "s2" halts writing "1".
transition_func d_left(All, bits) = {
    ("s1", #) |-- ("s2", #,   <-),
    ("s2", #) |-- (halt, "1", <>)
};

// Deciding function; both parameters are literals.
transition_func d_color({"x", "y"}, {"green", "blue"}) = {
    ("x", #)       |-- ("y",      #,       ->),
    ("y", "green") |-- (halt_yes, "green", <>),
    ("y", "blue")  |-- (halt_no,  "blue",  <>)
};

// Bounces between cells 1 and 2 forever.
transition_func d_bounce({"go", "come"}, bits) = {
    ("go",   #) |-- ("come", #, ->),
    ("come", #) |-- ("go",   #, <-)
};

// ---- Turing machines: ref and literal parameters, both final-state kinds ----
tm Flipper(Copy, bits, "start", d_flip, {halt});
tm Lefty(All, bits, "s1", d_left, {halt});
tm Decider({"x", "y"}, colors, "x", d_color, {halt_yes, halt_no});
tm Bouncer({"go", "come"}, {"0", "1"}, "go", d_bounce, {halt});

// ---- Tapes ----
tape t_bits(bits)    = {~, #, "1", "0", "1"};
tape t_empty(bits)   = {~, #};
tape t_green(colors) = {~, #, "green"};
tape t_blue(colors)  = {~, #, "blue"};
tape t_gap({"green", "blue"}) = {~, #, #, "blue"};   // literal alphabet, blank inside the input

// ---- Runs ----

// halt after 11 steps
// ~ [#] "0" "1" "0"
run(Flipper, t_bits);

// Same tape again: runs work on a copy, so the result is identical.
// halt after 11 steps
// ~ [#] "0" "1" "0"
run(Flipper, t_bits);

// Empty input: the tape grows to the right.
// halt after 5 steps
// ~ [#]
run(Flipper, t_empty);

// Default ~ rule.
// halt after 3 steps
// ~ ["1"]
run(Lefty, t_empty);

// halt_yes after 2 steps
// ~ # ["green"]
run(Decider, t_green);

// halt_no after 2 steps
// ~ # ["blue"]
run(Decider, t_blue);

// No transition for ("y", #).
// stuck in stop_state "y" reading # after 1 steps
// ~ # [#] "blue"
run(Decider, t_gap);

// Step limit.
// did not halt within 10000 steps (stop_state "go")
// ~ [#]
run(Bouncer, t_empty);

// ---- Errors ----
// The interpreter stops at the first runtime error, so these are commented
// out. Uncomment ONE at a time; each should report the error shown.

// already defined:
// alphabet bits = {"0"};

// Undefined set 'Nope':
// state_set Bad = Nope \/ {"a"};

// Undefined set 'bits' (an alphabet used where a state set belongs):
// tm Swapped(bits, Copy, "start", d_flip, {halt});

// State is not in the state set:
// transition_func e1({"a"}, bits) = { ("b", "0") |-- ("a", "0", ->) };

// Symbol is not in the alphabet:
// transition_func e2({"a"}, bits) = { ("a", "2") |-- ("a", "0", ->) };

// Next state is not in the state set:
// transition_func e3({"a"}, bits) = { ("a", "0") |-- ("b", "0", ->) };

// Reading '~' must write '~' and move '->':
// transition_func e4({"a"}, bits) = { ("a", ~) |-- ("a", ~, <-) };

// '~' cannot be written:
// transition_func e5({"a"}, bits) = { ("a", "0") |-- ("a", ~, ->) };

// Duplicate transition for this (state, symbol):
// transition_func e6({"a"}, bits) = { ("a", "0") |-- ("a", "0", ->), ("a", "0") |-- ("a", "1", ->) };

// Start state is not in the state set:
// tm e7(Copy, bits, "nowhere", d_flip, {halt});

// Undefined transition function 'd_none':
// tm e8(Copy, bits, "start", d_none, {halt});

// Transition enters 'halt', which this machine does not declare:
// tm e9(Copy, bits, "start", d_flip, {halt_yes, halt_no});

// Transition enters 'halt_no', which this machine does not declare
// (the first offending transition in map order is ("y","blue")):
// tm e10({"x", "y"}, colors, "x", d_color, {halt});

// Tape symbol is not in the alphabet:
// tape e11(bits) = {~, #, "green"};

// Undefined machine 'Ghost':
// run(Ghost, t_bits);

// Undefined tape 't_ghost':
// run(Flipper, t_ghost);
