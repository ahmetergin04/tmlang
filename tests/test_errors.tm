// Parser error tests: each broken declaration must give exactly one error,
// and the parser must recover and continue with the next declaration.
// Nothing is printed except the errors (book behaviour), and the exit code is 65.
// The expected error is written in the comment above each case.

alphabet a = {"a", "b"};
state_set Q = {"s", "q"};

// 1. Missing ';'. The error is reported at the next keyword, and that
//    declaration (state_set Q2) is still parsed normally.
// [line 13] Error  at 'state_set': Expect ';' after alphabet declaration.
alphabet missing_semicolon = {"x"}
state_set Q2 = {"p"};

// 2. Tape without '~'.
// [line 17] Error  at '#': Expect '~' to start a tape set.
tape no_blip(a) = {#, "a"};

// 3. Tape without '#' after '~' (start convention).
// [line 21] Error  at '"a"': Expect '#' after '~' in tape set.
tape no_blank(a) = {~, "a"};

// 4. '{halt_yes}' alone: a deciding machine needs both halt states.
// [line 25] Error  at '}': Expect ',' after 'halt_yes'.
tm only_yes(Q, a, "s", d, {halt_yes});

// 5. Transition with 4 elements: the move is missing.
// [line 30] Error  at ')': Expect ',' after written symbol.
transition_func d(Q, a) = {
    ("s", "a") |-- ("q", "a")
};

// 6. Condition without 'head'.
// [line 37] Error  at '==': Expect 'head' in condition.
ctm no_head(a) = {
    (>, _, R.A),
    (A, == "a", R.A)
};

// 7. '_' outside the '>' row.
// [line 44] Error  at '_': '_' is only allowed in the '>' row.
ctm late_wildcard(a) = {
    (>, _, R.A),
    (A, _, halt)
};

// 8. The first row is not the '>' row.
// [line 50] Error  at 'A': The first ctm row must be the '>' row.
ctm no_start(a) = {
    (A, head == "a", R.A)
};

// A correct declaration at the end: parsed, but not printed because of the
// errors above.
tape t(a) = {~, #, "a", "b"};
