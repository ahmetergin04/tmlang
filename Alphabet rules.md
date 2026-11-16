General

- [ ] G1. Compare alphabets by their resolved contents (eval_set), not by name. numbers and {"1","2","3","4"} are equal.
- [ ] G2. # is implicitly in every alphabet. Skip it in every check below.

Run

- [ ] R1. run(TM, tape): tape alphabet ⊆ TM alphabet.
- [ ] R2. run(CTM, tape): tape alphabet ⊆ CTM alphabet.
- [ ] R3. run(CTM, tape) where the CTM is _: no alphabet check.

Typed CTM declaration (alphabet is a name or a set literal)

- [ ] C1. Each TM called in a chain: CTM alphabet ⊆ that TM's alphabet.
- [ ] C2. Each typed CTM called in a chain: CTM alphabet ⊆ that CTM's alphabet.
- [ ] C3. Each _ CTM, R or L called in a chain: always allowed.
- [ ] C4. Each symbol written in a chain ("1".R.A): symbol ∈ CTM alphabet.
- [ ] C5. head == x and head != x: x ∈ CTM alphabet.
- [ ] C6. head in S: S ⊆ CTM alphabet.
- [ ] C7. Every machine called in a chain must already be declared (declare-before-use).

_ CTM declaration

- [ ] U1. Written symbols in chains: only #.
- [ ] U2. head == x and head != x: x must be #.
- [ ] U3. head in S: not allowed, since any S other than # names specific symbols.
- [ ] U4. Chain calls: only R, L, or other _ CTMs. Calling a TM or a typed CTM is an error.
- [ ] U5. C7 (declare-before-use) applies here too.
