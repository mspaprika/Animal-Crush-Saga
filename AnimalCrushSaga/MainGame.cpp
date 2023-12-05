#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER

#include "Play.h"
#include "MainGame.h"

enum AnimalState
{
	STATE_IDLE,
	STATE_SWIPE,
	STATE_MATCH,
	STATE_FALL,

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

	bool empty{ false };
};

std::vector<Square> squares;


void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground(SPR_BACKGROUND);

	CreateGameObjects();
	CheckSquare();
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

void UpdateAnimals()
{
	std::vector <int> vAnimals = Play::CollectGameObjectIDsByType(TYPE_ANIMAL);

	for (int id : vAnimals)
	{
		GameObject& animal = Play::GetGameObject(id);

		switch (animal.state)
		{
			case STATE_IDLE:
			{
				UpdateMouseMovement();
				
				break;
			}
			case STATE_SWIPE:
			{
				Swipe(animal);
				break;
			}
			case STATE_MATCH:
			{
				animal.type = TYPE_DESTROYED;
				break;
			}
			case STATE_FALL:
			{
				break;
			}
		}
		
		Play::UpdateGameObject(animal);
	}
}

void UpdateGameStates()
{
	switch (gState.state)
	{
		case STATE_PLAY:
		{
			UpdateAnimals();
			CheckSquare();
			break;
		}
	}

	UpdateDestroyed();
}

void CreateGameObjects()
{
	int idSquare = 0;

	for (int i = 1; i < LINE_HEIGHT; i++)
	{
		for (int j = 1; j < LINE_WIDTH; j++)
		{
			Square square;
			square.posX = LEFT_OFFSET + SQUARE_SIDE_LENGTH * j;
			square.posY = SQUARE_SIDE_LENGTH * i;
			square.sideLength = SQUARE_SIDE_LENGTH;
			square.id = idSquare;
			square.horizontalID = j;
			square.verticalID = i;


			Play::DrawRect( { square.posX - square.sideLength / 2, square.posY - square.sideLength / 2 }, { square.posX + square.sideLength / 2, square.posY + square.sideLength / 2 }, Play::cWhite);

			int id = 0;
			int sideLength = square.sideLength / 4;

			switch (Play::RandomRoll(5))
			{
				case 1:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, sideLength, SPR_CAT);
					Play::GetGameObject(id).animalType = TYPE_CAT;
					break;
				}
				case 2:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, sideLength, SPR_BEAR);
					Play::GetGameObject(id).animalType = TYPE_BEAR;
					break;
				}
				case 3:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, sideLength, SPR_FOX);
					Play::GetGameObject(id).animalType = TYPE_FOX;
					break;
				}
				case 4:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, sideLength, SPR_MOUSE);
					Play::GetGameObject(id).animalType = TYPE_MOUSE;
					break;
				}
				case 5:
				{
					id = Play::CreateGameObject(TYPE_ANIMAL, { square.posX, square.posY }, sideLength, SPR_SNAKE);
					Play::GetGameObject(id).animalType = TYPE_SNAKE;
					break;
				}
			}

			square.animalID = id;

			GameObject& animal = Play::GetGameObject(id);
			animal.squareId = square.id;
			animal.state = STATE_IDLE;
			animal.scale = 0.4f;
			idSquare++;

			squares.push_back(square);
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

		if (IsMouseOver(animal))
		{
			CheckAnimalCollision(animal);
			animal.pos = Play::GetMousePos();
			animal.state = STATE_SWIPE;	
		}
		else
		{
			animal.pos = GetSquarePos(animal.squareId);
		}
	}
}

void CheckAnimalCollision(GameObject& animal)
{
	std::vector <int> vAnimals = Play::CollectGameObjectIDsByType(TYPE_ANIMAL);

	for (int id : vAnimals)
	{
		GameObject& otherAnimal = Play::GetGameObject(id);

		if (animal.pos != otherAnimal.pos && IsAnimalCollidingOtherAnimal(animal, otherAnimal))
		{
			SwipeAnimals(animal, otherAnimal);
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

void SwipeAnimals(GameObject& obj_1, GameObject& obj_2)
{
	obj_2.targetPos = GetSquarePos(obj_1.squareId);
	obj_1.targetPos = GetSquarePos(obj_2.squareId);

	if (obj_1.targetPos.x > obj_1.pos.x)
	{
		obj_1.velocity.x = 2.f;
		obj_2.velocity.x = -2.f;
	}
	else if (obj_1.targetPos.x < obj_1.pos.x)
	{
		obj_1.velocity.x = -2.f;
		obj_2.velocity.x = 2.f;		
	} 
	else if (obj_1.targetPos.y > obj_1.pos.y)
	{
		obj_1.velocity.y = 2.f;
		obj_2.velocity.y = -2.f;
	}
	else if (obj_1.targetPos.y < obj_1.pos.y)
	{
		obj_1.velocity.y = -2.f;
		obj_2.velocity.y = 2.f;
	}

	obj_1.state = STATE_SWIPE;
	obj_2.state = STATE_SWIPE;

}

void Swipe(GameObject& animal)
{
	if (animal.pos == animal.targetPos)
	{
		animal.velocity = { 0, 0 };
		animal.state = STATE_IDLE;
		animal.squareId = GetSquareId(animal.targetPos);
		squares[animal.squareId].empty = false;
		squares[animal.squareId].animalID = animal.GetId();
		animal.targetPos = { 0, 0 };
		CheckSquare();
	
		return;
	}

	if (animal.targetPos == Point2f{ 0, 0 })
	{
		return;
	}

	if (animal.velocity.x > 0 && animal.pos.x > animal.targetPos.x)
	{
		animal.velocity.x = 0;
		animal.pos = animal.targetPos;
	}
	else if (animal.velocity.x < 0 && animal.pos.x < animal.targetPos.x)
	{
		animal.velocity.x = 0;
		animal.pos = animal.targetPos;
	}
	else if (animal.velocity.y > 0 && animal.pos.y > animal.targetPos.y)
	{
		animal.velocity.y = 0;		
		animal.pos = animal.targetPos;
	}
	else if (animal.velocity.y < 0 && animal.pos.y < animal.targetPos.y)
	{
		animal.velocity.y = 0;
		animal.pos = animal.targetPos;
	}
}

Point2f GetSquarePos(int id)
{
	return { squares[id].posX, squares[id].posY };
}

int GetSquareId(Point2f pos)
{
	for (Square& s : squares)
	{
		if (s.posX == pos.x && s.posY == pos.y)
			return s.id;
	}
}

void CheckSquare()
{
	for (Square& s : squares)
	{
		if (!s.empty && s.id < squares.size() - 2 && s.horizontalID < LINE_WIDTH - 2)
		{
			MatchThreeHorizontal(s.id);
		}
		if (!s.empty && s.id < squares.size() - 2 && s.verticalID < LINE_HEIGHT - 2)
		{
			MatchThreeVertical(s.id);
		}
	}
}

void MatchThreeHorizontal(int id)
{
	GameObject& animal_one = Play::GetGameObject(squares[id].animalID);
	GameObject& animal_two = Play::GetGameObject(squares[id + 1].animalID);
	GameObject& animal_three = Play::GetGameObject(squares[id + 2].animalID);

		
	if (animal_one.animalType == animal_two.animalType && animal_two.animalType == animal_three.animalType)
	{
		animal_one.state = STATE_MATCH;
		animal_two.state = STATE_MATCH;
		animal_three.state = STATE_MATCH;
	}
}

void MatchThreeVertical(int id)
{
	GameObject& animal_one = Play::GetGameObject(squares[id].animalID);
	GameObject& animal_two = Play::GetGameObject(squares[id + 10].animalID);
	GameObject& animal_three = Play::GetGameObject(squares[id + 20].animalID);

	if (animal_one.animalType == animal_two.animalType && animal_two.animalType == animal_three.animalType)
	{
		animal_one.state = STATE_MATCH;
		animal_two.state = STATE_MATCH;
		animal_three.state = STATE_MATCH;
	}
}

void UpdateDestroyed()
{
	std::vector<int> vDestroyed = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);
	for (int id : vDestroyed)
	{
		Play::DestroyGameObject(id);	
	}
}

