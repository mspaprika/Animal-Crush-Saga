#pragma once

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

const char* SPR_BACKGROUND = "Data//Backgrounds//green-background.png";
const char* SPR_CAT = "cat";
const char* SPR_BEAR = "bear";
const char* SPR_FOX = "fox";
const char* SPR_MOUSE = "mouse";
const char* SPR_RABBIT = "rabbit";
const char* SPR_SNAKE = "chick";

const char* SPR_BOOM = "star";

const int SQUARE_SIDE_LENGTH = 75; // pixels
const int LINE_WIDTH = 11; // <<  10 + 1 >>  it's actally 10 :).
const int LINE_HEIGHT = 9; // <<  8 + 1 >>  it's actally 8 :).

const int LEFT_OFFSET = 150;

const int SWEEP_LIMIT = 69;

const Vector2f FALL_SPEED{ 0.f, 8.f };
const Vector2f SWIPE_SPEED{ 5.f, 5.f };

const int NEW_POS_Y = -50;

struct Square
{
	int posX;
	int posY;
	int sideLength;
	int id;
	int horizontalID;
	int verticalID;
	int animalID;

	bool booked{ false };
	bool empty{ false };
};

enum AnimalState
{
	STATE_IDLE,
	STATE_SWIPE,
	STATE_MATCH,
	STATE_FALL,
	STATE_WAIT,

	TOTAL_ANIMAL_STATES,
};

enum GameFlow
{
	STATE_PLAY,

	TOTAL_STATES,
};

enum GameObjectType
{
	TYPE_ANIMAL,
	TYPE_BOOM,
	TYPE_DESTROYED,

	TOTAL_TYPES,
};

enum AnimalType
{
	TYPE_CAT,
	TYPE_BEAR,
	TYPE_FOX,
	TYPE_MOUSE,
	TYPE_RABBIT,
	TYPE_SNAKE,

	TOTAL_ANIMAL_TYPES,
};

struct GameState
{
	int score = 0;
	bool mouseActive{ true };

	GameFlow state = STATE_PLAY;
	AnimalState animalState = STATE_IDLE;
};

void UpdateGameStates();
void UpdateDestroyed();
void CreateGameObjects();
void GenerateAnimals();
int GenerateOneAnimal(Point2f pos);

void Draw();
void DrawGameObjects(int TYPE);
void UpdateGameObjects(int TYPE);
void UpdateAnimals();

void UpdateMouseMovement();

bool IsMouseOver(const GameObject& object);
void CheckObjectCollision();

void CheckAnimalCollision(GameObject& animal);
bool IsAnimalCollidingOtherAnimal(const GameObject& obj_1, const GameObject& obj_2);

void SwipeAnimals(GameObject& obj_1, GameObject& obj_2);
void Swipe(GameObject& animal);

Point2f GetSquarePos(int id);
int GetSquareId(Point2f pos);

void CheckSquare();

void MatchThree(Square& s);
void MatchThreeHorizontal(int id);
void MatchThreeVertical(int id);
void CheckThreeAnimals(int id, int id2, int id3);

void MatchFour(Square& s);
void MatchFourHorizontal(int id);
void MatchFourVertical(int id);
void CheckFourAnimals(int id, int id2, int id3, int id4);

void DockAnimal(GameObject& animal);
void BottomSweep(int id);

void ReleaseNewAnimal(Square& s);
void TopRowCheck();
void Boom(Point2f pos);
void UpdateBoom();

void CheckIfSquareEmpty();
bool IsAnimalInSquare(Square& s);
