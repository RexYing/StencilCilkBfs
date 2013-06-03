#include "maze.h"

using namespace std;

/*
 * return an array that represents the input
 * index 0: width; 1: height;
 * 2, 3: index of starting and destination vertex;
 * 4, 5: size of vertex_list_ and edge_list_
 * the reset width*height entries are the data
 */
void RectMaze::ProcessInput() {
    cin >> width_ >> height_;
    total_size_ = width_ * height_ + kMetadataSize;

    string line;
    /*
     * read in data into a matrix representing maze
     */
    int** matrix = new int*[height_];
    for (int i = 0; i < height_; i++) {
	matrix[i] = new int[width_];
        cin >> line;
        for (int j = 0; j < width_; j++) {
	    switch(line[j]) {
		case 'X':
		    matrix[i][j] = BLOCK;
		    break;
		case '.':
		    matrix[i][j] = PASS;
		    break;
		case 'S':
		    matrix[i][j] = START;
		    break;
		case 'E':
		    matrix[i][j] = DEST;
		    break;
		default:
		    DEBUG("unrecognized input: %c", line[j]);
	    }
        }
    }
    ProcessVertexEdgeLists(matrix);
    delete matrix;
}

/*
 * linearize 2D array
 */
inline int RectMaze::Linearize(int row, int col) {
    return row * width_ + col;
}

/*
 * fill in vertex_list_ and edge_list_ according to the matrix information.
 */
void RectMaze::ProcessVertexEdgeLists(int** matrix) {

    /* If vertices[x][y] is not zero, it represets the vertex index of that position. */
    int** vertices = new int*[height_];
    for (int i = 0; i < height_; i++)
	vertices[i] = new int[width_]();
    
    /* initialize vertex/edge list */
    vertex_list_ = new int[2 * width_ * height_];
    edge_list_ = new int[2 * ((width_ - 1) * height_ + (height_ - 1) * width_)];

    int vertex_index = 0;
    int edge_index = 0;
    for (int i = 0; i < height_; i++)
	for (int j = 0; j < width_; j++)
	    if (matrix[i][j] != BLOCK) {
		vertices[i][j] = vertex_index;
		vertex_index++;
	    }
     
    vertex_index = 0;
    for (int i = 0; i < height_; i++)
	for (int j = 0; j < width_; j++) {
            if (matrix[i][j] == BLOCK)
		continue;

	    // check if the vertex is starting point or destination
	    if (matrix[i][j] == START)
		start_vertex_ = vertex_index;
	    if (matrix[i][j] == DEST)
		dest_vertex_ = vertex_index;

	    //start edge index for that vertex
	    vertex_list_[vertex_index] = edge_index;
	    vertex_index++;

	    if ((i > 0) && (matrix[i - 1][j] != BLOCK)) {
	        //edge_list_[edge_index] = Linearize(i - 1, j); // destination in edge list
		edge_list_[edge_index] = vertices[i - 1][j];
		edge_index++;
       	    }
            if ((i < height_ - 1) && (matrix[i + 1][j] != BLOCK)) {
                //edge_list_[edge_index] = Linearize(i + 1, j);
		edge_list_[edge_index] = vertices[i + 1][j];
		edge_index++;
	    }
	    if ((j > 0) && (matrix[i][j - 1] != BLOCK)) {
		//edge_list_[edge_index] = Linearize(i, j - 1);
		edge_list_[edge_index] = vertices[i][j - 1];
		edge_index++;
	    }
	    if ((j < width_ - 1) && (matrix[i][j + 1] != BLOCK)) {
		//edge_list_[edge_index] = Linearize(i, j + 1);
		edge_list_[edge_index] = vertices[i][j + 1];
		edge_index++;
	    }
	}
    /*
     * set the size of array
     */
    vertex_size_ = vertex_index;
    edge_size_ = edge_index;
    total_size_ = vertex_size_ + edge_size_ + kMetadataSize;
}

int* RectMaze::MetadataArray() {
    int* metadata = new int[kMetadataSize];
    metadata[0] = width_;
    metadata[1] = height_;
    metadata[2] = start_vertex_;
    metadata[3] = dest_vertex_;
    metadata[4] = vertex_size_;
    metadata[5] = edge_size_;
    return metadata;
}
