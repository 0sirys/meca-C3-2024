module parqueo(output one,output two,output three, output allfree, input p1, input p2, input p3, input p4);
// declaracion de entradas negadas
    wire NA,NB,NC,ND;
// declaracion de salidas de compuertas And
    wire S1_1,S1_2,S1_3,S1_4,S1_5,S1_6,S1_7,S1_8;
    wire S2_1,S2_2,S2_3,S2_4,S2_5,S2_6;
    wire S3_1,S3_2,S3_3,S3_4,S3_5,S3_6,S3_7,S3_8,S3_9,S3_A,S3_B,S3_C,S3_D,S3_E; 
// negacion de entradas
    not Na (NA,p4);
    not Nb(NB,p3);
    not Nc(NC,p2);
    not Nd (ND,p1);
// salida allfree adyacentes %S4%
    and and0(S0_1,NA,NB);
    and and1(S0_2,NC,ND);
    and and2(allfree,S0_1,S0_2);
// salida 2free %S1%
//mintermino
    and and3 (S1_1,NA,NB);
    and and4 (S1_2,S1_1,p3);
//mintermino
    and and5 (S1_3,p2,NC);
    and and6 (S1_4,S1_3,ND);
//mintermino
    and and7 (S1_5,p1,NB);
    and and8 (S1_6,NC,p4);
    and and9 (S1_7,S1_5,S1_6);
//
    or or1 (S1_8,S1_2,S1_4);
    or or2 (two,S1_8,S1_7);
//###########################

// salida 3free %S2%
//mintermino
    and andA (S2_1,NA,NB);
    and andB (S2_2,NC,p4);
    and andC (S2_3,S2_1,S2_2);
//mintermino
    and andD (S2_4,p1,NB);
    and andE (S2_5,NC,ND);
    and andF (S2_6,S2_4,S2_5);
//
    or or3 (three,S2_3,S2_6);
// salida onefree %S4%
//mintermino
    and and10 (S3_1,NA,p2);
    and and11 (S3_2,p3,p4);
    and and12 (S3_3,S3_1,S3_2);
//mintermino
    and and13 (S3_4,p1,NB);
    and and14 (S3_5,p3,p4);
    and and15 (S3_6,S3_4,S3_5);
//mintermino
    and and16 (S3_7,p1,p2);
    and and17 (S3_8,NC,p4);
    and and18 (S3_9,S3_7,S3_8);
//mintermino
    and and19 (S3_A,p1,p2);
    and and1A (S3_B,p3,ND);
    and and1B (S3_C,S3_A,S3_B);
//
    or or4 (S3_D,S3_3,S3_6);
    or or5 (S3_E,S3_9,S3_C);
    or or6 (one,S3_D,S3_E);

endmodule