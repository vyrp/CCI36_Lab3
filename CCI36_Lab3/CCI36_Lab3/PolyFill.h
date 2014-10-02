#ifndef _POLYFILL_
#define _POLYFILL_

#define MAX_POLY 50

typedef struct {
	int x, y;
} point_type;
typedef struct {
	int n;
	point_type vertex[MAX_POLY];
} polygon_type;
typedef struct {
	int Ymax, Ymin; float Xinter, dx;
} edge_type;
typedef struct {
	int n;
	edge_type edge[MAX_POLY];
} edge_list_type;

typedef struct {
	float x, y;
} float_point_type;
typedef struct {
	int n;
	float_point_type vertex[MAX_POLY];
} float_polygon_type;

void InsertVertex(polygon_type &poly, int x, int y);

//void ScanFillPolygon(polygon_type poly);
#endif
