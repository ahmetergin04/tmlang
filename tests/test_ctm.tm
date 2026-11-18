// CTM tests: declarations, alphabet rules (Alphabet rules.md) and runs.
// Expected output of each run is written above it.

// ---- Alphabets ----
alphabet numbers     = {"1", "2", "3", "9"};
alphabet numbers_sub = {"1", "2", "3"};
alphabet colors      = {"green", "blue"};

// ---- TMs used inside CTM chains ----

// Bumps the symbol under the head: 1 -> 2 -> 3 -> 9. Stuck on #.
transition_func d_bump({"s"}, numbers) = {
    ("s", "1") |-- (halt, "2", <>),
    ("s", "2") |-- (halt, "3", <>),
    ("s", "3") |-- (halt, "9", <>),
    ("s", "9") |-- (halt, "9", <>)
};
tm Bump({"s"}, numbers, "s", d_bump, {halt});

// A machine over the smaller alphabet (for the C1 error case).
transition_func d_noop({"s"}, numbers_sub) = { ("s", #) |-- (halt, #, <>) };
tm Noop_sub({"s"}, numbers_sub, "s", d_noop, {halt});

// ---- Tapes ----
tape t1(numbers)          = {~, #, "1", "2", "3", "3"};
tape t_nums(numbers)      = {~, #, "1", "9", "3"};
tape t_sub(numbers_sub)   = {~, #, "1", "2", "3"};
tape t_empty(numbers_sub) = {~, #};
tape t_colors(colors)     = {~, #, "green", "blue"};

// ---- Typed CTMs ----

// From the grammar doc: skip every symbol in numbers_sub, then halt.
// Uses: head in (Membership), | (Logical OR), head == # (G2).
ctm skip_sub(numbers) = {
    (>, _, R.A),
    (A, head in numbers_sub, R.A),
    (A, head == "9" | head == #, halt)
};

// Calls a TM in a chain (C1: numbers is a subset of Bump's alphabet).
// Ends in halt_yes: a CTM can end in any halt state.
ctm bump_all(numbers) = {
    (>, _, R.A),
    (A, head in numbers_sub, Bump.R.A),
    (A, head == "9", R.A),
    (A, head == #, halt_yes)
};

// Calls a typed CTM (C2), writes a symbol (C4), uses & and |.
// bump_all ends in halt_yes, but a called machine's halt state is ignored:
// the chain just continues.
ctm reset_nines(numbers) = {
    (>, _, bump_all.L.B),
    (B, head == "9" & head != "1", "1".L.B),
    (B, head == "2" | head == "3", L.B),
    (B, head == #, halt)
};

// Rows of one label are tried top to bottom; "2" has no row, so it gets stuck.
ctm only_ones(numbers) = {
    (>, _, R.A),
    (A, head == "1", R.A),
    (A, head == #, halt)
};

// A TM that gets stuck inside a chain ends the whole run.
ctm bad_bump(numbers) = {
    (>, _, Bump.halt)
};

// A typed CTM over the smaller alphabet (for the C2 and R2 cases).
ctm step_sub(numbers_sub) = {
    (>, _, R.halt)
};

// ---- _ CTMs (any alphabet) ----

// Move right until a blank.
ctm to_end(_) = {
    (>, _, R.A),
    (A, head != #, R.A),
    (A, head == #, halt)
};

// Go to the end, then erase leftwards. Calls another _ CTM (U4) and writes # (U1).
ctm erase_back(_) = {
    (>, _, to_end.L.A),
    (A, head != #, #.L.A),
    (A, head == #, halt)
};

// Jumping to a row counts as a step, so this hits the step limit.
ctm spin(_) = {
    (>, _, A),
    (A, head == #, A)
};

// L on ~ moves right (like every machine reading ~).
ctm left_wall(_) = {
    (>, _, L.L.L.halt)
};

// ---- Runs ----

// halt after 10 steps
// ~ # "1" "2" "3" "3" [#]
run(skip_sub, t1);

// halt_yes after 10 steps
// ~ # "2" "9" "9" [#]
run(bump_all, t_nums);

// bump_all gives ~ # "2" "9" "9" #, then reset_nines walks back
// turning every "9" into "1".
// halt after 20 steps
// ~ [#] "2" "1" "1"
run(reset_nines, t_nums);

// stuck in stop_state A reading "2" after 4 steps
// ~ # "1" ["2"] "3" "3"
run(only_ones, t1);

// stuck in stop_state "s" reading # after 0 steps
// ~ [#] "1" "2" "3" "3"
run(bad_bump, t1);

// R3: a _ CTM runs on any tape.
// halt after 14 steps
// ~ [#]
run(erase_back, t_colors);

// R2: numbers_sub is a subset of numbers.
// halt after 8 steps
// ~ # "1" "2" "3" [#]
run(skip_sub, t_sub);

// did not halt within 10000 steps (stop_state A)
// ~ [#]
// run(spin, t_empty);

// cell 1 -> L to ~ -> L moves right -> L to ~ again
// halt after 3 steps
// [~]
run(left_wall, t_empty);

// ---- Errors ----
// The interpreter stops at the first runtime error, so these are commented
// out. Uncomment ONE at a time; each should report the error shown.

// R1 The tape's alphabet is not a subset of the alphabet of 'Bump':
// run(Bump, t_colors);

// R2 The tape's alphabet is not a subset of the alphabet of 'step_sub':
// run(step_sub, t1);

// C1 The CTM's alphabet is not a subset of the alphabet of 'Noop_sub':
// ctm e1(numbers) = { (>, _, Noop_sub.halt) };

// C2 The CTM's alphabet is not a subset of the alphabet of 'step_sub':
// ctm e2(numbers) = { (>, _, step_sub.halt) };

// C4 Written symbol is not in the CTM's alphabet:
// ctm e3(numbers_sub) = { (>, _, "9".halt) };

// C4 Written symbol is not in the CTM's alphabet (~ is never in an alphabet):
// ctm e4(numbers) = { (>, _, ~.halt) };

// C5 Symbol is not in the CTM's alphabet:
// ctm e5(numbers_sub) = { (>, _, R.A), (A, head == "9", halt) };

// C6 Set is not a subset of the CTM's alphabet:
// ctm e6(numbers_sub) = { (>, _, R.A), (A, head in numbers, halt) };

// C7 Undefined machine or row label 'Ghost':
// ctm e7(numbers) = { (>, _, Ghost.halt) };

// C7 Undefined machine or row label 'e8' (a CTM cannot call itself):
// ctm e8(_) = { (>, _, e8.halt) };

// U1 A '_' CTM can only write '#':
// ctm e9(_) = { (>, _, "1".halt) };

// U2 A '_' CTM can only compare the head with '#':
// ctm e10(_) = { (>, _, R.A), (A, head == "1", halt) };

// U3 A '_' CTM cannot use 'head in':
// ctm e11(_) = { (>, _, R.A), (A, head in numbers, halt) };

// U4 A '_' CTM can only call R, L and other '_' CTMs (a TM):
// ctm e12(_) = { (>, _, Bump.halt) };

// U4 A '_' CTM can only call R, L and other '_' CTMs (a typed CTM):
// ctm e13(_) = { (>, _, skip_sub.halt) };

// A chain must end with a halt state or a row label:
// ctm e14(_) = { (>, _, R.L) };

// A halt state must be the last step of a chain:
// ctm e15(_) = { (>, _, halt.R) };

// A row label must be the last step of a chain:
// ctm e16(_) = { (>, _, A.R), (A, head == #, halt) };

// Row label 'Bump' has the same name as a machine:
// ctm e17(numbers) = { (>, _, R.Bump), (Bump, head == #, halt) };

// 'R' is a built-in machine:
// ctm R(_) = { (>, _, halt) };

// 'Bump' is already defined (TMs and CTMs share names):
// ctm Bump(_) = { (>, _, halt) };
