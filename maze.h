#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAZE
#define MAZE

/*
 * number denoting different properties of a vertex
 */
#define BLOCK 0
#define PASS 1
#define START 2
#define DEST 3

/*
 * Rectangular Maze stored as graph
 */
class RectMaze {
  
  private:
    int width_;
    int height_;
    int vertex_size_;
    int edge_size_;
    /*
     * vertex_size_ + edge_size_ + kMetadataSize
     */
    int total_size_;
    /* vertex list:
     * one vertex is represented by two numbers:
     * the first and last index of the edge list, between which 
     * are edges efferent from that vertex (first inclusive, last exclusive)
     */
    int* vertex_list_;
    /*
     * each number is the destination of a directed edge.
     * if it is an undirected graph, edges will be repeated twice
     */
    int* edge_list_;
    /*
     * index of vertices representing starting point and destination.
     */
    int start_vertex_;
    int dest_vertex_;
    
    void ProcessVertexEdgeLists(int** matrix);
    int Linearize(int row, int col);
    
  public:
    /*
     * 2 for width/height
     * 2 for starting/destination vertex
     * 2 for vertex list and edge list sizes
     */
    const int kMetadataSize = 6;

    void ProcessInput();
    int vertex_size() { return vertex_size_; }
    int edge_size() { return edge_size_; }
    int total_size() { return total_size_; }
    int start_vertex() { return start_vertex_; }
    int dest_vertex() { return dest_vertex_; }
    int* vertex_list() { return vertex_list_; }
    int* edge_list() { return edge_list_; }

    /*
     * for transferring data to kernel
     */
    int* MetadataArray();
};

#ifdef NDEBUG
    #define DEBUG(M, ...)
#else
    #define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#endif /* Rect Maze*/

