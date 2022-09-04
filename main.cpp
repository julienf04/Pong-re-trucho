// ----------------------- BIBLIOTECAS -----------------------
#include <windows.h>
#include <iostream>
#include <conio.h>

using namespace std;



// ----------------------- CONSTANTES Y STRUCTS --------------
#define PI 3.14159265358979323846264338327950288419716939937510

typedef unsigned int uint;

struct Pos
{
    float x,
          y;
};

struct GameObject
{
    Pos pos;
    int figureLengthVertical;
    const LPCWSTR* figure;
    COORD colliderOffset;
};

const int BUFFERS_LENGTH = 2;
const HANDLE WINAPI BUFFERS[BUFFERS_LENGTH] =
{
    GetStdHandle(STD_OUTPUT_HANDLE),
    CreateConsoleScreenBuffer(GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL)
};

const int WIDTH = 120,
          HEIGHT = 30;

const int FIGURE_PLAYERS_LENGTH = 4,
          FIGURE_BALL_LENGTH = 1;
const LPCWSTR FIGURE_PLAYERS[] =
{
    L"X",
    L"X",
    L"X",
    L"X"
},
       FIGURE_BALL[] =
{
    L"o"
};

const Pos POS_START_PLAYER = { 8, HEIGHT / 2 - FIGURE_PLAYERS_LENGTH / 2 },
          POS_START_AI = { WIDTH - 9, HEIGHT / 2 - FIGURE_PLAYERS_LENGTH / 2 },
          POS_START_BALL = { WIDTH / 2, HEIGHT / 2 };
const int BALL_RAND_POS_X = 40,
          BALL_RAND_POS_Y = 20;

const COORD COLLIDER_OFFSET_PLAYER = { 1, 0 },
            COLLIDER_OFFSET_AI = { -1, 0 },
            POS_MSG_MENU_0 = { 3, 1 },
            POS_MSG_MENU_1 = { 3, 3 },
            POS_MSG_MENU_2 = { 3, 5 },
            POS_MSG_MENU_WINNER = { 10, 8 };

const float SPEED_PLAYERS = 1.0,
            SPEED_BALL_GROWTH_RATE = 0.01;


void InitGameObjects(GameObject &player, GameObject &AI, GameObject &ball)
{
    player.pos = POS_START_PLAYER;
    AI.pos = POS_START_AI;
    ball.pos = POS_START_BALL;

    srand((unsigned)time(0));
    ball.pos.x += (rand() % BALL_RAND_POS_X) - (BALL_RAND_POS_X / 2);
    ball.pos.y += (rand() % BALL_RAND_POS_Y) - (BALL_RAND_POS_Y / 2);

    player.figure = AI.figure = FIGURE_PLAYERS;
    player.figureLengthVertical = AI.figureLengthVertical = FIGURE_PLAYERS_LENGTH;
    ball.figure = FIGURE_BALL;
    ball.figureLengthVertical = FIGURE_BALL_LENGTH;

    player.colliderOffset = COLLIDER_OFFSET_PLAYER;
    AI.colliderOffset = COLLIDER_OFFSET_AI;
    ball.colliderOffset.X = ball.colliderOffset.Y = 0;
}

void InitVariables(float &speedBallX, float &speedBallY, bool &firstUPTouch, bool &firstDownTouch, bool &endGame)
{
    speedBallX = speedBallY = 1.0;
    firstUPTouch = firstDownTouch = true;
    endGame = false;
}

void InitProgram()
{
    COORD coord;
    coord.X = WIDTH;
    coord.Y = HEIGHT;
    for (int i = 0; i < BUFFERS_LENGTH; i++)
    {
        SetConsoleScreenBufferSize(BUFFERS[i], coord);
        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 1;
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(BUFFERS[i], &cursorInfo);
    }

    SetConsoleTitleA("Pong re trucho");
}

void InitGame(GameObject &player, GameObject &AI, GameObject &ball,
    float &speedBallX, float &speedBallY, bool &firstUPTouch, bool &firstDownTouch, bool &endGame)
{
    InitGameObjects(player, AI, ball);
    InitVariables(speedBallX, speedBallY, firstUPTouch, firstDownTouch, endGame);
}

void SetConsoleCursorPos(int buffer, short x, short y)
{
    COORD pos = { x, y };
    SetConsoleCursorPosition(BUFFERS[buffer], pos);
}

COORD PosToCOORD(Pos pos)
{
    COORD coord = { pos.x, pos.y };
    return coord;
}

void ClearConsole(int frameCount)
{
    DWORD ignore;
    COORD pos;
    pos.X = 0;
    pos.Y = 0;
    FillConsoleOutputCharacter(BUFFERS[frameCount%2], ' ', WIDTH * HEIGHT, pos, &ignore);
}

void WriteText(LPCWSTR text, COORD pos, int frameCount)
{
    DWORD ignore;
    WriteConsoleOutputCharacterW(BUFFERS[frameCount%2], text, wcslen(text), pos, &ignore);
}

void SetActiveConsole(int frameCount)
{
    SetConsoleActiveScreenBuffer(BUFFERS[frameCount%2]);
}

void DrawGO(GameObject &go, int frameCount)
{
    DWORD ignore;
    COORD coord = PosToCOORD(go.pos);
    for (int i = 0; i < go.figureLengthVertical; i++, coord.Y++)
    {
        WriteConsoleOutputCharacterW(BUFFERS[frameCount%2], go.figure[i], wcslen(go.figure[i]), coord, &ignore);
    }
    
}

bool IsCollisionPlayerWithBall(GameObject &player, GameObject &ball)
{
    COORD coordPlayer = PosToCOORD(player.pos),
          coordBall = PosToCOORD(ball.pos);
    return (coordBall.X + ball.colliderOffset.X == coordPlayer.X + player.colliderOffset.X)
        && coordBall.Y + (ball.figureLengthVertical - 1) + ball.colliderOffset.Y >= coordPlayer.Y
        && coordBall.Y + ball.colliderOffset.Y <= coordPlayer.Y + (player.figureLengthVertical - 1) + player.colliderOffset.Y;
}

bool IsCollisionAIWithBall(GameObject &AI, GameObject &ball)
{
    COORD coordAI = PosToCOORD(AI.pos),
          coordBall = PosToCOORD(ball.pos);
    return (coordBall.X + ball.colliderOffset.X == coordAI.X + AI.colliderOffset.X)
        && coordBall.Y + (ball.figureLengthVertical - 1) + ball.colliderOffset.Y >= coordAI.Y
        && coordBall.Y + ball.colliderOffset.Y <= coordAI.Y + (AI.figureLengthVertical - 1) + AI.colliderOffset.Y;
}

bool IsCollisionWithUpConsole(GameObject &go)
{
    COORD coord = PosToCOORD(go.pos);
    return coord.Y + go.colliderOffset.Y < 1;
}

bool IsCollisionWithDownConsole(GameObject &go)
{
    COORD coord = PosToCOORD(go.pos);
    return coord.Y + go.colliderOffset.Y + (go.figureLengthVertical - 1) >= HEIGHT - 1;
}

bool IsCollisionWithLeftConsole(GameObject &go)
{
    COORD coord = PosToCOORD(go.pos);
    return coord.X + go.colliderOffset.X < 1;
}

bool IsCollisionWithRightConsole(GameObject &go)
{
    COORD coord = PosToCOORD(go.pos);
    return coord.X + go.colliderOffset.X + wcslen(go.figure[0]) >= WIDTH - 1;
}

void PlayerLogic(GameObject &player, double deltaTime, bool &firstUpTouch, bool &firstDownTouch)
{
    if ((GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W')) && !IsCollisionWithUpConsole(player))
        {
            if (firstUpTouch)
            {
                player.pos.y = (int)player.pos.y;
                firstUpTouch = false;
            }
            else
                player.pos.y -= SPEED_PLAYERS * deltaTime;
        }
        else
            firstUpTouch = true;
        if ((GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S')) && !IsCollisionWithDownConsole(player))
        {
            if (firstDownTouch)
            {
                player.pos.y = (int)(player.pos.y + 1) - 0.00001;
                firstDownTouch = false;
            }
            else
                player.pos.y += SPEED_PLAYERS * deltaTime;
        }
        else
            firstDownTouch = true;
}

void BallLogic(GameObject &ball, GameObject &player, GameObject &AI,
    float &speedBallX, float &speedBallY, double deltaTime, int &dirBallX, int &dirBallY)
{
    ball.pos.x += speedBallX * deltaTime * dirBallX;
        ball.pos.y += speedBallY * deltaTime * dirBallY;

    if (IsCollisionWithUpConsole(ball))
        dirBallY = 1;
    if (IsCollisionWithDownConsole(ball))
        dirBallY = -1;
    if (IsCollisionPlayerWithBall(player, ball))
        dirBallX = 1;
    if (IsCollisionAIWithBall(AI, ball))
        dirBallX = -1;
    
    ((rand() % 2) == 0)
        ? speedBallX += SPEED_BALL_GROWTH_RATE * deltaTime
        : speedBallY += SPEED_BALL_GROWTH_RATE * deltaTime;
}
 
void AILogic(GameObject &AI, GameObject &ball, int dirBallY, float speedBallX, float speedBallY, float deltaTime)
{
    double avgSpeeds = speedBallY / speedBallX;
    double offset = HEIGHT / 2;
    double destiny = ((HEIGHT *
        asin(
            sin(
                (PI / HEIGHT) * (
                    ((AI.pos.x + AI.colliderOffset.X) * avgSpeeds)
                    + (-offset)
                    + (ball.pos.x * -avgSpeeds)
                    + (ball.pos.y * dirBallY)
                    )
                )
            )) / PI) + offset;

    AI.pos.y += AI.pos.y + (AI.figureLengthVertical / 2) < destiny
        ? SPEED_PLAYERS * deltaTime
        : SPEED_PLAYERS * deltaTime * -1;

    if (IsCollisionWithUpConsole(AI))
        AI.pos.y = 0.9999999F;
    else if (IsCollisionWithDownConsole(AI))
        AI.pos.y = HEIGHT - AI.figureLengthVertical;
}

LONGLONG GetTime()
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

void GameLogic(GameObject &player, GameObject &AI, GameObject &ball, uint frameCount, double deltaTime, 
    float &speedBallX, float &speedBallY, int &dirBallX, int &dirBallY,
    bool &firstUpTouch, bool &firstDownTouch, bool &endGame, bool &playerWon)
{
    //// Logica del juego
    BallLogic(ball, player, AI, speedBallX, speedBallY, deltaTime, dirBallX, dirBallY);
    PlayerLogic(player, deltaTime, firstUpTouch, firstDownTouch);
    AILogic(AI, ball, dirBallY, speedBallX, speedBallY, deltaTime);

    if (IsCollisionWithLeftConsole(ball))
    {
        endGame = true;
        playerWon = false;
    }
    else if (IsCollisionWithRightConsole(ball))
    {
        endGame = true;
        playerWon = true;
    }


    //// Dibujar objetos en consola
    ClearConsole(frameCount);

    DrawGO(player, frameCount);
    DrawGO(AI, frameCount);
    DrawGO(ball, frameCount);

    SetActiveConsole(frameCount);
}

void BuildMenu(bool endGame, bool playerWon)
{
    WriteText(L"Hola! Este es el juego Pong, pero re trucho", POS_MSG_MENU_0, 0);
    WriteText(L"Te reto a ganar un solo juego, SOLO UNO, si podes...", POS_MSG_MENU_1, 0);
    WriteText(L"Presiona el numero 1 para comenzar a jugar, o el 2 para salir.", POS_MSG_MENU_2, 0);
    if (endGame)
    {
        LPCWSTR wstr = playerWon
            ? L"GANASTEEEEEEEEEEEEEEEE!!!!!!!!!!"
            : L"Perdiste :(";
        WriteText(wstr, POS_MSG_MENU_WINNER, 0);
    }
}

int main()
{
    //// Definir variables
    GameObject player,
               AI,
               ball;
    bool endProgram = false,
         endGame = false,
         playerWon,
         firstUpTouch,
         firstDownTouch,
         buildMenu = true;
    uint frameCount = 0;
    int dirBallX = -1,
        dirBallY = -1;
    float speedBallX,
          speedBallY;
    double deltaTime = 0.0;

    //// Logica del programa
    InitProgram();
    do
    {
        if (buildMenu)
        {
            buildMenu = false;
            BuildMenu(endGame, playerWon);
        }
        char input;
        input = getch();

        switch (input)
        {
        case '1':
            InitGame(player, AI, ball, speedBallX, speedBallY, firstUpTouch, firstDownTouch, endGame);
            while (!endGame)
            {
                frameCount++;
                LONGLONG time = GetTime();
                GameLogic(player, AI, ball, frameCount, deltaTime, speedBallX, speedBallY,
                    dirBallX, dirBallY, firstUpTouch, firstDownTouch, endGame, playerWon);
                deltaTime = (GetTime() - time) / (double)1000000;
            }
            frameCount = 0;
            ClearConsole(0);
            SetActiveConsole(0);
            buildMenu = true;
            break;
        case '2':
            endProgram = true;
            break;
        }
    } while (!endProgram);
}