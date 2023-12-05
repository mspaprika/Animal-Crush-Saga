#pragma once


constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

const char* SPR_BACKGROUND = "Data//Backgrounds//forest3.png";
const char* SPR_CAT = "cat";
const char* SPR_BEAR = "bear";
const char* SPR_FOX = "fox";
const char* SPR_MOUSE = "mouse";
const char* SPR_RABBIT = "rabbit";
const char* SPR_SNAKE = "snake";

const int SQUARE_SIDE_LENGTH = 75; // pixels
const int LINE_WIDTH = 11; // <<  10 + 1 >>  it's actally 10 :).
const int LINE_HEIGHT = 9; // <<  8 + 1 >>  it's actally 8 :).

const int LEFT_OFFSET = 150;

void UpdateGameStates();
void UpdateDestroyed();
void CreateGameObjects();

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
void MatchThreeHorizontal(int id);
void MatchThreeVertical(int id);