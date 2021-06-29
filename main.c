#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <math.h>
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include <SOIL.h>

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB8;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB8 *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void uploadTexture();
void seamcarve(int targetWidth); // executa o algoritmo
void freemem();                  // limpa memória (caso tenha alocado dinamicamente)

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];
Img *source;
Img *mask;
Img *target;

// Imagem selecionada (0,1,2)
int sel;

boolean first = TRUE;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB8 *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}


//
// Implemente AQUI o seu algoritmo
void seamcarve(int targetWidth)
{
    // Aplica o algoritmo e gera a saida em target->img...

    RGB8(*ptr)[target->width] = (RGB8(*)[target->width])target->img;
    RGB8(*ptrM)[mask->width] = (RGB8(*)[mask->width])mask->img;
    RGB8(*ptrS)[source->width] = (RGB8(*)[source->width])source->img;
    int copiaMascara[mask->height][mask->width];

    for (int y = 0; y < target->height; y++){
        for (int x = 0; x < target->width; x++){
            ptr[y][x].r = ptrS[y][x].r;
            ptr[y][x].g = ptrS[y][x].g;
            ptr[y][x].b = ptrS[y][x].b;

            if(ptrM[y][x].r < 50 && ptrM[y][x].g > 150 && ptrM[y][x].b < 50){
                copiaMascara[y][x] = 2;
            }else if(ptrM[y][x].r > 150 && ptrM[y][x].g < 50 && ptrM[y][x].b < 50){
                copiaMascara[y][x] = 1;         
            }else{
                copiaMascara[y][x] = 0;
            }
        }
    }

    for(int i = targetW; i < target->width; i++){

        int deltaX;
        int deltaY;
        int eng[target->height][target->width];
        int engTotal[target->height][target->width];

        //calcula energia
        for (int y = 0; y < target->height; y++){
            for (int x = 0; x < targetW; x++){
                //if(copiaMascara[y][x] == 2){
                    //deltaX = 1000000;
                    //deltaY = 0;
                //}else if(copiaMascara[y][x] == 1){
                    //deltaX = -1000000;
                    //deltaY = 0;
                //}else{
                    if(y == 0){
                        deltaY = pow(ptr[target->height-1][x].r - ptr[y+1][x].r, 2) + pow(ptr[target->height-1][x].g - ptr[y+1][x].g, 2) + pow(ptr[target->height-1][x].b - ptr[y+1][x].b, 2);
                    }else if(y==target->height-1){
                        deltaY = pow(ptr[y-1][x].r - ptr[0][x].r, 2) + pow(ptr[y-1][x].g - ptr[0][x].g, 2) + pow(ptr[y-1][x].b - ptr[0][x].b, 2);
                    }else{
                        deltaY = pow(ptr[y-1][x].r - ptr[y+1][x].r, 2) + pow(ptr[y-1][x].g - ptr[y+1][x].g, 2) + pow(ptr[y-1][x].b - ptr[y+1][x].b, 2);
                    }

                    if(x == 0){
                        deltaX = pow(ptr[y][targetW-1].r - ptr[y][x+1].r, 2) + pow(ptr[y][targetW-1].g - ptr[y][x+1].g, 2) + pow(ptr[y][targetW-1].b - ptr[y][x+1].b, 2);
                    }else if(x==targetW-1){
                        deltaX = pow(ptr[y][x-1].r - ptr[y][0].r, 2) + pow(ptr[y][x-1].g - ptr[y][0].g, 2) + pow(ptr[y][x-1].b - ptr[y][0].b, 2);
                    }else{
                        deltaX = pow(ptr[y][x-1].r - ptr[y][x+1].r, 2) + pow(ptr[y][x-1].g - ptr[y][x+1].g, 2) + pow(ptr[y][x-1].b - ptr[y][x+1].b, 2);
                    }
                //}
                eng[y][x] = deltaX + deltaY;
            }
        }

        //calcula energia total
        for (int y = 0; y < target->height; y++){
            for (int x = 0; x < targetW; x++){
                if(y == 0){
                    engTotal[y][x] = eng[y][x];
                }else{
                    if(x == 0){
                        engTotal[y][x] = MIN(engTotal[y-1][x],engTotal[y-1][x+1]) + eng[y][x];
                    }else if(x==targetW-1){
                        engTotal[y][x] = MIN(engTotal[y-1][x-1],engTotal[y-1][x]) + eng[y][x];
                    }else{
                        engTotal[y][x] = MIN(MIN(engTotal[y-1][x-1],engTotal[y-1][x]),engTotal[y-1][x+1]) + eng[y][x];
                    }
                }
                
            }
        }

        int xDoMenorAtual = 0;
        int menor = __INT_MAX__;
        int menorCaminho[target->height];
        int xDoMenorAnterior = 0;

        //primeiro pixel do menor caminho
        for(int x = 0; x < targetW; x++){
            if(menor > engTotal[target->height-1][x]){
                menor = engTotal[target->height-1][x];
                xDoMenorAnterior = x;
            }
        }

        menor = __INT_MAX__;
        menorCaminho[target->height-1] = xDoMenorAnterior;

        //traca o menor caminho
        for(int y = target->height-1; y > 0; y--){
            if(xDoMenorAnterior+1 >= targetW){
                for(int offset = -1; offset < 1; offset++){
                    if(menor > engTotal[y-1][xDoMenorAnterior+offset]){
                        menor = engTotal[y-1][xDoMenorAnterior+offset];
                        xDoMenorAtual = xDoMenorAnterior+offset;
                    }
                }
            }else if (xDoMenorAnterior-1 <= 0){
                for(int offset = 0; offset <= 1; offset++){
                    if(menor > engTotal[y-1][xDoMenorAnterior+offset]){
                        menor = engTotal[y-1][xDoMenorAnterior+offset];
                        xDoMenorAtual = xDoMenorAnterior+offset;
                    }
                }
            }else{
                for(int offset = -1; offset <= 1; offset++){
                    if(menor > engTotal[y-1][xDoMenorAnterior+offset]){
                        menor = engTotal[y-1][xDoMenorAnterior+offset];
                        xDoMenorAtual = xDoMenorAnterior+offset;
                    }
                }
            }
            xDoMenorAnterior = xDoMenorAtual;
            menorCaminho[y-1] = xDoMenorAnterior;
            menor = __INT_MAX__;
        }
    /*
        for(int y = 0; y < target->height; y++){
            ptr[y][menorCaminho[y]].r = ptr[y][menorCaminho[y]].g = ptr[y][menorCaminho[y]].b = 0;
            ptrM[y][menorCaminho[y]].r = ptrM[y][menorCaminho[y]].g = ptrM[y][menorCaminho[y]].b = 0;
        }
    */

        //corta o menor caminho da image
        for(int y = 0; y < target->height; y++){
            for(int x = menorCaminho[y]; x < target->width; x++){
                ptr[y][x].r = ptr[y][x+1].r;
                ptr[y][x].g = ptr[y][x+1].g;
                ptr[y][x].b = ptr[y][x+1].b;

                //copiaMascara[y][x] = copiaMascara[y][x+1];
            }
        }
        
    }
    

    //faz tudo fora do limite branco 
    for(int y = 0; y < target->height; y++){
        for(int x = target->width; x > targetW; x--){
            ptr[y][x].r = ptr[y][x].g = ptr[y][x].b = 255;
        }
    }

    // Chame uploadTexture a cada vez que mudar
    // a imagem (pic[2])
    uploadTexture();
    glutPostRedisplay();
}


void freemem()
{
    // Libera a memória ocupada pelas 3 imagens
    free(pic[0].img);
    free(pic[1].img);
    free(pic[2].img);
}

/********************************************************************
 * 
 *  VOCÊ NÃO DEVE ALTERAR NADA NO PROGRAMA A PARTIR DESTE PONTO!
 *
 ********************************************************************/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if (pic[0].width != pic[1].width || pic[0].height != pic[1].height)
    {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width = pic[1].width;
    pic[2].height = pic[1].height;

    // Ponteiros para as structs das imagens, para facilitar
    source = &pic[0];
    mask = &pic[1];
    target = &pic[2];

    // Largura desejada inicialmente é a largura da janela
    targetW = target->width;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Registra a funcao callback para tratamento das setas
    glutSpecialFunc(arrow_keys);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Máscara : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width * height * 3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char *)pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        freemem();
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if (key == 's')
    {
        seamcarve(targetW);
    }
    glutPostRedisplay();
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        if (targetW <= pic[2].width - 10)
            targetW += 10;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 10)
            targetW -= 10;
        seamcarve(targetW);
        break;
    default:
        break;
    }
}
// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 target->width, target->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, target->img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
