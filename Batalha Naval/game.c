/* game.c */
#include "bttlship.h"

void
recount(void) {
    int b, f, x, y, px;

    table->player[0].bsites = table->player[1].bsites = 0;
    for ( y=0; y<N_Y; ++y ) {
        for ( x=0; x<N_X; ++x ) {
            f = table->flg[x][y];
            if ( !(f & (FLG_P1|FLG_P2)) )
                continue;   /* Sea zone */
            px = f & FLG_P1 ? 0 : 1;
            if ( !(f & FLG_BOMBD) )
                ++table->player[px].bsites;
            b = table->sea[x][y];
        }
    }
    if ( !table->player[0].bsites
    ||   !table->player[1].bsites )
        flg_game_over = 1;
}

void
bomb(int x,int y) {
    if ( table->flg[x][y] & (FLG_P1|FLG_P2) )
        table->flg[x][y] |= FLG_BOMBD;
    else
        table->flg[x][y] |= FLG_SPLSH;
}

int
getInput(int *px,int *py) {
    char buf[256];
    char *cp;
    char cx, cy, mx, my;

    mx = 'A' + N_X - 1;
    my = 'A' + N_Y - 1;
    fputs(": ",stdout);
    fgets(buf,sizeof buf,stdin);
    for ( cp=buf; *cp; ++cp )
        if ( islower(*cp) )
            *cp = toupper(*cp);

    cx = buf[1];
    cy = buf[0];
    if ( cx < 'A' || cx > mx
    ||   cy < 'A' || cy > my )
        return INP_NONE; /* Bad -- show battle */

    *px = cx - 'A';
    *py = cy - 'A';
    return INP_YX;      /* Bombs away! */
}

int
draw_hz(int sx,int sy,int z,int who) {
    int x, ex;
    char ch = who ? '=' : '-';
    char f = who ? FLG_P1 : FLG_P2;

    if ( sx + z > N_X ) {
        if ( (ex = sx - z) < 0 )
            return 0;       /* Can't */
        for ( x=sx; x>ex; --x )
            if ( table->sea[x][sy] != '.'
            || ( sy+1 < N_Y && table->sea[x][sy+1] != '.' )
            || ( sy-1 >= 0  && table->sea[x][sy-1] != '.' ) )
                return 0;   /* Can't */
        for ( x=sx; x>ex+1; --x ) {
            table->sea[x][sy] = ch;
            table->flg[x][sy] = f;
	}    
        table->sea[ex+1][sy] = '<';
        table->flg[ex+1][sy] = f;
    } else  {
        ex = sx + z;
        for ( x=sx; x<ex; ++x )
            if ( table->sea[x][sy] != '.'
            || ( sy+1 < N_Y && table->sea[x][sy+1] != '.' )
            || ( sy-1 >= 0  && table->sea[x][sy-1] != '.' ) )
                return 0;   /* Can't */
        for ( x=sx; x<ex-1; ++x ) {
            table->sea[x][sy] = ch;
            table->flg[x][sy] = f;
        }
        table->sea[ex-1][sy] = '>';
        table->flg[ex-1][sy] = f;
    }
    return 1;   /* Drawn */
}

int
draw_vt(int sx,int sy,int z,int who) {
    int y, ey;
    char ch = who ? '!' : '|';
    char f = who ? FLG_P1 : FLG_P2;

    if ( sy + z > N_Y ) {
        if ( (ey = sy - z) < 0 )
            return 0;       /* Can't */
        for ( y=sy; y>ey; --y )
            if ( table->sea[sx][y] != '.'
            || ( sx+1 < N_X && table->sea[sx+1][y] != '.' )
            || ( sx-1 >= 0  && table->sea[sx-1][y] != '.' ) )
                return 0;   /* Can't */
        for ( y=sy; y>ey+1; --y ) {
            table->sea[sx][y] = ch;
            table->flg[sx][y] = f;
        }
        table->sea[sx][ey+1] = '^';
        table->flg[sx][ey+1] = f;
    } else {
        ey = sy + z;
        for ( y=sy; y<ey; ++y )
            if ( table->sea[sx][y] != '.'
            || ( sx+1 < N_X && table->sea[sx+1][y] != '.' )
            || ( sx-1 >= 0  && table->sea[sx-1][y] != '.' ) )
                return 0;   /* Can't */
        for ( y=sy; y<ey-1; ++y ) {
            table->sea[sx][y] = ch;
            table->flg[sx][y] = f;
        }
        table->sea[sx][ey-1] = 'V';
        table->flg[sx][ey-1] = f;
    }        
    return 1;   /* Drawn */
}

void
genBattle(void) {
    int x, y, z, dir;
    int count = 0;
    int drawn = 0;
    time_t t;
    
    memset(table->sea,'.',sizeof table->sea);
    memset(table->flg,0,sizeof table->flg);

    do  {
        if ( count > 10000 ) {
            time(&t);
            srand(t);       /* Re-seed the generator */
            count = 0;
        } else
            ++count;        /* Increment the safety cnt */

        z = rand() % N_Z + 2; /* Length of ship */
        dir = rand() & 1;
        x = rand() % N_X;
        y = rand() % N_Y;
        if ( dir ) {
            if ( draw_hz(x,y,z,drawn&1)
            ||   draw_vt(x,y,z,drawn&1) )
                ++drawn;
        } else {
            if ( draw_vt(x,y,z,drawn&1)
            ||   draw_hz(x,y,z,drawn&1) )
                ++drawn;
        }
    } while ( drawn < N_SHIPS );
}

void
showRow(void) {
    int x;

    fputs("  ",stdout);
    for ( x=0; x<N_X; ++x ) {
        fputc(' ',stdout);
        fputc('A'+x,stdout);
    }
    fputc('\n',stdout);
}

void
showBattle(void) {
    int x, y;
    char f = !us
        ? (FLG_SEEN1|FLG_P1)
        : (FLG_SEEN0|FLG_P2);

    showRow();

    for ( y=0; y<N_Y; ++y ) {
        fputc('A'+y,stdout);
        fputc('|',stdout);
        for ( x=0; x<N_X; ++x ) {
            fputc(' ',stdout);
            if ( table->flg[x][y] & FLG_BOMBD ) {
                if ( table->flg[x][y] & FLG_P1 )
                    fputc('@',stdout);
                else
                    fputc('#',stdout);
            } else if ( table->flg[x][y] & f )
                fputc(table->sea[x][y],stdout);
            else if ( table->flg[x][y] & FLG_SPLSH )
                fputc('*',stdout);
            else
                fputc('.',stdout);
        }
        puts(" |");
    }

    showRow();
    recount();
    printf("ENEMY HAS %03d BOMB SITES LEFT\n",
        table->player[them].bsites);
    printf("YOU HAVE  %03d BOMB SITES LEFT\n",
        table->player[us].bsites);
}

/* End game.c */
