#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER

#include "Play.h"
#include "MainGame.h"


GameState gState;

std::vector<Square> squares;


void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground(SPR_BACKGROUND);

	CreateGameObjects();
	GenerateAnimals();
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
				animal.pos = animal.targetPos;

				if (animal.squareId <= SWEEP_LIMIT)
					BottomSweep(animal.squareId);

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
				squares[animal.squareId].empty = true;
				squares[animal.squareId].booked = false;
				TopRowCheck();

				break;
			}
			case STATE_FALL:
			{
				if (animal.pos.y >= animal.targetPos.y)
				{
					DockAnimal(animal);
					if (animal.squareId <= SWEEP_LIMIT)
						BottomSweep(animal.squareId);	

					CheckSquare();
					TopRowCheck();
				}
				break;
			}
			case STATE_WAIT:
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

			int id  = GenerateOneAnimal({ square.posX, square.posY });
			square.animalID = id;

			GameObject& animal = Play::GetGameObject(id);
			animal.squareId = square.id;
			animal.state = STATE_IDLE;
			animal.targetPos = animal.pos;
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
			if (animal.state == STATE_IDLE)
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
		obj_1.velocity.x = SWIPE_SPEED.x;
		obj_2.velocity.x = SWIPE_SPEED.x * (-1);
	}
	else if (obj_1.targetPos.x < obj_1.pos.x)
	{
		obj_1.velocity.x = SWIPE_SPEED.x * (-1);
		obj_2.velocity.x = SWIPE_SPEED.x;
	} 
	else if (obj_1.targetPos.y > obj_1.pos.y)
	{
		obj_1.velocity.y = SWIPE_SPEED.y;
		obj_2.velocity.y = SWIPE_SPEED.y * (-1);
	}
	else if (obj_1.targetPos.y < obj_1.pos.y)
	{
		obj_1.velocity.y = SWIPE_SPEED.y * (-1);
		obj_2.velocity.y = SWIPE_SPEED.y;
	}

	obj_1.state = STATE_SWIPE;
	obj_2.state = STATE_SWIPE;

}

void Swipe(GameObject& animal)
{
	if (animal.pos == animal.targetPos)
	{
		DockAnimal(animal);
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
		MatchThree(s);
	}
}

void MatchThree(Square& s)
{
	if (!s.empty && s.id < squares.size() - 2 && s.horizontalID < LINE_WIDTH - 2)
	{
		if (Play::GetGameObject(s.animalID).state == STATE_IDLE)
			MatchThreeHorizontal(s.id);
	}
	if (!s.empty && s.id < squares.size() - 2 && s.verticalID < LINE_HEIGHT - 2)
	{
		if (Play::GetGameObject(s.animalID).state == STATE_IDLE)
			MatchThreeVertical(s.id);
	}
}

void MatchThreeHorizontal(int id)
{
	int id2 = id + 1;
	int id3 = id + 2;

	GameObject& animal_one = Play::GetGameObject(squares[id].animalID);
	GameObject& animal_two = Play::GetGameObject(squares[id2].animalID);
	GameObject& animal_three = Play::GetGameObject(squares[id3].animalID);


	if (animal_two.state != STATE_IDLE || animal_three.state != STATE_IDLE)
		return;
		
	if (animal_one.animalType == animal_two.animalType && animal_two.animalType == animal_three.animalType)
	{
		animal_one.state = STATE_MATCH;
		animal_two.state = STATE_MATCH;
		animal_three.state = STATE_MATCH;
	}
}

void MatchThreeVertical(int id)
{
	int id2 = id + 10;
	int id3 = id + 20;

	GameObject& animal_one = Play::GetGameObject(squares[id].animalID);
	GameObject& animal_two = Play::GetGameObject(squares[id2].animalID);
	GameObject& animal_three = Play::GetGameObject(squares[id3].animalID);

	if (animal_two.state != STATE_IDLE || animal_three.state != STATE_IDLE)
		return;

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

void BottomSweep(int id)
{
	GameObject& animal_one = Play::GetGameObject(squares[id].animalID);
	Point2f pos = { squares[id].posX, squares[id].posY };

	if (squares[id].empty)
		return; 

	if (squares[id + 10].empty && !(squares[id + 10].booked))
	{
		animal_one.targetPos = { squares[id + 10].posX, squares[id + 10].posY };
		animal_one.velocity = FALL_SPEED;
		animal_one.state = STATE_FALL;	
		squares[id].empty = true;

		squares[id + 10].booked = true;
	}
}

void DockAnimal(GameObject& animal)
{
	animal.velocity = { 0, 0 };
	animal.state = STATE_IDLE;
	animal.squareId = GetSquareId(animal.targetPos);

	squares[animal.squareId].empty = false;
	squares[animal.squareId].animalID = animal.GetId();
	squares[animal.squareId].booked = false;
}

void GenerateAnimals()
{
	Point2f pos = { 0, 0 };

	for (int i = 0; i < LINE_WIDTH; i++)
	{
		pos = { LEFT_OFFSET + SQUARE_SIDE_LENGTH * i, NEW_POS_Y };
		GenerateOneAnimal(pos);
	}
}

int GenerateOneAnimal(Point2f pos)
{
	int id = 0;
	int random = Play::RandomRoll(5);
	
	switch (random)
	{
		case 1:
		{
			id = Play::CreateGameObject(TYPE_ANIMAL, pos, SQUARE_SIDE_LENGTH / 4, SPR_CAT);
			Play::GetGameObject(id).animalType = TYPE_CAT;
			break;
		}
		case 2:
		{
			id = Play::CreateGameObject(TYPE_ANIMAL, pos, SQUARE_SIDE_LENGTH / 4, SPR_BEAR);
			Play::GetGameObject(id).animalType = TYPE_BEAR;
			break;
		}
		case 3:
		{
			id = Play::CreateGameObject(TYPE_ANIMAL, pos, SQUARE_SIDE_LENGTH / 4, SPR_FOX);
			Play::GetGameObject(id).animalType = TYPE_FOX;
			break;
		}
		case 4:
		{
			id = Play::CreateGameObject(TYPE_ANIMAL, pos, SQUARE_SIDE_LENGTH / 4, SPR_MOUSE);
			Play::GetGameObject(id).animalType = TYPE_MOUSE;
			break;
		}
		case 5:
		{
			id = Play::CreateGameObject(TYPE_ANIMAL, pos, SQUARE_SIDE_LENGTH / 4, SPR_SNAKE);
			Play::GetGameObject(id).animalType = TYPE_SNAKE;
			break;
		}
	}

	GameObject& animal = Play::GetGameObject(id);
	animal.state = STATE_WAIT;
	animal.scale = 0.4f;

	return id;
}

void TopRowCheck()
{
	for (int i = 0; i < LINE_WIDTH; i++)
	{
		if (squares[i].empty)
		{
			ReleaseNewAnimal(squares[i]);
		}
	}
}

void ReleaseNewAnimal(Square& s)
{
	std::vector <int> vAnimals = Play::CollectGameObjectIDsByType(TYPE_ANIMAL);

	for (int id : vAnimals)
	{
		GameObject& animal = Play::GetGameObject(id);

		if (animal.state == STATE_WAIT && animal.pos.x == s.posX && !s.booked)
		{
			GenerateOneAnimal({ s.posX, NEW_POS_Y });
			animal.state = STATE_FALL;
			animal.targetPos = { s.posX, s.posY };
			animal.velocity = FALL_SPEED;

			s.booked = true;
		}
	}
}





