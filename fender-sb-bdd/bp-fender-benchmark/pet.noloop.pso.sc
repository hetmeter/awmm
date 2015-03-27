beginit

store flag0 = 0;

store flag1 = 0;

store turn = 0;

overflow = 0;

endinit

process 1:

/* initialize counters */
begin_atomic
flag0_cnt_p1 = 0;
flag1_cnt_p1 = 0;
turn_cnt_p1 = 0;
end_atomic

1:begin_atomic
/* Statement: store flag0 = 1; */
if ((flag0_cnt_p1 != 0))
  overflow = 1;
else
endif;
flag0_cnt_p1 = flag0_cnt_p1 + 1;
if ((flag0_cnt_p1 != 0))
  flag0_1_p1 = 1;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p1 != 0)
    flag0 = flag0_1_p1;
  flag0_cnt_p1 = 0;
else
endif;
if (flag1_cnt_p1 != 0)
    flag1 = flag1_1_p1;
  flag1_cnt_p1 = 0;
else
endif;
if (turn_cnt_p1 != 0)
    turn = turn_1_p1;
  turn_cnt_p1 = 0;
else
endif;
end_atomic

2:begin_atomic
/* Statement: store turn = 1; */
if ((turn_cnt_p1 != 0))
  overflow = 1;
else
endif;
turn_cnt_p1 = turn_cnt_p1 + 1;
if ((turn_cnt_p1 != 0))
  turn_1_p1 = 1;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p1 != 0)
    flag0 = flag0_1_p1;
  flag0_cnt_p1 = 0;
else
endif;
if (flag1_cnt_p1 != 0)
    flag1 = flag1_1_p1;
  flag1_cnt_p1 = 0;
else
endif;
if (turn_cnt_p1 != 0)
    turn = turn_1_p1;
  turn_cnt_p1 = 0;
else
endif;
end_atomic

3:nop;
/* Statement: load f1 = flag1; */
begin_atomic
if (flag1_cnt_p1 = 0)
  load f1 = flag1;
else
endif;
if ((flag1_cnt_p1 != 0))
  f1 = flag1_1_p1;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p1 != 0)
    flag0 = flag0_1_p1;
  flag0_cnt_p1 = 0;
else
endif;
if (flag1_cnt_p1 != 0)
    flag1 = flag1_1_p1;
  flag1_cnt_p1 = 0;
else
endif;
if (turn_cnt_p1 != 0)
    turn = turn_1_p1;
  turn_cnt_p1 = 0;
else
endif;
end_atomic

4:nop;
/* Statement: load t1 = turn; */
begin_atomic
if (turn_cnt_p1 = 0)
  load t1 = turn;
else
endif;
if ((turn_cnt_p1 != 0))
  t1 = turn_1_p1;
else
endif;
end_atomic

5:if (((t1 != 0) & (f1 != 0)))

  goto 3; 

else

endif;

6:nop;

7:begin_atomic
/* Statement: store flag0 = 0; */
if ((flag0_cnt_p1 != 0))
  overflow = 1;
else
endif;
flag0_cnt_p1 = flag0_cnt_p1 + 1;
if ((flag0_cnt_p1 != 0))
  flag0_1_p1 = 0;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p1 != 0)
    flag0 = flag0_1_p1;
  flag0_cnt_p1 = 0;
else
endif;
if (flag1_cnt_p1 != 0)
    flag1 = flag1_1_p1;
  flag1_cnt_p1 = 0;
else
endif;
if (turn_cnt_p1 != 0)
    turn = turn_1_p1;
  turn_cnt_p1 = 0;
else
endif;
end_atomic

process 2:

/* initialize counters */
begin_atomic
flag0_cnt_p2 = 0;
flag1_cnt_p2 = 0;
turn_cnt_p2 = 0;
end_atomic

1:begin_atomic
/* Statement: store flag1 = 1; */
if ((flag1_cnt_p2 != 0))
  overflow = 1;
else
endif;
flag1_cnt_p2 = flag1_cnt_p2 + 1;
if ((flag1_cnt_p2 != 0))
  flag1_1_p2 = 1;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p2 != 0)
    flag0 = flag0_1_p2;
  flag0_cnt_p2 = 0;
else
endif;
if (flag1_cnt_p2 != 0)
    flag1 = flag1_1_p2;
  flag1_cnt_p2 = 0;
else
endif;
if (turn_cnt_p2 != 0)
    turn = turn_1_p2;
  turn_cnt_p2 = 0;
else
endif;
end_atomic

2:begin_atomic
/* Statement: store turn = 0; */
if ((turn_cnt_p2 != 0))
  overflow = 1;
else
endif;
turn_cnt_p2 = turn_cnt_p2 + 1;
if ((turn_cnt_p2 != 0))
  turn_1_p2 = 0;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p2 != 0)
    flag0 = flag0_1_p2;
  flag0_cnt_p2 = 0;
else
endif;
if (flag1_cnt_p2 != 0)
    flag1 = flag1_1_p2;
  flag1_cnt_p2 = 0;
else
endif;
if (turn_cnt_p2 != 0)
    turn = turn_1_p2;
  turn_cnt_p2 = 0;
else
endif;
end_atomic

3:nop;
/* Statement: load f2 = flag0; */
begin_atomic
if (flag0_cnt_p2 = 0)
  load f2 = flag0;
else
endif;
if ((flag0_cnt_p2 != 0))
  f2 = flag0_1_p2;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p2 != 0)
    flag0 = flag0_1_p2;
  flag0_cnt_p2 = 0;
else
endif;
if (flag1_cnt_p2 != 0)
    flag1 = flag1_1_p2;
  flag1_cnt_p2 = 0;
else
endif;
if (turn_cnt_p2 != 0)
    turn = turn_1_p2;
  turn_cnt_p2 = 0;
else
endif;
end_atomic

4:nop;
/* Statement: load t2 = turn; */
begin_atomic
if (turn_cnt_p2 = 0)
  load t2 = turn;
else
endif;
if ((turn_cnt_p2 != 0))
  t2 = turn_1_p2;
else
endif;
end_atomic

5:if (((f2 != 0) & (t2 = 0)))

  goto 3; 

else

endif;

6:nop;

7:begin_atomic
/* Statement: store flag1 = 0; */
if ((flag1_cnt_p2 != 0))
  overflow = 1;
else
endif;
flag1_cnt_p2 = flag1_cnt_p2 + 1;
if ((flag1_cnt_p2 != 0))
  flag1_1_p2 = 0;
else
endif;
end_atomic

begin_atomic
/* Statement: fence; */
if (flag0_cnt_p2 != 0)
    flag0 = flag0_1_p2;
  flag0_cnt_p2 = 0;
else
endif;
if (flag1_cnt_p2 != 0)
    flag1 = flag1_1_p2;
  flag1_cnt_p2 = 0;
else
endif;
if (turn_cnt_p2 != 0)
    turn = turn_1_p2;
  turn_cnt_p2 = 0;
else
endif;
end_atomic

/* assert always ((pc{1} != 6) || (pc{2} != 6));*/


