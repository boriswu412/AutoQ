OPENQASM 2.0;
include "qelib1.inc";
qreg qubits[63];

cx qubits[15], qubits[41];
cz qubits[45], qubits[10];
cx qubits[54], qubits[25];
ry(pi/2) qubits[55];
t qubits[16];
ry(pi/2) qubits[44];
s qubits[45];
z qubits[56];
t qubits[51];
z qubits[18];
h qubits[31];
ry(pi/2) qubits[43];
z qubits[29];
s qubits[4];
h qubits[29];
y qubits[62];
t qubits[51];
t qubits[1];
t qubits[46];
cx qubits[4], qubits[60];
z qubits[46];
h qubits[11];
rx(pi/2) qubits[28];
cx qubits[39], qubits[19];
z qubits[22];
x qubits[19];
z qubits[39];
s qubits[16];
h qubits[57];
x qubits[19];
rx(pi/2) qubits[53];
t qubits[21];
h qubits[18];
x qubits[43];
ry(pi/2) qubits[62];
x qubits[1];
x qubits[19];
z qubits[61];
h qubits[29];
rx(pi/2) qubits[42];
cz qubits[1], qubits[61];
cz qubits[40], qubits[45];
z qubits[16];
x qubits[45];
ry(pi/2) qubits[28];
z qubits[39];
t qubits[22];
cz qubits[50], qubits[0];
t qubits[48];
s qubits[11];
y qubits[27];
s qubits[15];
x qubits[28];
h qubits[27];
rx(pi/2) qubits[32];
t qubits[0];
y qubits[4];
rx(pi/2) qubits[21];
ccx qubits[45], qubits[54], qubits[7];
ry(pi/2) qubits[29];
rx(pi/2) qubits[38];
x qubits[43];
t qubits[32];
x qubits[45];
t qubits[45];
s qubits[44];
ry(pi/2) qubits[55];
rx(pi/2) qubits[27];
ry(pi/2) qubits[21];
t qubits[40];
h qubits[1];
cx qubits[21], qubits[22];
t qubits[2];
y qubits[29];
rx(pi/2) qubits[40];
z qubits[54];
z qubits[60];
cx qubits[29], qubits[25];
s qubits[9];
cx qubits[17], qubits[52];
z qubits[8];
y qubits[29];
ry(pi/2) qubits[20];
z qubits[1];
z qubits[24];
y qubits[61];
h qubits[4];
ry(pi/2) qubits[55];
h qubits[16];
s qubits[50];
t qubits[30];
t qubits[34];
cx qubits[8], qubits[44];
rx(pi/2) qubits[12];
t qubits[57];
cx qubits[7], qubits[54];
ry(pi/2) qubits[31];
x qubits[62];
h qubits[48];
z qubits[12];
ccx qubits[2], qubits[16], qubits[42];
rx(pi/2) qubits[45];
rx(pi/2) qubits[28];
h qubits[62];
h qubits[14];
s qubits[52];
rx(pi/2) qubits[14];
s qubits[31];
h qubits[50];
ccx qubits[18], qubits[41], qubits[22];
t qubits[50];
h qubits[16];
t qubits[43];
x qubits[15];
t qubits[43];
cz qubits[38], qubits[24];
t qubits[1];
rx(pi/2) qubits[48];
cz qubits[4], qubits[18];
ry(pi/2) qubits[22];
cx qubits[39], qubits[53];
y qubits[26];
ry(pi/2) qubits[58];
cx qubits[54], qubits[43];
x qubits[12];
cx qubits[22], qubits[53];
rx(pi/2) qubits[35];
ccx qubits[0], qubits[28], qubits[7];
rx(pi/2) qubits[14];
rx(pi/2) qubits[14];
t qubits[1];
x qubits[15];
y qubits[38];
ccx qubits[24], qubits[27], qubits[22];
y qubits[61];
rx(pi/2) qubits[50];
cx qubits[59], qubits[39];
ccx qubits[51], qubits[59], qubits[10];
cx qubits[20], qubits[10];
z qubits[27];
s qubits[7];
cx qubits[44], qubits[4];
x qubits[60];
cx qubits[17], qubits[33];
ry(pi/2) qubits[39];
rx(pi/2) qubits[18];
cz qubits[58], qubits[34];
t qubits[45];
ry(pi/2) qubits[48];
cx qubits[16], qubits[42];
ccx qubits[24], qubits[11], qubits[51];
ry(pi/2) qubits[4];
z qubits[3];
t qubits[46];
h qubits[15];
cz qubits[42], qubits[32];
rx(pi/2) qubits[13];
h qubits[8];
s qubits[46];
y qubits[2];
cx qubits[41], qubits[47];
s qubits[24];
cx qubits[57], qubits[3];
ry(pi/2) qubits[19];
x qubits[44];
h qubits[17];
ccx qubits[55], qubits[27], qubits[41];
t qubits[42];
s qubits[15];
s qubits[25];
rx(pi/2) qubits[38];
z qubits[34];
t qubits[0];
s qubits[34];
h qubits[19];
z qubits[0];
x qubits[6];
ccx qubits[38], qubits[23], qubits[14];
z qubits[12];
z qubits[14];
t qubits[57];
ry(pi/2) qubits[37];
ry(pi/2) qubits[46];
t qubits[44];
ccx qubits[15], qubits[19], qubits[39];
cz qubits[38], qubits[39];
x qubits[9];
y qubits[36];
ccx qubits[13], qubits[18], qubits[26];
