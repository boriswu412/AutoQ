# Feynman -- quantum circuit toolkit
# Original (./10/circuit.qc):
#   Qubits: 11
#   H: 22
#   Z: 1
#   cnot: 5
#   Depth: 8
#   T depth: 0
# Result (0.756ms):
#   Qubits: 11
#   H: 22
#   Z: 1
#   cnot: 5
#   Depth: 8
#   T depth: 0
.v q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10
.i q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10
.o q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10
BEGIN 
H q0
H q1
H q2
H q3
H q4
H q5
H q6
H q7
H q8
H q9
H q10
Z q10
tof q0 q10
tof q2 q10
tof q4 q10
tof q6 q10
tof q8 q10
H q0
H q1
H q2
H q3
H q4
H q5
H q6
H q7
H q8
H q9
H q10
END
