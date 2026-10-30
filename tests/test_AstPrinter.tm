// AST printer test: covers every AST node at least once.
// The comment above each declaration is what ast_printer should print for it.

// (alphabet numbers {"1" "2" "3" "4"})
alphabet numbers = {"1", "2", "3", "4"};

// (alphabet numbers_sub {"1" "2"})
alphabet numbers_sub = {"1", "2"};

// (state_set Q1 {"A" "B" "C" "D"})
state_set Q1 = {"A", "B", "C", "D"};

// Set_Binary is left-associative: the first operator ends up deepest.
// (state_set Q1_sub (\/ (\ Q1 {"A" "D"}) {"E"}))
state_set Q1_sub = Q1 \ {"A", "D"} \/ {"E"};

// Both transition forms print the same way.
// (transition_func d1 Q1 numbers
//   ("A" "1" "B" "2" ->)
//   ("B" # "C" # <-)
//   ("C" "2" "B" "9" ->)
//   ("B" "3" halt "3" <>))
transition_func d1(Q1, numbers) = {
    ("A", "1", "B", "2", ->),
    ("B", #, "C", #, <-),
    ("C", "2") |- ("B", "9", ->),
    ("B", "3") |- (halt, "3", <>)
};

// Set literals as arguments instead of names.
// (transition_func d2 {"s" "q"} {"a" "b"}
//   ("s" ~ "s" ~ ->)
//   ("s" "a" halt_yes "a" <>)
//   ("s" "b" halt_no "b" <>))
transition_func d2({"s", "q"}, {"a", "b"}) = {
    ("s", ~, "s", ~, ->),
    ("s", "a", halt_yes, "a", <>),
    ("s", "b") |- (halt_no, "b", <>)
};

// Computing machine: one halt state.
// (tm M1 Q1 numbers "A" d1 {halt})
tm M1(Q1, numbers, "A", d1, {halt});

// Deciding machine: two halt states.
// (tm M2 {"s" "q"} {"a" "b"} "s" d2 {halt_yes halt_no})
tm M2({"s", "q"}, {"a", "b"}, "s", d2, {halt_yes, halt_no});

// (tape t1 numbers [~ "1" # "2" "3"])
tape t1(numbers) = {~, "1", #, "2", "3"};

// (tape t2 {"a" "b"} [~ "a" "b"])
tape t2({"a", "b"}) = {~, "a", "b"};

// Conditions: WildCard, Membership, Equality, and Logical
// (& binds tighter than |).
// (ctm skip_sub numbers
//   (> _ R.A)
//   (A (in head numbers_sub) R.A)
//   (A (in head {"3"}) "1".R.A)
//   (A (or (== head "4") (and (!= head #) (!= head "1"))) L.A)
//   (A _ halt))
ctm skip_sub(numbers) = {
    (>, _, R.A),
    (A, head in numbers_sub, R.A),
    (A, head in {"3"}, "1".R.A),
    (A, head == "4" | head != # & head != "1", L.A),
    (A, _, halt)
};

// CTM with "_" instead of an alphabet.
// (ctm clean_up _
//   (> _ #.R.B)
//   (B (== head #) halt)
//   (B _ L.B))
ctm clean_up(_) = {
    (>, _, #.R.B),
    (B, head == #, halt),
    (B, _, L.B)
};

// (run M1 t1)
run(M1, t1);

// (run M2 t2)
run(M2, t2);
