# Feynman -- quantum circuit toolkit
# Original (/home/alan23273850/libvata2/benchmarks/MCToffoli/10/circuit.qc):
#   Qubits: 20
#   H: 36
#   T: 126
#   cnot: 109
#   Depth: 137
#   T depth: 56
# Result (0.714ms):
#   Qubits: 20
#   H: 36
#   T: 126
#   cnot: 109
#   Depth: 137
#   T depth: 56
.v q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19
.i q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19
.o q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19
BEGIN 
tof q0 q1 q2
tof q3 q2 q4
tof q5 q4 q6
tof q7 q6 q8
tof q9 q8 q10
tof q11 q10 q12
tof q13 q12 q14
tof q15 q14 q16
tof q17 q16 q18
tof q18 q19
tof q17 q16 q18
tof q15 q14 q16
tof q13 q12 q14
tof q11 q10 q12
tof q9 q8 q10
tof q7 q6 q8
tof q5 q4 q6
tof q3 q2 q4
tof q0 q1 q2
END
