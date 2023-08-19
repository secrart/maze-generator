#include<iostream> 
#include<string>
#include<vector>
#include<random>
#include<functional>

/*

- Written by Joshua Daley (A.K.A Secrart) 2023

Simple maze image generator using Prim's Algorithm.

depends on: stb_image_write.h

*/

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb_image_write.h"

#define CUT_TILE_COLOR  0x000000
#define WALL_TILE_COLOR 0xFFFFFF


typedef unsigned long pixel_type;
typedef unsigned char channel_type; 
#define CHANNEL_NUM 3

#define PA(x, y, w) (((y) * w) + (x))

class MazeCell {

public:
	static const int FRONT_WALL = 1;
	static const int RIGHT_WALL = 2;
	static const int BACK_WALL = 4;
	static const int LEFT_WALL = 8;

public:
	MazeCell() : missing_mask(0), in(false), x(0), y(0){}

public:
	bool GetIn() { return in; }
	void SetIn(bool i) { in = i; }

	void SetPos(int x, int y) {
		this->x = x; 
		this->y = y; 
	}

	std::pair<int, int> GetPos() {
		return std::make_pair(x, y);
	}

private:
	bool in;
	int x, y; 

public:
	int missing_mask;

};

typedef std::vector<std::vector<MazeCell>> Maze;

int ask_maze_size(const std::string& msg);

Maze generate_maze(int x, int y);


void plot_wall(channel_type* point, int x, int y, int w, MazeCell& cell);

void plot_pixel(channel_type* data, int x, int y, int w, unsigned long val);

int main() {

	std::cout << "Robert's shitty maze generator (Even numbers only please)" << std::endl;
	
	std::string input;
	int sizex = ask_maze_size("Enter Maze Size (input*input): ");
	int sizey = sizex;

	auto maze = generate_maze(sizex, sizey);

	int prnt_scl = 2; 

	int psx = sizex * prnt_scl;
	int psy = sizey * prnt_scl; 

	channel_type* grid = new channel_type[(psx + 1) * (psy + 1) * CHANNEL_NUM];
	


	for (int i = 0; i < (psx + 1); i++) {
		for (int j = 0; j < (psy + 1); j ++) {
			plot_pixel(grid, i, j, psx + 1, WALL_TILE_COLOR);
		}
	}

	for (int i = 1; i < psx; i += prnt_scl) {
		for (int j = 1; j < psy; j += prnt_scl) {
			plot_wall(grid, i, j, psx, maze[i / prnt_scl][j / prnt_scl]);
		}
	}

	stbi_write_png("./output.png", psx + 1, psy + 1, CHANNEL_NUM, grid, psx * CHANNEL_NUM);

	//delete[] grid;

	return 0;
}


void plot_pixel(channel_type* data, int x, int y, int w, unsigned long rgb) {

	unsigned char rv = ((rgb >> 16) & 0xFF);
	unsigned char gv = (rgb >> 8) & 0xFF;
	unsigned char bv = (rgb) & 0xFF;

	struct rgb_value { unsigned char r, g, b; };
	auto frameData = (rgb_value*)data;
	frameData[y * w + x] = rgb_value{ rv, gv, bv };

}

void plot_wall(channel_type* grid, int x, int y, int w, MazeCell& cell) {

	plot_pixel(grid, x, y, w, CUT_TILE_COLOR);

	if (cell.missing_mask & MazeCell::FRONT_WALL) {
		plot_pixel(grid, x, y + 1, w, CUT_TILE_COLOR);;
	}

	if (cell.missing_mask & MazeCell::BACK_WALL) {
		plot_pixel(grid, x, y - 1, w, CUT_TILE_COLOR);;
	}

	if (cell.missing_mask & MazeCell::LEFT_WALL) {
		plot_pixel(grid, x - 1, y, w, CUT_TILE_COLOR);;
	}

	if (cell.missing_mask & MazeCell::RIGHT_WALL) {
		plot_pixel(grid, x + 1, y, w, CUT_TILE_COLOR);;
	}

}

int ask_maze_size(const std::string& msg) {

	std::string input;

	while (true) {
		std::cout << msg;
		std::cin >> input;
		int try_size = 0;

		try {
			try_size = std::stoi(input);
		}
		catch (std::invalid_argument e) {
			std::cout << "Invalid Input!" << std::endl;
			input.clear();
			continue;
		}
		catch (std::out_of_range e) {
			std::cout << "Size Too Big!" << std::endl;
			input.clear();
			continue;
		}

		return try_size;
	}

}

std::vector<MazeCell*> find_adjacent(Maze& ref, int x, int y) {

	std::vector<MazeCell*> adjacent;

	if (x < ref.size() - 1) {
		auto& cell = ref[x + 1][y];
		adjacent.push_back(&cell);
	}
	if(y < ref[x].size() - 1) {
		auto& cell = ref[x][y + 1];
		adjacent.push_back(&cell);
	}
	if (x > 0) {
		auto& cell = ref[x - 1][y];
		adjacent.push_back(&cell);
	}
	if (y > 0) {
		auto& cell = ref[x][y - 1];
		adjacent.push_back(&cell);
	}
	return adjacent;

}

template<typename T>
void append_if(std::vector<T>& a, std::vector<T>& b, std::function<bool(T&)> condition) {
	for (auto& e : b) {
		if (condition(e)) {
			a.push_back(e);
		}
	}
}


// It is assumed that B is adjacent to A
std::pair<int, int> get_cut_bitmask(MazeCell& a, MazeCell& b) {

	auto [ax, ay] = a.GetPos(); 
	auto [bx, by] = b.GetPos();

	if (bx > ax)
		return std::make_pair(MazeCell::RIGHT_WALL, MazeCell::LEFT_WALL);
	if (bx < ax)
		return std::make_pair(MazeCell::LEFT_WALL, MazeCell::RIGHT_WALL);
	if (by > ay)
		return std::make_pair(MazeCell::FRONT_WALL, MazeCell::BACK_WALL);
	if (by < ay)
		return std::make_pair(MazeCell::BACK_WALL, MazeCell::FRONT_WALL);
}

Maze generate_maze(int x, int y) {

	Maze result;

	for (int i = 0; i < x; i++) {
		result.push_back(std::vector<MazeCell>());
		for (int j = 0; j < x; j++) {
			result[i].push_back(MazeCell());
			result[i][j].SetPos(i, j);
		}
	}

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> xGen(0, x - 1);
	std::uniform_int_distribution<int> yGen(0, y - 1);

	std::vector<MazeCell*> frontier;

	frontier.push_back(&result[xGen(mt)][yGen(mt)]);

	while (!frontier.empty()) {

		std::uniform_int_distribution<int> fGen(0, frontier.size() - 1);
		MazeCell& current = *frontier[fGen(mt)];
		
		current.SetIn(true);
		
		auto cPos = current.GetPos();
		auto adjacent = find_adjacent(result, cPos.first, cPos.second);

		append_if<MazeCell*>(frontier, adjacent, [](MazeCell* e) -> bool { return !e->GetIn(); });
		frontier.erase(std::find(frontier.begin(), frontier.end(), &current));

		std::vector<MazeCell*> possible_cuts; 

		for (auto& e : adjacent)
			if (e->GetIn()) possible_cuts.push_back(e); 

		switch (possible_cuts.size()) {

		case(0):
			continue; 
		case(1): {
			auto [acut, bcut] = get_cut_bitmask(current, *possible_cuts[0]);
			current.missing_mask |= acut;
			possible_cuts[0]->missing_mask |= bcut;
			break;
		}
		default: {
			std::uniform_int_distribution<int> fGen(0, possible_cuts.size() - 1);
			int index = fGen(mt);
			auto [acut, bcut] = get_cut_bitmask(current, *possible_cuts[index]);
			current.missing_mask |= acut;
			possible_cuts[index]->missing_mask |= bcut;
			break;
		}

		}

	}

	return result; 

}