shared B0, B1, B2, B3, B4, B5;
local t0, t1, t2, t3, t4, t5, ifvar6, ifvar17;

init
10: store B0 = *;
11: store B1 = *;
12: store B2 = *;
13: store B3 = *;
14: store B4 = *;
15: store B5 = *;

16: load t0 = B0;
17: load t1 = B1;
18: load t2 = B2;
19: load t3 = B3;
20: load t4 = B4;
21: load t5 = B5;

22: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

23: nop; 

/* Statement: i = 1 */
24:  load t0 = B0;
25:  load t1 = B1;
26:  load t2 = B2;
27:  load t3 = B3;
28:  load t4 = B4;
29:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
30:   store B4 = choose(false, false);


 /* update predicate - B5: (t1 = (i + j)) */
31:   store B5 = choose(false, false);
32: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

33: nop; 

/* Statement: j = 1 */
34:  load t0 = B0;
35:  load t1 = B1;
36:  load t2 = B2;
37:  load t3 = B3;
38:  load t4 = B4;
39:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
40:   store B4 = choose(false, false);


 /* update predicate - B5: (t1 = (i + j)) */
41:   store B5 = choose(false, false);
42: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

/* Reset local variables */ 
43: t0 = 0; 
44: t1 = 0; 
45: t2 = 0; 
46: t3 = 0; 
47: t4 = 0; 
48: t5 = 0; 
49: ifvar6 = 0; 
50: ifvar17 = 0; 


process 1
1: nop; 

/* Statement: k1 = 0 */
51: begin_atomic;
52:  load t0 = B0;
53:  load t1 = B1;
54:  load t2 = B2;
55:  load t3 = B3;
56:  load t4 = B4;
57:  load t5 = B5;

 /* update predicate - B0: (k1 = 0) */
58:   store B0 = choose((t0 != 0) || (t0 == 0) || (t1 != 0) || (t1 == 0) || (t2 != 0) || (t2 == 0) || (t3 != 0) || (t3 == 0) || (t4 != 0) || (t4 == 0) || (t5 != 0) || (t5 == 0), false);


 /* update predicate - B1: (k1 = 1) */
59:   store B1 = choose(false, (t0 != 0) || (t0 == 0) || (t1 != 0) || (t1 == 0) || (t2 != 0) || (t2 == 0) || (t3 != 0) || (t3 == 0) || (t4 != 0) || (t4 == 0) || (t5 != 0) || (t5 == 0));


60: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

61: end_atomic;
2: nop; 

/* Statement: if (k1 < 2) */ 
65: begin_atomic;
66: ifvar6 = 0;
67: load t0 = B0;
68: load t1 = B1;
69: load t2 = B2;
70: load t3 = B3;
71: load t4 = B4;
72: load t5 = B5;
73: end_atomic;

74: if (*) goto 62;
75: if (true) goto 63;
62: assume(!(false)); 
76: ifvar6 = 1;
3: nop; 

/* Statement: t1 = j */
77: begin_atomic;
78:  load t0 = B0;
79:  load t1 = B1;
80:  load t2 = B2;
81:  load t3 = B3;
82:  load t4 = B4;
83:  load t5 = B5;

 /* update predicate - B5: (t1 = (i + j)) */
84:   store B5 = choose(false, false);
85: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

86: end_atomic;
4: nop; 

/* Statement: t2 = i */
87: begin_atomic;
88:  load t0 = B0;
89:  load t1 = B1;
90:  load t2 = B2;
91:  load t3 = B3;
92:  load t4 = B4;
93:  load t5 = B5;

94: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

95: end_atomic;
5: nop; 

/* Statement: t1 = t1 + t2 */
96: begin_atomic;
97:  load t0 = B0;
98:  load t1 = B1;
99:  load t2 = B2;
100:  load t3 = B3;
101:  load t4 = B4;
102:  load t5 = B5;

 /* update predicate - B5: (t1 = (i + j)) */
103:   store B5 = choose(false, false);
104: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

105: end_atomic;
6: nop; 

/* Statement: i = t1 */
106: begin_atomic;
107:  load t0 = B0;
108:  load t1 = B1;
109:  load t2 = B2;
110:  load t3 = B3;
111:  load t4 = B4;
112:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
113:   store B4 = choose(false, false);


 /* update predicate - B5: (t1 = (i + j)) */
114:   store B5 = choose(false, false);
115: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

116: end_atomic;
7: nop; 

/* Statement: k1 = k1 + 1 */
117: begin_atomic;
118:  load t0 = B0;
119:  load t1 = B1;
120:  load t2 = B2;
121:  load t3 = B3;
122:  load t4 = B4;
123:  load t5 = B5;

 /* update predicate - B0: (k1 = 0) */
124:   store B0 = choose(false, (t0 != 0) || (t1 != 0));


 /* update predicate - B1: (k1 = 1) */
125:   store B1 = choose((t0 != 0), (t0 == 0) || (t1 != 0));


126: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

127: end_atomic;
/* Statement: 8: goto goto 2 */ 
8:   if (true) goto 2; 

63:  if (ifvar6 == 0) goto 128; 
129: if (true) goto 64;
128: assume(!((t0 != 0) || (t1 != 0))); 
64: nop;


process 2
1: nop; 

/* Statement: k2 = 0 */
130: begin_atomic;
131:  load t0 = B0;
132:  load t1 = B1;
133:  load t2 = B2;
134:  load t3 = B3;
135:  load t4 = B4;
136:  load t5 = B5;

 /* update predicate - B2: (k2 = 0) */
137:   store B2 = choose((t0 != 0) || (t0 == 0) || (t1 != 0) || (t1 == 0) || (t2 != 0) || (t2 == 0) || (t3 != 0) || (t3 == 0) || (t4 != 0) || (t4 == 0) || (t5 != 0) || (t5 == 0), false);


 /* update predicate - B3: (k2 = 1) */
138:   store B3 = choose(false, (t0 != 0) || (t0 == 0) || (t1 != 0) || (t1 == 0) || (t2 != 0) || (t2 == 0) || (t3 != 0) || (t3 == 0) || (t4 != 0) || (t4 == 0) || (t5 != 0) || (t5 == 0));


139: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

140: end_atomic;
2: nop; 

/* Statement: if (k2 < 2) */ 
144: begin_atomic;
145: ifvar17 = 0;
146: load t0 = B0;
147: load t1 = B1;
148: load t2 = B2;
149: load t3 = B3;
150: load t4 = B4;
151: load t5 = B5;
152: end_atomic;

153: if (*) goto 141;
154: if (true) goto 142;
141: assume(!(false)); 
155: ifvar17 = 1;
3: nop; 

/* Statement: t3 = j */
156: begin_atomic;
157:  load t0 = B0;
158:  load t1 = B1;
159:  load t2 = B2;
160:  load t3 = B3;
161:  load t4 = B4;
162:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
163:   store B4 = choose(false, false);


164: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

165: end_atomic;
4: nop; 

/* Statement: t4 = i */
166: begin_atomic;
167:  load t0 = B0;
168:  load t1 = B1;
169:  load t2 = B2;
170:  load t3 = B3;
171:  load t4 = B4;
172:  load t5 = B5;

173: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

174: end_atomic;
5: nop; 

/* Statement: t3 = t3 + t4 */
175: begin_atomic;
176:  load t0 = B0;
177:  load t1 = B1;
178:  load t2 = B2;
179:  load t3 = B3;
180:  load t4 = B4;
181:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
182:   store B4 = choose(false, false);


183: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

184: end_atomic;
6: nop; 

/* Statement: j = t3 */
185: begin_atomic;
186:  load t0 = B0;
187:  load t1 = B1;
188:  load t2 = B2;
189:  load t3 = B3;
190:  load t4 = B4;
191:  load t5 = B5;

 /* update predicate - B4: (t3 = (i + j)) */
192:   store B4 = choose(false, false);


 /* update predicate - B5: (t1 = (i + j)) */
193:   store B5 = choose(false, false);
194: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

195: end_atomic;
7: nop; 

/* Statement: k2 = k2 + 1 */
196: begin_atomic;
197:  load t0 = B0;
198:  load t1 = B1;
199:  load t2 = B2;
200:  load t3 = B3;
201:  load t4 = B4;
202:  load t5 = B5;

 /* update predicate - B2: (k2 = 0) */
203:   store B2 = choose(false, (t2 != 0) || (t3 != 0));


 /* update predicate - B3: (k2 = 1) */
204:   store B3 = choose((t2 != 0), (t2 == 0) || (t3 != 0));


205: assume(!((t0 != 0 && t1 != 0) || (t2 != 0 && t3 != 0))); 

206: end_atomic;
/* Statement: 8: goto goto 2 */ 
8:   if (true) goto 2; 

142:  if (ifvar17 == 0) goto 207; 
208: if (true) goto 143;
207: assume(!((t2 != 0) || (t3 != 0))); 
143: nop;

assert always true;

