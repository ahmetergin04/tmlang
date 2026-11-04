// A Turing machine that scans to the right until it finds two consecutive 'a's and then halts.

alphabet a = { "a", "b"};

state_set Q = { "s", "q" };

// The head starts on the # after ~ (start convention), so "s" first reads #.
// No transition for ~: the default rule ("s", ~) |-- ("s", ~, ->) applies.
transition_func d(Q,a) = {
    ("s", #)   |-- ("s",  #,   ->),
    ("s", "a") |-- ("q",  "a", ->),
    ("s", "b") |-- ("s",  "b", ->),
    ("q", #)   |-- ("s",  #,   ->),
    ("q", "a") |-- (halt, "a", <>),
    ("q", "b") |-- ("s",  "b", ->)
};

tm M (Q,a,"s",d, {halt});

tape t(a) = { ~, #, "a", "b", "a", "b", "a", "b", "a", "b", "b", "b", "b", "a", "a" };

run(M, t);
