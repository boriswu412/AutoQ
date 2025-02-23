Numbers
c0 := (-1 + 0 * A(1/8) + 0 * A(2/8) + 0 * A(3/8)) / (V2 ^ 2)
c1 := (0 + 0 * A(1/8) + 0 * A(2/8) + 0 * A(3/8)) / (V2 ^ 0)
c2 := (1 + 0 * A(1/8) + -1 * A(2/8) + 0 * A(3/8)) / (V2 ^ 4)
c3 := (1 + 0 * A(1/8) + 0 * A(2/8) + 0 * A(3/8)) / (V2 ^ 2)
c4 := (1 + 0 * A(1/8) + 1 * A(2/8) + 0 * A(3/8)) / (V2 ^ 4)
Transitions
[1](1, 2) -> 0
[2](3, 4) -> 1
[2](5, 4) -> 2
[3](6, 7) -> 5
[3](8, 9) -> 4
[3](10, 7) -> 3
[c0] -> 6
[c1] -> 8
[c2] -> 9
[c3] -> 10
[c4] -> 7
Constraints