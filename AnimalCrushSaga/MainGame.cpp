#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER

#include "Play.h"
#include "MainGame.h"



enum GameFlow
{
	STATE_PLAY,

	TOTAL_STATES,
};

enum GameObjectType
{
	TYPE_ANIMAL,

	TOTAL_TYPES,
};

struct GameState
{
	int score = 0;
	bool mouseActive = true;

	GameFlow state = STATE_PLAY;
};

GameState gState;

struct Square
{
	int posX;
	int posY;
	int sideLength;
	int id;
	int horizontalID;
	int verticalID;
	int animalID;
};

Square square;

std::vector<Square> squares;


void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground(SPR_BACKGROUND);

	CreateGameObjects();
}

bool MainGameUpdate( float elapsedTime )
{
	
	UpdateMouseMovement();

	UpdateGameStates();
	Draw();


	return Play::KeyDown( VK_ESCAPE );
}

int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();

	for (int i = 0; i < squares.size(); i++)
	{
		Play::DrawRect({ squares[i].posX - squares[i].sideLength / 2, squares[i].posY - squares[i].sideLength / 2 }, { squares[i].posX + squares[i].sideLength / 2, squares[i].posY + squares[i].sideLength / 2 }, Play::cWhite);
	}

	DrawGameObjects(TYPE_ANIMAL);

	Play::PresentDrawingBuffer();
}

void DrawGameObjects(int TYPE)
{
	std::vector<int> objects = Play::CollectGameObjectIDsByType(TYPE);

	for (int id : objects)
	{
		GameObject& object = Play::GetGameObject(id);
		Play::DrawObjectRotated(object);
	}
}

void UpdateGameObjects(int TYPE)
{
	std::vector<int> objects = Play::CollectGameObjectIDsByType(TYPE);

	for (int id : objects)
	{
		GameObject& object = Play::GetGameObject(id);
		Play::UpdateGameObject(object);
	}
}

void UpdateGameStates()
{
	switch (gState.state)
	{
		case STATE_PLAY:
		{
			UpdateGameObjects(TYPE_ANIMAL);
			UpdateMouseMovement();
			break;
		}
	}
}

void CreateGameObjects()
{
	int id = 0;

	for (int i = 1; i < LINE_WIDTH; i++)
	{
		for (int j = 1; j < LINE_HEIGHT; j++)
		{

			square.posX = LEFT_OFFSET + SQUARE_SIDE_LENGTH * i;
			square.posY = SQUARE_SIDE_LENGTH * j;
			square.sideLength = SQUARE_SIDE_LENGTH;
			square.id = id;
			square.horizontalID = i;
			square.verticalID = j;

			

			Play::DrawRect( { square.posX - square.sideLength / 2, square.posY - square.sideLength / 2 }, { square.posX + square.sideLength / 2, square.posY + square.sideLength / 2 }, Play::cWhite);

			int id = 0;

			switch (Play::RandomRoll(5))
			{
				case 1:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, square.sideLength / 4, SPR_CAT);
					break;
				}
				case 2:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, square.sideLength / 4, SPR_BEAR);
					break;
				}
				case 3:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, square.sideLength / 4, SPR_FOX);
					break;
				}
				case 4:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, square.sideLength / 4, SPR_MOUSE);
					break;
				}
				case 5:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, square.sideLength / 4, SPR_SNAKE);
					break;
				}
			}

			square.animalID = id;
			squares.push_back(square);

			GameObject& animal = Play::GetGameObject(id);
			animal.scale = 0.4f;
			animal.squareId = square.id;
			id++;
		}
	}
}

void UpdateMouseMovement()
{
	if (Play::GetMouseButton(Play::LEFT) && gState.mouseActive)
	{
		CheckObjectCollision();
	}
}

void CheckObjectCollision()
{
	std::vector<int> vAnimals = Play::CollectGameObjectIDsByType(TYPE_ANIMAL);

	for (int i = 0; i < vAnimals.size(); i++)
	{
		GameObject& animal = Play::GetGameObject(vAnimals[i]);

		if (IsMouseOver(animal) )
		{
			CheckAnimalCollision(animal);
			animal.pos = Play::GetMousePos();
		}
	}
}

void CheckAnimalCollision(GameObject& animal)
{
	std::vector <int> vAnimals = Play::CollectGameObjectIDsByType(TYPE_ANIMAL);

	for (int id : vAnimals)
	{
		GameObject& otherAnimal = Play::GetGameObject(id);

		Point2D tempPos1 = { squares[animal.squareId].posX, squares[animal.squareId].posY, };
		Point2D tempPos2 = { squares[otherAnimal.squareId].posX, squares[otherAnimal.squareId].posY, };

		int tempId = squares[animal.squareId].animalID;
		int tempSquareId = squares[animal.squareId].id;

		if (animal.pos != otherAnimal.pos && IsAnimalCollidingOtherAnimal(animal, otherAnimal))
		{
			gState.mouseActive = false;

			animal.pos = tempPos2;
			animal.squareId = otherAnimal.squareId;
			squares[otherAnimal.squareId].animalID = animal.squareId;

			otherAnimal.pos = tempPos1;
			otherAnimal.squareId = tempId;
			squares[tempId].animalID = tempSquareId;

		}
		else
		{
			animal.pos = tempPos1;
			gState.mouseActive = true;
		}

		if (animal.pos == otherAnimal.pos)
		{
			animal.pos = tempPos1;
			otherAnimal.pos = tempPos2;
			gState.mouseActive = true;
		}
	}
}

bool IsMouseOver(const GameObject& object)
{
	if (object.pos.x - object.radius < Play::GetMousePos().x &&
		object.pos.x + object.radius > Play::GetMousePos().x &&
		object.pos.y - object.radius < Play::GetMousePos().y &&
		object.pos.y + object.radius > Play::GetMousePos().y )
		return true;

	return false;
}
	
bool IsAnimalCollidingOtherAnimal(const GameObject& obj_1, const GameObject& obj_2)
{
	if (obj_1.pos.x - obj_1.radius < obj_2.pos.x + obj_2.radius &&
		obj_1.pos.x + obj_1.radius > obj_2.pos.x - obj_2.radius &&
		obj_1.pos.y - obj_1.radius < obj_2.pos.y + obj_2.radius &&
		obj_1.pos.y + obj_1.radius > obj_2.pos.y - obj_2.radius)
		return true;

	return false;
}






