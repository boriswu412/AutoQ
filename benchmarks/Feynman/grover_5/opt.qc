# Feynman -- quantum circuit toolkit
# Original (/home/alan23273850/libvata2/benchmarks/Feynman/grover_5/circuit.qc):
#   Qubits: 9
#   H: 142
#   T: 336
#   X: 65
#   cnot: 288
#   Depth: 389
#   T depth: 152
# Result (3.559ms):
#   Qubits: 9
#   H: 142
#   T: 336
#   X: 65
#   cnot: 288
#   Depth: 389
#   T depth: 152
.v a b c d e result 0 1 2
.i 
.o a b c d e result 0 1 2
BEGIN 
X result
H a
H b
H c
H d
H e
H result
X e
X a
X b
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H 2
Z d 1 2
H 2
H result
Z e 2 result
H result
H 2
Zd d 1 2
H 2
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
H a
H b
H c
H d
H e
X e
X a
X b
X c
X d
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H e
Z d 1 e
H e
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
X c
X d
H a
H b
H c
H d
H e
X e
X a
X b
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H 2
Z d 1 2
H 2
H result
Z e 2 result
H result
H 2
Zd d 1 2
H 2
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
H a
H b
H c
H d
H e
X e
X a
X b
X c
X d
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H e
Z d 1 e
H e
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
X c
X d
H a
H b
H c
H d
H e
X e
X a
X b
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H 2
Z d 1 2
H 2
H result
Z e 2 result
H result
H 2
Zd d 1 2
H 2
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
H a
H b
H c
H d
H e
X e
X a
X b
X c
X d
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H e
Z d 1 e
H e
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
X c
X d
H a
H b
H c
H d
H e
X e
X a
X b
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H 2
Z d 1 2
H 2
H result
Z e 2 result
H result
H 2
Zd d 1 2
H 2
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
H a
H b
H c
H d
H e
X e
X a
X b
X c
X d
H 0
Z a b 0
H 0
H 1
Z c 0 1
H 1
H e
Z d 1 e
H e
H 1
Zd c 0 1
H 1
H 0
Zd a b 0
H 0
X e
X a
X b
X c
X d
H a
H b
H c
H d
H e
END
