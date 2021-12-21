#include "AVRMAX.hpp"
#include <stdio.h> 
#include <stdlib.h> 

/***************************************************************************/
/*                               AVR-Max,                                  */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/* AVR ATMega88V port, iterative Negamax, by Andre Adrian                  */
/***************************************************************************/
/* 14dez2008 adr: merge avr_c_io and uMax                                  */
/* 18dez2008 adr: return -l if Negamax stack underrun                      */
/* 24dez2008 adr: add functions 1=new game, 2=set level, 3=show PV         */
/* 26dez2008 adr: bug fix, undo modifications on uMax 4.8                  */
/* 29dez2009 adr: Key Debouncing with inhibit (Sperrzeit nach Loslassen)   */

const signed char w[] = {
  0,2,2,7,-1,8,12,23};                         /* relative piece values    */

const signed char o[] = {
  -16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0,    /* step-vector lists */
  7,-1,11,6,8,3,6,                             /* 1st dir. in o[] per piece*/
  6,3,5,7,4,5,3,6};                            /* initial piece setup      */

/* better readability of AVR Program memory arrays */
#define o(ndx)  o[ndx]
#define w(ndx)  w[ndx]


AVRMAX::AVRMAX(){
    reset() ;
}


void AVRMAX::reset(){
  k=16;O=Q=R=0;
  for(W=0;W<sizeof b;++W)b[W]=0;
  W=8;
  while(W--){ 
   b[W]=(b[W+112]=o(W+24)+8)+8;b[W+16]=18;b[W+96]=9; /* initial board setup*/
   L=8;while(L--)
    b[16*L+W+8]=(W-4)*(W-4)+(L+L-7)*(L+L-7)/4;       /* center-pts table   */
  }                                                  /*(in unused half b[])*/
}


void AVRMAX::print_row(int r, char row[17]){
  for (int c = 0 ; c < 8 ; c++){
    int i = (c * 16) + r ;
    row[c*2] = ".?inkbrq?I?NKBRQ"[b[i]&15] ;
    row[c*2+1] = ' ' ;
  }
  row[16] = '\0' ;
}


/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/***************************************************************************/
/* version 4.8 (1953 characters) features:                                 */
/* - recursive negamax search                                              */
/* - all-capture MVV/LVA quiescence search                                 */
/* - (internal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - a hash table storing score and best move                              */
/* - futility pruning                                                      */
/* - king safety through magnetic, frozen king in middle-game              */
/* - R=2 null-move pruning                                                 */
/* - keep hash and repetition-draw detection                               */
/* - better defense against passers through gradual promotion              */
/* - extend check evasions in inner nodes                                  */
/* - reduction of all non-Pawn, non-capture moves except hash move (LMR)   */
/* - full FIDE rules (expt under-promotion) and move-legality checking     */

/* better readability of working struct variables */
#define q _.q
#define l _.l
#define e _.e
#define E _.E
#define z _.z
#define n _.n
#define m _.m
#define v _.v
#define V _.V
#define P _.P
#define r _.r
#define j _.j
#define B _.B
#define d _.d
#define h _.h
#define C _.C
#define u _.u
#define p _.p
#define x _.x
#define y _.y
#define F _.F
#define G _.G
#define H _.H
#define t _.t
#define X _.X
#define Y _.Y
#define a _.a

#define M 0x88                              /* Unused bits in valid square */  
#define S 128                               /* Sign bit of char            */
#define I 8000                              /* Infinity score              */


void AVRMAX::D(){                                       /* iterative Negamax search */               
    D:if (--J<A) {               /* stack pointer decrement and underrun check */
    ++J;DD=-l;goto R;                                    /* simulated return */
    } 
    q=Dq;l=Dl;e=De;E=DE;z=Dz;n=Dn;                          /* load arguments */
    a=Da;                                         /* load return address state*/

    --q;                                          /* adj. window: delay bonus */
    k^=24;                                        /* change sides             */
    d=X=Y=0;                                      /* start iter. from scratch */
    while(d++<n||d<3||                            /* iterative deepening loop */
    z&K==I&&(max>0&d<98||                  /* root: deepen upto time   */
    (K=X,L=Y&~M,d=3)))                          /* time's up: go do best    */
    {x=B=X;                                       /* start scan at prev. best */
    h=Y&S;                                       /* request try noncastl. 1st*/
    if(d<3)P=I;else 
    {*J=_;Dq=-l;Dl=1-l;De=-e;DE=S;Dz=0;Dn=d-3;   /* save locals, arguments   */
    Da=0;goto D;                                /* Search null move         */
    R0:_=*J;P=DD;                                  /* load locals, return value*/
    }
    m=-P<l|R>35?d>2?-I:e:-P;                     /* Prune or stand-pat       */
    max--;                                       /* node count (for timing)  */
    do{u=b[x];                                   /* scan board looking for   */
    if(u&k)                                     /*  own piece (inefficient!)*/
    {r=p=u&7;                                   /* p = piece type (set r>0) */
        j=o(p+16);                                 /* first step vector f.piece*/
        while(r=p>2&r<0?-r:-o(++j))                /* loop over directions o[] */
        {A:                                        /* resume normal after best */
        y=x;F=G=S;                                /* (x,y)=move, (F,G)=castl.R*/
        do{                                       /* y traverses ray, or:     */
        H=y=h?Y^h:y+r;                           /* sneak in prev. best move */
        if(y&M)break;                            /* board edge hit           */
        m=E-S&b[E]&&y-E<2&E-y<2?I:m;             /* bad castling             */
        if(p<3&y==E)H^=16;                       /* shift capt.sqr. H if e.p.*/
        t=b[H];if(t&k|p<3&!(y-x&7)-!t)break;     /* capt. own, bad pawn mode */
        i=37*w(t&7)+(t&192);                     /* value of capt. piece t   */
        m=i<0?I:m;                               /* K capture                */
        if(m>=l&d>1)goto J;                      /* abort on fail high       */

        v=d-1?e:i-p;                             /* MVV/LVA scoring          */
        if(d-!t>1)                               /* remaining depth          */
        {v=p<6?b[x+8]-b[y+8]:0;                  /* center positional pts.   */
        b[G]=b[H]=b[x]=0;b[y]=u|32;             /* do move, set non-virgin  */
        if(!(G&M))b[F]=k+6,v+=50;               /* castling: put R & score  */
        v-=p-4|R>29?0:20;                       /* penalize mid-game K move */
        if(p<3)                                 /* pawns:                   */
        {v-=9*((x-2&M||b[x-2]-u)+               /* structure, undefended    */
                (x+2&M||b[x+2]-u)-1              /*        squares plus bias */
                +(b[x^16]==k+36))                 /* kling to non-virgin King */
                -(R>>2);                          /* end-game Pawn-push bonus */
            V=y+r+1&S?647-p:2*(u&y+16&32);         /* promotion or 6/7th bonus */
            b[y]+=V;i+=V;                          /* change piece, add score  */
        }
        v+=e+i;V=m>q?m:q;                       /* new eval and alpha       */
        C=d-1-(d>5&p>2&!t&!h);
        C=R>29|d<3|P-I?C:d;                     /* extend 1 ply if in check */
        do
            if(C>2|v>V)
            {*J=_;Dq=-l;Dl=-V;De=-v;DE=F;Dz=0;Dn=C; /* save locals, arguments  */
            Da=1;goto D;                          /* iterative eval. of reply */
    R1:      _=*J;s=-DD;                           /* load locals, return value*/
            }else s=v;                             /* or fail low if futile    */
        while(s>q&++C<d);v=s;
        if(z&&K-I&&v+I&&x==K&y==L)              /* move pending & in root:  */
        {Q=-e-i;O=F;                            /*   exit if legal & found  */
            R+=i>>7;++J;DD=l;goto R;               /* captured non-P material  */
        }
        b[G]=k+6;b[F]=b[y]=0;b[x]=u;b[H]=t;     /* undo move,G can be dummy */
        }
        if((v>m)||((v==m)&&(rand()&1)))          /* new best, update max,best*/
        m=v,X=x,Y=y|S&F;                        /* mark double move with S  */
        if(h){h=0;goto A;}                       /* redo after doing old best*/
        if(x+r-y|u&32|                           /* not 1st step,moved before*/
            p>2&(p-4|j-7||                        /* no P & no lateral K move,*/
            b[G=x+3^r>>1&7]-k-6                   /* no virgin R in corner G, */
            ||b[G^1]|b[G^2])                      /* no 2 empty sq. next to R */
            )t+=p<5;                               /* fake capt. for nonsliding*/
        else F=y;                                /* enable e.p.              */
        }while(!t);                               /* if not capt. continue ray*/
    }}}while((x=x+9&~M)-B);                      /* next sqr. of board, wrap */
    J:if(m>I-M|m<M-I)d=98;                         /* mate holds to any depth  */
    m=m+I|P==I?m:0;                              /* best loses K: (stale)mate*/
    if(z&d>2){
        LX = X ; LY = Y ;
    }
    }                                             /*    encoded in X S,8 bits */
    k^=24;                                        /* change sides back        */
    ++J;DD=m+=m<e;                                /* delayed-loss bonus       */
    R:if (J!=A+AVRMAX_U) switch(a){case 0:goto R0;case 1:goto R1;}
    else return;
}


void AVRMAX::do_move(char *c, long max){
  this->max = max ;                                 /* set time control   */
  LX = LY = 0 ; 
  K = I ;                                            /* invalid move       */
  if (c[0] != '\n'){
    K = c[0]-16*c[1]+799 ; 
    L = c[2]-16*c[3]+799 ;                           /* parse entered move */
  }
  Dq=-I;Dl=I;De=Q;DE=O;Dz=1;Dn=4;               /* store arguments of D() */
  Da=0;                                         /* state */
  D();           
  if (DD == 0){
    c[0] = '=' ;
    c[1] = '\0' ;
  }
  else if (DD == (-I + 1)){
    c[0] = 'X' ;
    c[1] = '\0' ;
  }
  else {
    c[0] = 'a' + (LX & 7) ; 
    c[1] = '8' - (LX >> 4) ; 
    c[2] = 'a' + (LY & 7) ; 
    c[3] = '8' - (LY >> 4 & 7) ; 
    c[4] = '\0' ;
  }
}
