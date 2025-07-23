#include <grrlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <time.h>
#include <math.h> 
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
//images
#include <GFX_grassbg_png.h>
#include <GFX_images_png.h>
#include <GFX_font_png.h>
#include <GFX_gradient_png.h>
#include <GFX_gradient4x_png.h>
#include <GFX_images32_png.h>

//colors, yep, I'm using game maker names
#define c_black   0x000000FF
#define c_marron  0x800000FF
#define c_green   0x008000FF
#define c_olive   0x808000FF
#define c_navy    0x000080FF
#define c_purple  0x800080FF
#define c_teal    0x008080FF
#define c_gray    0x808080FF
#define c_silver  0xC0C0C0FF
#define c_red     0xFF0000FF
#define c_lime    0x00FF00FF
#define c_yellow  0xFFFF00FF
#define c_orange  0xFF7F00FF
#define c_blue    0x0000FFFF
#define c_fuchsia 0xFF00FFFF
#define c_aqua    0x00FFFFFF
#define c_white   0xFFFFFFFF

static u8 CalculateFrameRate(void);

//variables
int rmax;//for rng
int rmin;

int money = 500; //user can see this
int level = 1;
int xp = 0;
float xpneed = 10;
int score = 0;

int screen = 1;
int selectorpos = 0; 
int selx = 0;
int sely = 0;
int selmode = 0; //pointer/buttons mode
int previr1x = 0;
int previr1y = 0;

float bar; //float variable to use decimals for healthbars
float temp = 0; //the same but for other healthbars

//"objects"
int tower[10][6][8];

int enemypointer = 0;
int enemycount = 0;
int enemyspawncooldown = 1400;

int fireball[300][3]; //maxcount, values
int fireballpointer = 0;
int maxfireballs = 299;
int fireballrot = 0;
int fireballcount = 0; //just for debug

int min(int a, int b) {
    return (a < b) ? a : b;
}

void create_fireball(int _x,int _y,int _damage)
{
    if(fireballpointer != maxfireballs)
    {
        fireballpointer++;
    }
    else
    {
        fireballpointer = 0;
    }
        fireball[fireballpointer][0] = _x;
        fireball[fireballpointer][1] = _y;
        fireball[fireballpointer][2] = _damage;
    fireballcount++; //then I'll delete this line (maybe)
}
void destroy_fireball(int index)
{
    fireball[index][0] = 0;
    fireball[index][1] = 0;
    fireball[index][2] = 0;
    fireballcount--; //this too
}
void heal_tower(int _x,int _y,int _c)
{
    if(tower[_x][_y][0] + _c < tower[_x][_y][6]) //si no pasa el límite
    {
        tower[_x][_y][0] += _c; //curar
    }
    else
    {
        tower[_x][_y][0] = tower[_x][_y][6]; //vida = vidamax
    }
}


void tower_action(int _x,int _y) //contador llega a 0
{
    switch(tower[_x][_y][4])
    {
        case 0: //lanzador
        case 1: //lanzador 2
            create_fireball(_x << 6,_y,tower[_x][_y][7]);
        break;
        case 2: //aweonao de plata
            money += 25*tower[_x][_y][1];
        break;
        case 7: //maría, digo planta.
        heal_tower(_x,_y,15);
            switch(_x)
            {
                default:
                    heal_tower(_x+1,_y,15);
                    heal_tower(_x-1,_y,15);
                    break;
                case 0: //si está en la izquierda
                    heal_tower(_x+1,_y,15);
                break;

                case 10: //si está en la derecha
                    heal_tower(_x-1,_y,15);
                break;
            }
            switch (_y)
            {
                default:
                    heal_tower(_x,_y+1,15);
                    heal_tower(_x,_y-1,15);
                    break;
                case 0: //si está en la izquierda
                    heal_tower(_x,_y+1,15);
                break;

                case 6: //si está en la derecha
                    heal_tower(_x,_y-1,15);
                break;
            }

        break;
    }
}

//iniciar valores de las torres
static const int pvida[] =   {25,  250,  1, 150,  500, 5000,    1,  15};
static const int pprecio[] = {250, 900, 50, 500, 1500, 5000, 1000, 300};
static const int ptex[] =    {0,   2,   1,  8,   9,   10,    7,     11};
static const int pcooldown[]={120, 30,  600, -1,  -1,  -1,   -1,    120};

typedef struct
{
    int id, y, hp, color, damage, knockback, resistance, explode, maxhp;
    float x, speed;
}enemydata;

int maxenemies = 32; //CAMBIA AQUÍ EL MÁXIMO DE ENEMIGOS
enemydata enemies[32]; 

void create_enemy(int _tipo, int _y)
{
    if (enemycount != maxenemies)
    {
        while (enemies[enemypointer].id != -1)
        {
            enemypointer++;
            if (enemypointer >= maxenemies)
            {
                enemypointer = 0;
            }
        }
        enemies[enemypointer].id = enemypointer;
        enemies[enemypointer].y = _y; // RANDOM
        enemies[enemypointer].x = 640;
        enemies[enemypointer].damage = round(level * 1.2);
        enemies[enemypointer].hp = min(round(2 * (level * 1.8)),9);//velocidad máxima = 9
        enemies[enemypointer].knockback = -10;
        enemies[enemypointer].explode = 0; //unused variable
        enemies[enemypointer].speed = -1.3 * (level * 0.16);
        enemies[enemypointer].resistance = 0;

        switch (_tipo)
        {
            default:
            case 0:
                enemies[enemypointer].color = c_lime;
                break;
            case 1:
                enemies[enemypointer].color = c_red;
                enemies[enemypointer].damage <<= 2;
                break;
            case 2:
                enemies[enemypointer].color = c_blue;
                enemies[enemypointer].knockback = 0;
                enemies[enemypointer].hp >>= 2;
                enemies[enemypointer].speed *= 0.5;
                enemies[enemypointer].damage = 0;
                break;
            case 3:
                enemies[enemypointer].color = c_orange;
                enemies[enemypointer].resistance = 1;
                break;
            case 4:
                enemies[enemypointer].color = c_purple;
                enemies[enemypointer].speed *= 1.5;
                break;
            case 5:
                enemies[enemypointer].color = c_gray;
                enemies[enemypointer].hp <<= 2;
                enemies[enemypointer].speed *= 0.5;
                break;
            case 6:
                enemies[enemypointer].color = c_yellow;
                enemies[enemypointer].hp >>= 3;
                enemies[enemypointer].speed *= 3;
                break;
        }

        enemies[enemypointer].maxhp = enemies[enemypointer].hp;
        enemycount++;
    }
}

void destroy_enemy(int _id)
{
    enemies[_id].id = -1;
    enemies[_id].x = -1;
    enemies[_id].speed = -0;
    enemies[_id].hp = 0;
    enemies[_id].explode = 0;
    enemies[_id].color = c_white;
    enemies[_id].resistance = 0;
    enemies[_id].damage = 0;
    enemies[_id].y = -1;
    enemies[_id].knockback = 0;
    enemycount--;
}
void create_tower(int _x, int _y, int t)
{
    tower[_x][_y][0] = pvida[t];
    tower[_x][_y][1] = 1; //nivel?
    tower[_x][_y][2] = pprecio[t];
    tower[_x][_y][3] = pcooldown[t];//cooldown
    tower[_x][_y][4] = t; //tipo
    tower[_x][_y][5] = tower[_x][_y][3]; //cooldown máximo
    tower[_x][_y][6] = pvida[t]; //vida máxima
    tower[_x][_y][7] = 1; //Propiedad extra
}
void destroy_tower(int _x, int _y)
{
    tower[_x][_y][0] = 0;
    tower[_x][_y][1] = 0;
    tower[_x][_y][2] = 0;
    tower[_x][_y][3] = 0;
    tower[_x][_y][4] = 0;
    tower[_x][_y][5] = 0;
    tower[_x][_y][6] = 0;
    tower[_x][_y][7] = 0;
}

int main(int argc, char **argv) { //MAIN!!111111111!!11 CODE HEREEEEEEEE

    u8 FPS = 0;
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    ir_t ir1;
    //load font
    GRRLIB_texImg *font = GRRLIB_LoadTexture(GFX_font_png);
    GRRLIB_InitTileSet(font, 8, 16, 0);
    //images
    GRRLIB_texImg *gfx_grassbg = GRRLIB_LoadTexture(GFX_grassbg_png);
    GRRLIB_texImg *gfx_gradient = GRRLIB_LoadTexture(GFX_gradient_png);
    GRRLIB_texImg *gfx_gradient4x = GRRLIB_LoadTexture(GFX_gradient4x_png);
    //tiles
    GRRLIB_texImg *gfx_images = GRRLIB_LoadTexture(GFX_images_png);
    GRRLIB_InitTileSet(gfx_images, 64, 64, 0);
    GRRLIB_texImg *gfx_images32 = GRRLIB_LoadTexture(GFX_images32_png);
    GRRLIB_InitTileSet(gfx_images32, 32, 32, 0);

    srand(time(NULL));
    //some data
    for (int i = 0; i < maxenemies; i++) {
        enemies[i].id = -1;
    }

    // Loop forever
    while(1) {

        WPAD_ScanPads();  // Scan the Wiimotes
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;
        WPAD_SetVRes(0, 640, 480);
        WPAD_IR(WPAD_CHAN_0, &ir1);//ir1.x  ir1.y
        if(screen == 1)
        {
        int mouse_x = floor(ir1.x);
        int mouse_y = floor(ir1.y);
        mouse_x = mouse_x >> 6;
        mouse_y = mouse_y >> 6;

        if(abs(ir1.x - previr1x) > 3 || abs(ir1.y - previr1y) > 3)
        {
            selmode = 0;
        }
        previr1x = ir1.x;
        previr1y = ir1.y;
        
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS)
        {
            if(selectorpos < 8) selectorpos++;
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_MINUS)
        {
            if(selectorpos > 0) selectorpos--;
        }

        //selector
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN)
        {
            if(selx < 9) selx ++;
            selmode = 1;
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_UP)
        {
            if(selx > 0) selx --;
            selmode = 1;
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT)
        {
            if(sely < 5) sely ++;
            selmode = 1;
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT)
        {
            if(sely > 0) sely --;
            selmode = 1;
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_1)
        {
            if(sely < 6)
            {
            destroy_tower(selx,sely);
            }
        }
        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
        {
            if(mouse_y < 6)
            {
            destroy_tower(mouse_x,mouse_y);
            }
        }

        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A || WPAD_ButtonsDown(0) & WPAD_BUTTON_2)
        {
            if(selmode == 1)
            {
                mouse_x = selx;
                mouse_y = sely;
            }
            if(mouse_y < 6)
            {
                if(selectorpos != 8)
                {
                    if(tower[mouse_x][mouse_y][0] == 0 && money >= pprecio[selectorpos]) //Crea una torre
                    {
                        money -= pprecio[selectorpos];
                        create_tower(mouse_x,mouse_y,selectorpos);
                    }
                    else
                    { 
                        if (money >= tower[mouse_x][mouse_y][2] && tower[mouse_x][mouse_y][4] != 1 && tower[mouse_x][mouse_y][4] != 6)//subir de nivel una torre
                        {
                            money -= tower[mouse_x][mouse_y][2]; //quitar plata
                            tower[mouse_x][mouse_y][2] = round((tower[mouse_x][mouse_y][2]*1.5)/25)*25; //cambiar precio
                            tower[mouse_x][mouse_y][0] = tower[mouse_x][mouse_y][6] << 1; //actualizar vida
                            tower[mouse_x][mouse_y][6] = tower[mouse_x][mouse_y][0]; //actualizar vida máxima
                            tower[mouse_x][mouse_y][1]++; //subir el contador de niveles
                            switch(tower[mouse_x][mouse_y][4])//COOLDOWN!!1
                            {
                                case 0://lanzador 1
                                    if(tower[mouse_x][mouse_y][4] >= pcooldown[0]>>1)
                                    {
                                        tower[mouse_x][mouse_y][3]-=5;
                                    }
                                    else
                                    {
                                        tower[mouse_x][mouse_y][3] = pcooldown[0];
                                        tower[mouse_x][mouse_y][7]<<=1;
                                    }
                                break;
                                case 2://minero
                                    if(tower[mouse_x][mouse_y][4] >= pcooldown[2]>>2)
                                    {
                                        tower[mouse_x][mouse_y][4] -= 5;
                                    }
                                case 7://planta
                                    if(tower[mouse_x][mouse_y][3] >= 10)
                                    {
                                        tower[mouse_x][mouse_y][3]-=5;
                                    }
                                break;
                            }
                        }
                    }
                }
                else
                {
                    destroy_tower(mouse_x,mouse_y);
                }
            }
            else
            {
                if(mouse_x != 9)
                {
                    selectorpos = mouse_x;
                }
            }
        }

        if(enemyspawncooldown > 0)//enemy spawner
        {
            enemyspawncooldown--;
        }
        else
        {
            score += level;
             
             int pos = rand() % 6;
             rmin = 0, rmax = min(level,12);
             int rtipo = rand() % (rmax - rmin + 1) + rmin;
                switch(rtipo)
                {
                    default:
                    create_enemy(0,pos);
                    break;
                    case 2:
                    create_enemy(5,pos);
                    break;

                    case 3:
                    create_enemy(6,pos);
                    break;

                    case 5:
                    create_enemy(2,pos);
                    break;

                    case 7:
                    create_enemy(4,pos);
                    break;

                    case 10:
                    create_enemy(3,pos);
                    break;

                    case 12:
                    create_enemy(1,pos);
                    break;
                }
                if(600/level >=  5)
                {
                    enemyspawncooldown = 600/level;
                }
                else
                {
                    enemyspawncooldown = 5;
                }
            
            if(xp < xpneed)
            {
                xp++;
            }
            else
            {
                xpneed = floor(xpneed*1.1);
                xp = 0;
                level++;
            }
        }
        //background
        GRRLIB_DrawImg(0,0, gfx_grassbg, 0, 1, 1, c_white);
        GRRLIB_DrawImg(256,0, gfx_grassbg, 0, 1, 1, c_white);
        GRRLIB_DrawImg(512,0, gfx_grassbg, 0, 1, 1, c_white);
        GRRLIB_DrawImg(0,256, gfx_grassbg, 0, 1, 1, c_white);
        GRRLIB_DrawImg(256,256, gfx_grassbg, 0, 1, 1, c_white);
        GRRLIB_DrawImg(512,256, gfx_grassbg, 0, 1, 1, c_white);

        //rotation fo the fireball
        if(fireballrot < 26)
        {
            fireballrot++;
        }
        else
        {
            fireballrot = 0;
        }
       
        //draw fireballs
        for (int i = 0; i < maxfireballs; i++) {
            if (fireball[i][2] != 0) 
            {
                //we want them to move
                fireball[i][0] += 4;
                if(fireball[i][0] > 640)
                {
                    destroy_fireball(i);
                    continue;
                }
                GRRLIB_DrawTile(fireball[i][0], (fireball[i][1] << 6)+16, gfx_images32, 0, 1, 1, c_white, fireballrot>>1); //fireball
            }
        }


        //draw towers:
        for (int i = 0; i < 10; i++){
            for(int j = 0; j < 6; j++){
                if(tower[i][j][0] != 0)
                {
                    int i64 = i << 6;
                    int j64 = j << 6;
                    GRRLIB_DrawTile(i64, j64, gfx_images, 0, 1, 1, c_white, ptex[tower[i][j][4]]); //tower image

                    temp = tower[i][j][6];
                    switch(tower[i][j][4])//uhh this is my way to do it!
                    {

                        case 0: //lanzador
                        case 2: //minero
                        case 7: //planta
                        if(tower[i][j][3]>0) //cooldown logic
                        {
                            tower[i][j][3]--;
                        }
                        else
                        {
                            tower[i][j][3] = tower[i][j][5];
                            tower_action(i,j);
                        }
                        default: //walls
                        GRRLIB_DrawImg(i64, j64, gfx_gradient4x, 0, (tower[i][j][0]/temp)*16, 1, c_lime); //healthbar
                        GRRLIB_Printf(i64, j64, font, c_white, 1, "lv%d", tower[i][j][1]); //Nivel
                        GRRLIB_Printf(i64, j64+12, font, c_white, 1, "$%d", tower[i][j][2]);//Precio
                        break;

                        case 1: //lanzador 2
                        if(tower[i][j][3]>0)
                        {
                            tower[i][j][3]--;
                        }
                        else
                        {
                            tower[i][j][3] = tower[i][j][5];
                            tower_action(i,j);
                        }
                        GRRLIB_DrawImg(i64, j64, gfx_gradient4x, 0, (tower[i][j][0]/temp)*16, 1, c_lime); //healthbar
                        break;
                        
                        case 6: //teléfono
                        break;
                    }

                }
            }
        }

    //draw enemies
    for (int i = 0; i < maxenemies; i++)
    {
        if (enemies[i].hp > 0) //is alive?
        {
            if (enemies[i].x < 1) //YOU LOSE!!?
            {
                screen = 2;
            }

            GRRLIB_DrawTile(enemies[i].x, enemies[i].y << 6, gfx_images, 0, 1, 1, enemies[i].color, 4); // Imagen del enemigo
            enemies[i].x += enemies[i].speed;

        // Escaneo de fireballs para detectar colisión
        if(enemies[i].resistance == 0)
        {
            for (int j = 0; j < maxfireballs; j++)
            {
                if (enemies[i].y == fireball[j][1])  // Verifica si están en la misma coordenada Y
                {
                    if (fireball[j][0] > enemies[i].x)  // Verifica si la fireball está delante del enemigo
                    {
                        enemies[i].hp -= fireball[j][2]; // Resta vida según el daño de la fireball
                        destroy_fireball(j); //destruir la bola de fuego
                        if (enemies[i].hp <= 0)  // Si la vida baja de 0, destruir enemigo
                        {
                            destroy_enemy(i);
                            break; // Salir del loop de fireballs, ya que el enemigo ha sido eliminado
                        }
                    }
                }
            }
        }
        //Lógica de las torres
        int tx = floor(enemies[i].x /64);
        int ty = enemies[i].y;

        if(tower[tx][ty][0] != 0) //si hay una torre en la posición del enemigo
        {
            if(tower[tx][ty][4] == 6) //si es el teléfono
            {
                destroy_enemy(i);
                destroy_tower(tx,ty);
            }
            else
            {
            tower[tx][ty][0] -= enemies[i].damage;
            if(tower[tx][ty][0] <= 0)
            {
                destroy_tower(tx,ty);
                continue; //ya que la torre se eliminó y no hay nada más que revisar, saltar al siguente loop
            }
            enemies[i].resistance = 0;
            enemies[i].x -= enemies[i].knockback;
            }
        }
    }
}       

        //interfaz
        bar = (xp/xpneed)*160;
        GRRLIB_DrawImg(0, 384, gfx_gradient, 0, 160, 1, c_blue); //fondo items
        GRRLIB_DrawImg(0, 448, gfx_gradient, 0, 160, 0.5, c_gray); //fondo negro progreso
        GRRLIB_DrawImg(0, 448, gfx_gradient, 0, bar, 0.5, c_green); //progreso
        GRRLIB_Printf(0,448, font, c_white, 1, "FPS: %d", FPS); //FPS
        GRRLIB_Printf(576, 384, font, c_white, 1, "Lv: %d", level); // Level
        GRRLIB_Printf(576, 396, font, c_white, 1, "$: %d", money); // Money

        //imágenes de la "tienda"
        for (int i = 0; i < 8; i++) {
            int i64 = i << 6;
            GRRLIB_DrawTile(i64, 384, gfx_images, 0, 1, 1, c_white, ptex[i]); //Ícono
            GRRLIB_Printf(i64, 384, font, c_white, 1, "$%d", pprecio[i]); // Texto debajo de la imagen
        }        
        GRRLIB_DrawTile(512, 384, gfx_images, 0, 1, 1, c_white, 6); //extintor

        GRRLIB_DrawTile(selectorpos << 6, 384, gfx_images, 0, 1, 1, c_white, 5);//selector
        if(selmode == 0)
        {
        GRRLIB_DrawTile(ir1.x, ir1.y, gfx_images32, 0, 1, 1, c_white,14); //Cursor
        }
        else
        {
            GRRLIB_DrawTile(selx << 6, sely << 6, gfx_images, 0, 1, 1, c_black, 5);//selector
        }
        GRRLIB_Render();  // Render the frame buffer to the TV
        FPS = CalculateFrameRate();
        }
        if(screen == 2) //Lose screen :(
        {
            GRRLIB_DrawImg(0,0, gfx_grassbg, 0, 1, 1, c_gray);
            GRRLIB_DrawImg(256,0, gfx_grassbg, 0, 1, 1, c_gray);
            GRRLIB_DrawImg(512,0, gfx_grassbg, 0, 1, 1, c_gray);
            GRRLIB_DrawImg(0,256, gfx_grassbg, 0, 1, 1, c_gray);
            GRRLIB_DrawImg(256,256, gfx_grassbg, 0, 1, 1, c_gray);
            GRRLIB_DrawImg(512,256, gfx_grassbg, 0, 1, 1, c_gray);

            GRRLIB_Printf(0,0,font,c_red,1,"You lose :c");
            GRRLIB_Printf(0,16,font,c_white,1,"Final level: %d",level);
            GRRLIB_Printf(0,32,font,c_white,1,"Final score: %d",score);
            GRRLIB_Printf(0,440,font,c_white,1,"Press home to exit");
            GRRLIB_Render();  // Render the frame buffer to the TV
        }
    }

    //cleanup
    GRRLIB_FreeTexture(gfx_gradient);
    GRRLIB_FreeTexture(gfx_images32);
    GRRLIB_FreeTexture(font);
    GRRLIB_FreeTexture(gfx_images);
    GRRLIB_FreeTexture(gfx_grassbg);
    GRRLIB_FreeTexture(gfx_gradient4x);
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
    

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
/**
 * This function calculates the number of frames we render each second.
 * @return The number of frames per second.
 */
static u8 CalculateFrameRate(void) {
    static u8 frameCount = 0;
    static u32 lastTime;
    static u8 FPS = 0;
    const u32 currentTime = ticks_to_millisecs(gettime());

    frameCount++;
    if(currentTime - lastTime > 1000) {
        lastTime = currentTime;
        FPS = frameCount;
        frameCount = 0;
    }
    return FPS;
}
