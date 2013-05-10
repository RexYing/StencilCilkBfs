#include "maze.h"

using namespace std;

/*
 * return an array that represents the input
 * index 0: width; 1: height;
 * 2, 3: position of starting point;
 * 4, 5: position of destination
 * the reset width*height entries are the data
 */
int* RectMaze::process_input() {
    const int TASK_DESCRIPTION_SIZE = 6;
    cin >> width_ >> height_;
    data_size_ = (width_ * height_ + TASK_DESCRIPTION_SIZE) * sizeof(int);
    int* array = malloc((width_ * height_ + TASK_DESCRIPTION_SIZE) * sizeof(int));
    array[0] = width_;
    array[1] = height_;
    cin >> array[2] >> array[3] >> array[4] >> array[5];
    string line;
    /*
     * read in "height" lines of strings, each string contains "width" characters.
     * in the array, 0 represents obstacle.
     * the aim is to find a path comprising of 1s.
     */
    int count = 0;
    for (int i = 0; i < height_; i++) {
        cin >> line;
        for (string::iterator line_it = line.begin(); line_it < line.end(); line_it++) {
	    array[TASK_DESCRIPTION_SIZE + count] = *line_it; 
        }
    }
    return array;
}

size_t RectMaze::get_data_size() {
    return data_size_;
}
