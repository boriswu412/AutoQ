# Feynman -- quantum circuit toolkit
# Original (/home/alan23273850/libvata2/benchmarks/MCToffoli/77/circuit.qc):
#   Qubits: 154
#   H: 304
#   T: 1064
#   cnot: 913
#   Depth: 1142
#   T depth: 458
# Result (3.693ms):
#   Qubits: 154
#   H: 304
#   T: 1064
#   cnot: 913
#   Depth: 1142
#   T depth: 458
.v q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19 q20 q21 q22 q23 q24 q25 q26 q27 q28 q29 q30 q31 q32 q33 q34 q35 q36 q37 q38 q39 q40 q41 q42 q43 q44 q45 q46 q47 q48 q49 q50 q51 q52 q53 q54 q55 q56 q57 q58 q59 q60 q61 q62 q63 q64 q65 q66 q67 q68 q69 q70 q71 q72 q73 q74 q75 q76 q77 q78 q79 q80 q81 q82 q83 q84 q85 q86 q87 q88 q89 q90 q91 q92 q93 q94 q95 q96 q97 q98 q99 q100 q101 q102 q103 q104 q105 q106 q107 q108 q109 q110 q111 q112 q113 q114 q115 q116 q117 q118 q119 q120 q121 q122 q123 q124 q125 q126 q127 q128 q129 q130 q131 q132 q133 q134 q135 q136 q137 q138 q139 q140 q141 q142 q143 q144 q145 q146 q147 q148 q149 q150 q151 q152 q153
.i q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19 q20 q21 q22 q23 q24 q25 q26 q27 q28 q29 q30 q31 q32 q33 q34 q35 q36 q37 q38 q39 q40 q41 q42 q43 q44 q45 q46 q47 q48 q49 q50 q51 q52 q53 q54 q55 q56 q57 q58 q59 q60 q61 q62 q63 q64 q65 q66 q67 q68 q69 q70 q71 q72 q73 q74 q75 q76 q77 q78 q79 q80 q81 q82 q83 q84 q85 q86 q87 q88 q89 q90 q91 q92 q93 q94 q95 q96 q97 q98 q99 q100 q101 q102 q103 q104 q105 q106 q107 q108 q109 q110 q111 q112 q113 q114 q115 q116 q117 q118 q119 q120 q121 q122 q123 q124 q125 q126 q127 q128 q129 q130 q131 q132 q133 q134 q135 q136 q137 q138 q139 q140 q141 q142 q143 q144 q145 q146 q147 q148 q149 q150 q151 q152 q153
.o q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 q16 q17 q18 q19 q20 q21 q22 q23 q24 q25 q26 q27 q28 q29 q30 q31 q32 q33 q34 q35 q36 q37 q38 q39 q40 q41 q42 q43 q44 q45 q46 q47 q48 q49 q50 q51 q52 q53 q54 q55 q56 q57 q58 q59 q60 q61 q62 q63 q64 q65 q66 q67 q68 q69 q70 q71 q72 q73 q74 q75 q76 q77 q78 q79 q80 q81 q82 q83 q84 q85 q86 q87 q88 q89 q90 q91 q92 q93 q94 q95 q96 q97 q98 q99 q100 q101 q102 q103 q104 q105 q106 q107 q108 q109 q110 q111 q112 q113 q114 q115 q116 q117 q118 q119 q120 q121 q122 q123 q124 q125 q126 q127 q128 q129 q130 q131 q132 q133 q134 q135 q136 q137 q138 q139 q140 q141 q142 q143 q144 q145 q146 q147 q148 q149 q150 q151 q152 q153
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
tof q19 q18 q20
tof q21 q20 q22
tof q23 q22 q24
tof q25 q24 q26
tof q27 q26 q28
tof q29 q28 q30
tof q31 q30 q32
tof q33 q32 q34
tof q35 q34 q36
tof q37 q36 q38
tof q39 q38 q40
tof q41 q40 q42
tof q43 q42 q44
tof q45 q44 q46
tof q47 q46 q48
tof q49 q48 q50
tof q51 q50 q52
tof q53 q52 q54
tof q55 q54 q56
tof q57 q56 q58
tof q59 q58 q60
tof q61 q60 q62
tof q63 q62 q64
tof q65 q64 q66
tof q67 q66 q68
tof q69 q68 q70
tof q71 q70 q72
tof q73 q72 q74
tof q75 q74 q76
tof q77 q76 q78
tof q79 q78 q80
tof q81 q80 q82
tof q83 q82 q84
tof q85 q84 q86
tof q87 q86 q88
tof q89 q88 q90
tof q91 q90 q92
tof q93 q92 q94
tof q95 q94 q96
tof q97 q96 q98
tof q99 q98 q100
tof q101 q100 q102
tof q103 q102 q104
tof q105 q104 q106
tof q107 q106 q108
tof q109 q108 q110
tof q111 q110 q112
tof q113 q112 q114
tof q115 q114 q116
tof q117 q116 q118
tof q119 q118 q120
tof q121 q120 q122
tof q123 q122 q124
tof q125 q124 q126
tof q127 q126 q128
tof q129 q128 q130
tof q131 q130 q132
tof q133 q132 q134
tof q135 q134 q136
tof q137 q136 q138
tof q139 q138 q140
tof q141 q140 q142
tof q143 q142 q144
tof q145 q144 q146
tof q147 q146 q148
tof q149 q148 q150
tof q151 q150 q152
tof q152 q153
tof q151 q150 q152
tof q149 q148 q150
tof q147 q146 q148
tof q145 q144 q146
tof q143 q142 q144
tof q141 q140 q142
tof q139 q138 q140
tof q137 q136 q138
tof q135 q134 q136
tof q133 q132 q134
tof q131 q130 q132
tof q129 q128 q130
tof q127 q126 q128
tof q125 q124 q126
tof q123 q122 q124
tof q121 q120 q122
tof q119 q118 q120
tof q117 q116 q118
tof q115 q114 q116
tof q113 q112 q114
tof q111 q110 q112
tof q109 q108 q110
tof q107 q106 q108
tof q105 q104 q106
tof q103 q102 q104
tof q101 q100 q102
tof q99 q98 q100
tof q97 q96 q98
tof q95 q94 q96
tof q93 q92 q94
tof q91 q90 q92
tof q89 q88 q90
tof q87 q86 q88
tof q85 q84 q86
tof q83 q82 q84
tof q81 q80 q82
tof q79 q78 q80
tof q77 q76 q78
tof q75 q74 q76
tof q73 q72 q74
tof q71 q70 q72
tof q69 q68 q70
tof q67 q66 q68
tof q65 q64 q66
tof q63 q62 q64
tof q61 q60 q62
tof q59 q58 q60
tof q57 q56 q58
tof q55 q54 q56
tof q53 q52 q54
tof q51 q50 q52
tof q49 q48 q50
tof q47 q46 q48
tof q45 q44 q46
tof q43 q42 q44
tof q41 q40 q42
tof q39 q38 q40
tof q37 q36 q38
tof q35 q34 q36
tof q33 q32 q34
tof q31 q30 q32
tof q29 q28 q30
tof q27 q26 q28
tof q25 q24 q26
tof q23 q22 q24
tof q21 q20 q22
tof q19 q18 q20
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
