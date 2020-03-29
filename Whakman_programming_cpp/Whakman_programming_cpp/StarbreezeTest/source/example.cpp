#include <iostream>
#include <vector>
#include <Windows.h>
#include "sbzwhakman.h"
#include <fstream>
#include <string>
#include <unordered_set> 
class SBZLibraryScope
{
public:
	typedef ISBZLibrary*(*CreateLibraryFunc)();

	SBZLibraryScope()
		: dll(nullptr)
		, lib(nullptr)
	{
		dll = LoadLibraryA("SBZWhakman.dll");
		CreateLibraryFunc create_library = reinterpret_cast<CreateLibraryFunc>(GetProcAddress(dll, "CreateLibrary"));
		lib = create_library();
	}

	~SBZLibraryScope()
	{
		FreeLibrary(dll);
	}

	ISBZLibrary	*library() const
	{
		return lib;
	}
private:
	HMODULE dll;
	ISBZLibrary* lib;
};

class Tile
{
public:
	Tile(int image, int x, int y, bool left, bool right, bool up, bool down, int coin, float rot = 0.0f)
		: image(image)
		, x(x)
		, y(y)
		, rot(rot)
		, left(left)
		, right(right)
		, up(up)
		, down(down)
		, coin(coin)
	{
	}
	
	bool left, right, up, down;
	std::vector<Tile*> neighbors;
	int coin;
	int		image;
	int		x, y;
	float	rot;
	int gCost, hCost;
	Tile* parent;
	//int heapIndex;
	int fCost()
	{
		return gCost + hCost;
	}
	std::vector<Tile*> getNeigbors()
	{
		return neighbors;
	}
	/*int compare_to(Tile* other)
	{
		if (fCost < other->fCost)
		{
			return -1;
		}
		else if (fCost == other->fCost)
		{
			if (hCost < other->hCost)
			{
				return -1;
			}
			else if (hCost == other->hCost)
			{
				return 0;
			}
		}
		return 1;
	}*/
	//std::vector<Coin*> coins;
};

class Actor
{
public:
	Actor(float x, float y)
		:x(x)
		, y(y)
	{

	}
	virtual ~Actor() = default;
	virtual void update(float dt) = 0;
	virtual void draw() = 0;
	float getX()
	{
		return x;
	}
	float getY()
	{
		return y;
	}
protected:
	enum MoveState
	{
		None,
		Up,
		Down,
		Left,
		Right
	} dir;
	float x, y;
private:
};

const float k_WHAKMAN_SPEED = 120;
const float k_GHOST_SPEED = 60;
const float k_WHAKMAN_ANIMATION_FLIP_TIME = 0.3f;
const int tile_size = 64;
//jag skulle vilja ha en refferens till spelaren här och även några listor
//så att jag slipper skicka med listorna till spelaren eller till Tile'sen
//Testade med en separat klass för det men fick det inte att fungera.

class Ghost : public Actor
{
public:
	Ghost(ISBZLibrary* lib, float x, float y, std::vector<Actor*> player, std::vector<Tile*> tiles)
		: Actor(x, y)
		, lib(lib)
		, player(player)
		, tiles(tiles)
	{
		playerX = 0;
		playerY = 0;
		normalImage = lib->load_image("./images/ghost_01.png");
		fearImage = lib->load_image("./images/ghost_02.png");
		drawImage = normalImage;
	}
	~Ghost()
	{
		normalImage->destroy();
	}
	void update(float dt) override
	{
		update_animation(dt);
		update_movement(dt);
	}
	void update_animation(float dt)
	{
		timer += dt;
		if (timer > k_WHAKMAN_ANIMATION_FLIP_TIME)
		{
			timer = 0;
			//frame = (frame + 1) % image.size();
		}
	}
	float getDistanceToPlayer(int x3, int y3)
	{
		float x2 = (player[0]->getX() * player[0]->getX()) - (x3 * x3);
		float y2 = (player[0]->getY() * player[0]->getY()) - (y3 * y3);
		float dist = sqrt(abs(x2) + abs(y2));
		return dist;
	}
	int getDistance(Tile* from, Tile* to)
	{

		int y = (from->y * from->y) - (to->y * to->y);
		int x = (from->x * from->x) - (to->x * to->x);
		int dist = (int)sqrt(abs(y) + abs(x));
		return dist;
	}
	void fear()
	{
		scatter = 5.f;
		chasePlayer = 20.f;
		drawImage = fearImage;
		oldDestination = NULL;
		action = Scatter;
		std:: string showFear = "\n ghost gets feared";
		OutputDebugStringA(showFear.c_str());
	}
	void draw() override
	{
		drawImage->draw(static_cast<int>(x), static_cast<int>(y), rotation);
	}
	int getY()
	{
		return y;
	}
	int getX()
	{
		return x;
	}
private:
	int playerX, playerY;

	void update_movement(float dt)
	{
		Tile* newDestination = NULL;
		State newAction;
		if (action == FollowPlayer)
		{
			chasePlayer -= dt;
			if (player[0]->getX() < 0 || player[0]->getX() > 640 || player[0]->getY() < 0 || player[0]->getY() > 640)
			{
			}
			else
			{
				for (Tile* tile : tiles)
				{
					if (player[0]->getX() + 32 >= tile->x &&
						player[0]->getX() + 32 <= tile->x + tile_size &&
						player[0]->getY() + 32 >= tile->y &&
						player[0]->getY() + 32 <= tile->y + tile_size)
					{
						newDestination = tile;
					}
				}
			}
			if (chasePlayer <= 0)
			{
				action = Scatter;
				chasePlayer = 20.f;
				oldDestination = NULL;
				return;
			}
		}
		else if (action == Scatter)
		{
			scatter -= dt;
			int distance = std::distance(tiles.begin(), tiles.end());
			int rnd = rand() % distance + 1;
			newDestination = tiles[distance];
			if (scatter <= 0)
			{
				action = FollowPlayer;
				scatter = 5.f; 
				oldDestination = NULL;
				drawImage = normalImage;
				return;
			}
		}

		if (newDestination == NULL || newDestination == oldDestination)
		{

		}
		else
		{
			walkPath = getClosestPath(newDestination);
			oldDestination = newDestination;
		}

		
		if (walkPath.size() != 0)
		{
			Tile* currentTile = walkPath[0];
			if (getTile(x, y) == currentTile)
			{
				walkPath.erase(walkPath.begin());
				Tile* currentTile = walkPath[0];
			}
				
			if (abs(currentTile->x -x) < abs(currentTile->y -y))
			{
				int direction = currentTile->y - y;
				if (direction < 0)
				{
					dir = Up;
				}
				else
				{
					dir = Down;
				}
			}
			else
			{
				int direction = currentTile->x - x;
				if (direction < 0)
				{
					dir = Left;
				}
				else
				{
					dir = Right;
				}
			}

			int dx = 0, dy = 0;
			switch (dir)
			{
			case Up:
				dy = -1;
				x = currentTile->x;
				break;
			case Down:
				dy = 1;
				x = currentTile->x;
				break;
			case Right:
				dx = 1;
				y = currentTile->y;
				break;
			case Left:
				dx = -1;
				y = currentTile->y;
				break;
			default:
				None;
				break;
			}
			y += k_GHOST_SPEED * dy * dt;
			x += k_GHOST_SPEED * dx * dt;
		}
	}
	Tile* getTile(float x, float y)
	{
		for (Tile *tile : tiles)
		{
			if (x+32 >= tile->x &&
				x+32 <= tile->x + tile_size &&
				y+32 >= tile->y &&
				y+32 <= tile->y + tile_size)
			{
				return tile;
			}
		}
		return NULL;
	}
	std::vector<Tile*> getClosestPath(Tile* to)
	{
		Tile* targetTile = getTile(player[0]->getX(), player[0]->getY());
		
		Tile* startTile = getTile(x, y);
		if (targetTile == NULL || startTile== NULL)
		{
			std::vector<Tile*>empty;
			return empty;
		}
		std::list<Tile*> openSet;
		std::unordered_set<Tile*> closedSet;
		openSet.push_back(startTile);
		
		while (openSet.size() > 0)
		{
			Tile* currentTile = openSet.front();
			//här skulle jag vilja använda en heap istället, jag har tyvärr inte hittat ett sätt att få in en heap på
			//och jag fick inte min egna att fungera tyvärr. Detta är ett ganska så icke optimerat sätt att använda sig av
			//pathfindig på
			for (Tile* tile : openSet)
			{
				
				if (tile->fCost() < currentTile->fCost() || tile->fCost() == currentTile->fCost()
					&& tile->hCost < currentTile->hCost)
				{
					currentTile = tile;
				}
			}
			int pos = -1;
			bool remove = false;
			for (Tile* tile : openSet)
			{
				if (tile == currentTile)
				{
					remove = true;
					closedSet.insert(currentTile);
				}
			}
			if (remove)
			{
				openSet.remove(currentTile);
				
			}
		
			if (currentTile == targetTile)
			{
				return retracePath(startTile, targetTile);
				
			}
			for (Tile* neighbour : currentTile->getNeigbors())
			{
				 
				std::unordered_set<Tile*>::const_iterator it = closedSet.find(neighbour);
				if (it != closedSet.end())
				{
					continue;
				}
				
				int newMovementCostToNeighbour = currentTile->gCost + 64;

				//verkar inte finnas en cointain funktion i unordered_set (eller jag vet att det finns, goole sa det till mig 
				//men kanske inte i denna version) så jag kör min egen
				bool openContains = false;
				for (Tile* tile : openSet)
				{
					if (tile == neighbour)
					{
						openContains = true;
						break;
					}
						
				}
				if (newMovementCostToNeighbour < neighbour->gCost || !openContains)
				{
					neighbour->gCost = newMovementCostToNeighbour;
					neighbour->hCost = getDistance(neighbour, targetTile);
					neighbour->parent = currentTile;

					if (!openContains)
					{
						openSet.push_back(neighbour);
					}
				}
			}
		}
		std::vector<Tile*> empty;
		return empty;

	}

	std::vector<Tile*> retracePath(Tile *start, Tile *end)
	{
		std::vector<Tile*> path;
		Tile* currentTile = end;
	/*	std::string tile_x = "\nTile start: " + std::to_string(currentTile->x) + " " + std::to_string(currentTile->y);
		OutputDebugStringA(tile_x.c_str());*/
		while (currentTile != start)
		{
			path.push_back(currentTile);
			currentTile = currentTile->parent;
		}
		std::vector<Tile*> reversed;
		for (int c = path.size() - 1; c > -1; c--)
		{
			/*std::string tile_x = "\n reversed path x & y: " + std::to_string(path[c]->x) +" " + std::to_string(path[c]->y);
			OutputDebugStringA(tile_x.c_str());*/
			reversed.push_back(path[c]);
		}
		return reversed;
		
	}
	

	ISBZLibrary* lib;
	IImage* normalImage;
	IImage* fearImage;
	IImage* drawImage;
	std::vector<IImage*> images;
	std::vector<Tile*> tiles;
	int frame = 0;
	float rotation = 0.f;
	float timer = 0.f;
	float chasePlayer, scatter;
	std::vector<Actor*>player;
	std::vector<Tile*> walkPath;
	Tile* oldDestination;
	enum State
	{
		FollowPlayer,
		Scatter
	} action;
};

class Whakman : public Actor
{
public:
	Whakman(ISBZLibrary* lib, std::vector<Tile*> tiles, float x, float y, bool *fear, int *score)
		: Actor(x,y)
		,lib(lib)
		,tiles(tiles)
		,fear(fear)
		,score(score)
	{
		oldDir = None;
		images.push_back(lib->load_image("./images/whakman_01.png"));
		images.push_back(lib->load_image("./images/whakman_02.png"));
	}

	~Whakman()
	{
		for (auto image : images)
		{
			image->destroy();
		}
	}

	void update(float dt) override
	{
		update_animation(dt);
		update_movement(dt);
	}

	void draw() override
	{
		images[frame]->draw(static_cast<int>(x), static_cast<int>(y), rotation);
	}
	int getX()
	{
		return x;
	}
	int getY()
	{
		return y;
	}
private:
	void update_animation(float dt)
	{
		timer += dt;
		if (timer > k_WHAKMAN_ANIMATION_FLIP_TIME)
		{
			timer = 0;
			frame = (frame + 1) % images.size();
		}
	}

	void update_movement(float dt)
	{
		int keys[8];
		int nr_pressed = lib->pressed_keys(keys, 8);
		float dx = 0, dy = 0;
		if (nr_pressed > 0)
		{
			for (int i = 0; i < nr_pressed; ++i)
			{
				switch (keys[i])
				{
				case 'w':
				case ISBZLibrary::KC_UP:
					rotation = 270.f;
					dir = Up;
					break;
				case 'a':
				case ISBZLibrary::KC_LEFT:
					rotation = 180.f;
					dir = Left;
					break;
				case 's':
				case ISBZLibrary::KC_DOWN:
					rotation = 90.f;
					dir = Down;
					break;
				case 'd':
				case ISBZLibrary::KC_RIGHT:
					rotation = 0.f;
					dir = Right;
					break;
				}
			}
		}
		for (Tile* tile : tiles)
		{
			if (x + 32 >= tile->x &&
				x + 32 <= tile->x + tile_size &&
				y + 32 >= tile->y &&
				y + 32 <= tile->y + tile_size)
			{

				if (tile->coin == 1)
				{
					tile->coin = 0;
				}
				else if (tile->coin == 2)
				{
					tile->coin = 0;
					*fear = true;
				}
				switch (dir)
				{
				case Up:
					if (!tile->up)
					{
						dir = oldDir;
					}
					else
					{
						x = tile->x;
					}
					break;
				case Down:
					if (!tile->down)
					{
						dir = oldDir;
					}
					else
					{
						x = tile->x;
					}
					break;
				case Right:
					if (!tile->right)
					{
						dir = oldDir;
					}
					else
					{
						y = tile->y;
					}
					break;
				case Left:
					if (!tile->left)
					{
						dir = oldDir;
					}
					else
					{
						y = tile->y;
					}
					break;
				default:
					None;
					break;
				}
				break;
			}
		}
		oldDir = dir;

		switch (dir)
		{
		case Up:
			dy = -1;
			break;
		case Down:
			dy = 1;
			break;
		case Right:
			dx = 1;
			break;
		case Left:
			dx = -1;
			break;
		default:
			None;
			break;
		}
		
		y += k_WHAKMAN_SPEED * dy * dt;
		x += k_WHAKMAN_SPEED * dx * dt;
		if (x < -64)
		{
			x = 640;
		}
		else if (x > 640)
		{
			x = -64;
		}
		else if (y < -64)
		{
			y = 640;
		}
		else if (y > 640)
		{
			y = -64;
		}
	}
	MoveState oldDir;
	std::vector<IImage*> images;
	std::vector<Tile*> tiles;
	int frame = 0;
	float rotation = 0.f;
	float timer = 0.f;
	bool* fear;
	int* score;
	ISBZLibrary* lib;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmd, int nshow)
{
	bool fear = false;
	
	SBZLibraryScope library_scope;
	int coinImage = 2, skullImage = 3;
	int score, deadGhost=0;
	auto *lib = library_scope.library();
	lib->init(640, 640);
	IFont *font = lib->load_font("fonts/LondonBetween.ttf", 30);

	std::vector<IImage*> images;
	images.push_back(lib->load_image("images/wall_cross.png"));
	images.push_back(lib->load_image("images/wall_straight.png"));
	images.push_back(lib->load_image("images/coin.png"));
	images.push_back(lib->load_image("images/skull.png"));

	std::vector<Tile*> tiles;
	std::vector<Actor*> player;
	std::list<Ghost*> ghosts;
	std::list<Ghost*> removeGhosts;
	for (int y = 0; y < 10; ++y)
	{
		for (int x = 0; x < 10; ++x)
		{
			int coin = 1;
			if (x == 5 && y == 0 || x == 5 && y == 8)
			{
				coin = 2;
			}
			bool xtile = (y % 2) == 0;
			bool ytile = (x % 3) == 0;
			if (xtile && ytile)
			{
				tiles.push_back(new Tile(0, x * 64, y * 64, true, true, true, true,coin, 0));
			}
			else if (xtile)
			{
				tiles.push_back(new Tile(1, x * 64, y * 64,true, true, false,false, coin, 0));
			}
			else if (ytile)
			{
				tiles.push_back(new Tile(1, x * 64, y * 64,false,false,true,true,coin, 90));
			}
		}
	}
	//lägger gör så att alla tiles håller koll på sina grannar
	//det här sättet att göra det på är rätt kostsamt och jag borde
	//göra på ett annat sätt, men för tillfället duger det.
	for (auto* tile : tiles)
	{
		if (tile->up)
		{

			for (auto* tiley : tiles)
			{
				if (tile->y == tiley->y - 64 && tile->x == tiley->x)
				{
					tile->neighbors.push_back(tiley);
				}
			}

		}
		if (tile->down)
		{

			for (auto* tiley : tiles)
			{
				if (tile->y == tiley->y + 64 && tile->x == tiley->x)
				{
					tile->neighbors.push_back(tiley);
				}
			}


		}
		if (tile->right)
		{

			for (auto* tiley : tiles)
			{
				if (tile->x == tiley->x + 64 && tile->y == tiley->y)
				{
					tile->neighbors.push_back(tiley);
				}

			}
		}
		if (tile->left)
		{
			if (tile->x != 0)
			{

			}
			for (auto* tiley : tiles)
			{
				if (tile->x == tiley->x - 64 && tile->y == tiley->y)
				{
					tile->neighbors.push_back(tiley);
				}
			}
		}
	}
	
	player.push_back(new Whakman(lib, tiles,0,0, &fear,&score));
	ghosts.push_back(new Ghost(lib, 576, 0, player, tiles));
	ghosts.push_back(new Ghost(lib, 0, 576, player, tiles));
	ghosts.push_back(new Ghost(lib, 576, 576, player, tiles));
	bool run = true;
	float prev_time = lib->time();
	float time_at_fear;
	bool fearTimer = false;
	while (lib->update() && run)
	{
		if (fear)
		{
			fearTimer = true;
			time_at_fear = lib->time();
			fear = false;
			for (Ghost* ghost : ghosts)
			{
				ghost->fear();
			}
		}
		if (fearTimer)
		{
			if (lib->time() - time_at_fear >= 5.f)
			{
				fearTimer = false;
			}
		}
		/*std:: string showFear = "\n" + std::to_string(fear);
		OutputDebugStringA(showFear.c_str());*/
		float cur_time = lib->time();
		float dt = cur_time - prev_time;
		prev_time = cur_time;

		if (lib->is_down(ISBZLibrary::KC_ESC))
		{
			run = false;
		}

		for (auto *actor : player)
		{
			actor->update(dt);
		}
		for (auto* actor : ghosts)
		{
			actor->update(dt);
		}
		score = -1;
		for (auto *tile : tiles)
		{
			images[tile->image]->draw(tile->x, tile->y, tile->rot);
			if (tile->coin == 1)
			{
				images[coinImage]->draw(tile->x, tile->y, 0);
			}
			else if (tile->coin == 2)
			{
				images[skullImage]->draw(tile->x, tile->y, 0);
			}
			else
			{
				score++;
			}
		}

		for (auto *actor : player)
		{
			actor->draw();
		}
		for (auto* actor : ghosts)
		{
			actor->draw();
		}
		for (Ghost* ghost : ghosts)
		{
			if (player[0]->getX() >= ghost->getX()
				&& player[0]->getX() <= ghost->getX() + 32
				&& player[0]->getY() + 32 <= ghost->getY() + 32
				&& player[0]->getY() + 32 >= ghost->getY()
				||
				player[0]->getX() + 32 <= ghost->getX()+32
				&& player[0]->getX() + 32 >= ghost->getX()
				&& player[0]->getY() + 32 <= ghost->getY() +32
				&& player[0]-> getY() +32 >= ghost->getY())
			{
				if (fearTimer)
				{
					deadGhost + 10;
					removeGhosts.push_back(ghost);
				}
				else
				{
					bool Esc = true;
					while (Esc)
					{
						std::string gameOver = "GAME OVER. \nyour score is: " + std::to_string(score) + " \npress Esc to quit";
						font->draw(2, 200, gameOver.c_str(), IFont::Color(255, 0, 0, 0));
						if (lib->update() && lib->is_down(ISBZLibrary::KC_ESC))
						{
							Esc = false;
						}
					}
					run = false;
				}
			}
			
		}
		for (Ghost* rg : removeGhosts)
		{
			ghosts.remove(rg);
		}
		removeGhosts.clear();
		score += deadGhost;
		std::string stringScore = "Score: " + std::to_string(score);
		font->draw(0, 640 - 30, stringScore.c_str(), IFont::Color(255, 0, 0, 0));
	}
	for (auto actor : player)
	{
		delete actor;
	}
	for (auto actor : ghosts)
	{
		delete actor;
	}
	for (auto image : images)
	{
		image->destroy();
	}

	lib->destroy();

	return 0;
}

//här har jag försökt skriva min egna heap, jag lyckades tyvärr inte
//att få den att fungera i det här programmet men jag tänkte ändå att det kunde
// vara kul att visa upp

//template <class T>
//class Heap : IHeapItem
//{
//	T[] items;
//	int currentItemCount;
//	public Heap(int maxHeapSize)
//	{
//		items = new T[maxHeapSize];
//		sortUp(item);
//		currentItemCount++;
//	}
//
//	public void add(T item)
//	{
//		heapIndex = currentItemCount;
//		item[currentItemCount] = item;
//	}
//	public T remove_first()
//	{
//		T firstItem = item[0];
//		currentItemCount--;
//		items[0] = items.currentItemCount;
//		items[0].heapIndex = 0;
//		sortDown(items[0]);
//		return firstItem;
//	}
//	public int count()
//	{
//		return currentItemCount;
//	}
//	public bool contains(T item)
//	{
//		return items[item.heapIndex] == item;
//	}
//	public void update_item(T item)
//	{
//		sort_up(item);
//	}
//
//	void sort_down(T item)
//	{
//		while (true)
//		{
//			int ChildIndexLeft = item.heapIndex * 2 + 1;
//			int ChildIndexRight = item.heapIndex * 2 + 2;
//			int swapIdex = 0;
//
//			if (ChildIndexLeft < currentItemCount)
//			{
//				swapIdex = ChildIndexLeft;
//
//				if (ChildIndexRight < currentItemCount)
//				{
//					if (items[ChildIndexLeft].compareTo(items[ChildIndexRight]) < 0)
//					{
//						swapIndex = 
//					}
//				}
//				if (item.compareTo(items[swapIndex]) < 0)
//				{
//					swap(item, items[swapIdex]);
//				}
//				else
//				{
//					return;
//				}
//			}
//			else
//			{
//				return;
//			}
//		}
//	}
//	void sort_up(T item)
//	{
//		int prentIndex = (item.heapIndex - 1) / 2;
//		while (true)
//		{
//			T parentItem = item[parentIndex];
//			if (item.compareTo(parentItem) > 0)
//			{
//				swap(item, parantItem);
//			}
//			else
//			{
//				break;
//			}
//			prentIndex = (item.heapIndex - 1) / 2;
//		}
//	}
//	void swap(T itemA, T itemB)
//	{
//		items[itemA.heapIndex] = itemB;
//		items[itemB.heapIndex] = itemA;
//		int tempAindex = itemA.heapIndex;
//		int tempBindex = itemB.heapIndex;
//		itemA.heapIndex = itemBindex;
//		itemB.heapIndex = itemAindex;
//	}
//
//	int compare_to(T item1, T item2)
//	{
//		if (item1 < item2)
//		{
//			return 1;
//		}
//		else if(item1 > item2)
//		{
//			return -1;
//		}
//		else
//		{
//			return 0;
//		}
//	}
//};

//template <class T>
//class IHeapItem
//{
//public:
//	int heapIndex;
//	int compare_to(T otherItem) = 0;
//};