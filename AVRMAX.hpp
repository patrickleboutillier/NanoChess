#ifndef AVRMAX_HPP
#define AVRMAX_HPP


#define AVRMAX_U  24          /* D() Stack array size        */


class AVRMAX {
  private:
    long max;
    struct {
      short q,l,e;         
      short m,v,V,P;
      unsigned char E,z,n;    /* Args: E=e.p. sqr.; z=level 1 flag; n=depth        */          
      signed char r;          /* step vector current ray                           */
      unsigned char j,        /* j=loop over directions (rays) counter             */ 
      B,d,                    /* B=board scan start, d=iterative deepening counter */ 
      h,C,                    /* h=new ply depth?, C=ply depth?                    */
      u,p,                    /* u=moving piece, p=moving piece type               */
      x,y,                    /* x=origin square, y=target square of current move  */
      F,                      /* F=e.p., castling skipped square                   */
      G,                      /* G=castling R origin (corner) square               */
      H,t,                    /* H=capture square, t=piece on capture square       */
      X,Y,                    /* X=origin, Y=target square of best move so far     */
      a;                      /* D() return address state                          */
    } _, A[AVRMAX_U],*J=A+AVRMAX_U;         /* _=working set, A=stack array, J=stack pointer     */

    short Q,                  /* pass updated eval. score    */
    K,                        /* input move, origin square   */
    i,s,                      /* temp. evaluation term       */
    Dq,Dl,De,                 /* D() arguments */
    DD;                       /* D() return value */

    unsigned char L,          /* input move, target square*/
    b[129],                   /* board: half of 16x8+dummy*/     
    k,                        /* moving side 8=white 16=black*/
    O,                        /* pass e.p. flag at game level*/
    R,                        /* captured non pawn material  */
    DE,Dz,Dn,                 /* D() arguments            */
    Da,                       /* D() state                */
    W,                        /* @ temporary                */
    hv,                       /* zeige Hauptvariante      */
    LX,LY;

    void D() ;


  public:
    AVRMAX() ;
    void reset() ;
    void print_row(int r, char row[17]) ;
    void do_move(char *c, long max) ;

} ;

#endif
