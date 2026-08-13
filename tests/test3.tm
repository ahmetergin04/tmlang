// A Turing machine that scans to the right until it finds two consecutive 'a's and then halts.

alphabet a = { "a", "b"};

state_set Q = { "s", "q" };

transition_func d(Q,a) = { 
    ("s", "#", "s", ->),
    ("s","a","q", ->),
    ("s","b","s", ->),
    ("q","#","s", ->),
    ("q","a","halt","a"),
    ("q","b","s", ->)
};

TM M (Q,a,d,"s", {"halt"});

tape t(a) = { "#ababababbbbaa"};

run(M, t);
