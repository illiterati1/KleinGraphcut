// KleinGraphcut.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GraphNode.h"
#include "GraphEdge.h"
#include "Constants.h"
#include <opencv2\opencv.hpp>

using namespace std;
using namespace klein_graphcut;

cv::Mat M= cv::imread("D:\\BSR\\BSDS500\\data\\images\\val\\42049.jpg");
int h = M.rows;
int w = M.cols;

double capacity(cv::Mat& image, pair<int, int> pairOne, pair<int, int> pairTwo)
{
	if (pairOne.first < 0 || pairOne.first >= w || pairOne.second < 0 || pairOne.second >= h ||
		pairTwo.first < 0 || pairTwo.first >= w || pairTwo.second < 0 || pairTwo.second >= h)
		return INF_CAP;
	double diff, weight;
	diff = 0.0;
	auto val = image.at<cv::Vec<char, 3>>(pairOne.second, pairOne.first) - image.at<cv::Vec<char, 3>>(pairTwo.second, pairTwo.first);
	for (int i = 0; i < 3; i++)
	{
		
		diff += val[i] * val[i];
	}
	weight = exp(-sqrt(diff));
	return weight;
}

int main()
{
	cv::Mat& image = M;

	vector<vector<shared_ptr<VertexNode>>> vertices(h, vector<shared_ptr<VertexNode>>(w));
	for (auto y = 0; y < h; ++y)
	{
		for (auto x = 0; x < w; ++x)
		{
			vertices[y][x] = make_shared<VertexNode>();
			vertices[y][x]->setCoordinates(x, y);
		}
	}

	vector<vector<shared_ptr<FaceNode>>> interiorFaces(h - 1, vector<shared_ptr<FaceNode>>(w - 1));
	for (int y = 0; y < h - 1; ++y)
	{
		for (int x = 0; x < w - 1; ++x)
		{
			interiorFaces[y][x] = make_shared<FaceNode>(x, y);
		}
	}
	vector<shared_ptr<FaceNode>> leftFaces(h - 1);
	for (int y = 0; y < leftFaces.size(); ++y)
	{
		leftFaces[y] = make_shared<FaceNode>(-1, y);
	}
	vector<shared_ptr<FaceNode>> rightFaces(h - 1);
	for (int y = 0; y < rightFaces.size(); ++y)
	{
		rightFaces[y] = make_shared<FaceNode>(w - 1, y);
	}
	vector<shared_ptr<FaceNode>> topFaces(w - 1);
	for (int x = 0; x < topFaces.size(); ++x)
	{
		topFaces[x] = make_shared<FaceNode>(x, -1);
	}
	vector<shared_ptr<FaceNode>> bottomFaces(w - 1);
	for (int x = 0; x < bottomFaces.size(); ++x)
	{
		bottomFaces[x] = make_shared<FaceNode>(x, h - 1);
	}
	vector<shared_ptr<GraphEdge>> edges;
	int numEdges = h * w + (h - 1) * (w - 1) + 2 * (h - 1) + 2 * (w - 1) - 1;
	edges.reserve(numEdges);
	shared_ptr<VertexNode> root = make_shared<VertexNode>();
	FaceNode &dualRoot = *topFaces[0];

	// For each interior face, add edge along row
	for (int y = 0; y < h - 1; ++y)
	{
		for (int x = 0; x < w - 2; ++x)
		{
			edges.push_back(make_shared<GraphEdge>(vertices[y][x + 1], vertices[y + 1][x + 1], interiorFaces[y][x], interiorFaces[y][x + 1],
				capacity(image, make_pair(x + 1, y), make_pair(x + 1, y + 1))));
		}
	}
	// For each interior face, add edge along column
	for (int y = 0; y < h - 2; ++y)
	{
		for (int x = 0; x < w - 1; ++x)
		{
			edges.push_back(make_shared<GraphEdge>(vertices[y + 1][x], vertices[y + 1][x + 1], interiorFaces[y + 1][x], interiorFaces[y][x],
				capacity(image, make_pair(x, y + 1), make_pair(x + 1, y + 1))));
		}
	}

	// Add edges along outer faces
	for (int i = 0; i < topFaces.size(); ++i)
	{
		if (i < topFaces.size() - 1)
			edges.push_back(make_shared<GraphEdge>(root, vertices[0][i + 1], topFaces[i], topFaces[i + 1], INF_CAP));
		else
			edges.push_back(make_shared<GraphEdge>(root, vertices[0][i + 1], topFaces[i], rightFaces[0], INF_CAP));

		edges.push_back(make_shared<GraphEdge>(vertices[0][i], vertices[0][i + 1], interiorFaces[0][i], topFaces[i],
			capacity(image, make_pair(0, i), make_pair(0, i + 1))));
	}
	for (int i = 0; i < rightFaces.size(); ++i)
	{
		if (i < rightFaces.size() - 1)
			edges.push_back(make_shared<GraphEdge>(root, vertices[i + 1][w - 1], rightFaces[i], rightFaces[i + 1], INF_CAP));
		else
			edges.push_back(make_shared<GraphEdge>(root, vertices[i + 1][w - 1], rightFaces[i], bottomFaces.back(), INF_CAP));
		edges.push_back(make_shared<GraphEdge>(vertices[i][w - 1], vertices[i + 1][w - 1], interiorFaces[i][w - 2], rightFaces[i],
			capacity(image, make_pair(i, w - 1), make_pair(i + 1, w - 1))));
	}
	for (int i = bottomFaces.size() - 1; i >= 0; --i)
	{
		if (i > 0)
			edges.push_back(make_shared<GraphEdge>(root, vertices[h - 1][i], bottomFaces[i], bottomFaces[i - 1], INF_CAP));
		else
			edges.push_back(make_shared<GraphEdge>(root, vertices[h - 1][i], bottomFaces[i], leftFaces.back(), INF_CAP));
		edges.push_back(make_shared<GraphEdge>(vertices[h - 1][i + 1], vertices[h - 1][i], interiorFaces[h - 2][i], bottomFaces[i],
			capacity(image, make_pair(h - 1, i + 1), make_pair(h - 1, i))));
	}
	for (int i = leftFaces.size() - 1; i >= 0; --i)
	{
		if (i > 0)
			edges.push_back(make_shared<GraphEdge>(root, vertices[i][0], leftFaces[i], leftFaces[i - 1], INF_CAP));
		else
			edges.push_back(make_shared<GraphEdge>(root, vertices[i][0], leftFaces[i], topFaces[0], INF_CAP));
		edges.push_back(make_shared<GraphEdge>(vertices[i + 1][0], vertices[i][0], interiorFaces[i][0], leftFaces[i],
			capacity(image, make_pair(h - 1, i + 1), make_pair(h - 1, i))));
	}

	// Create rotation system for dual vertices
	for (int y = 0; y < h - 1; ++y)
	{
		for (int x = 0; x < w - 1; ++x)
		{
			shared_ptr<FaceNode> up, down, left, right;
			shared_ptr<FaceNode> current = interiorFaces[y][x];
			up = (y == 0) ? topFaces[x] : interiorFaces[y - 1][x];
			down = (y == h - 2) ? bottomFaces[x] : interiorFaces[y + 1][x];
			left = (x == 0) ? leftFaces[y] : interiorFaces[y][x - 1];
			right = (x == w - 2) ? rightFaces[y] : interiorFaces[y][x + 1];

			current->setRotation(up, left);
			current->setRotation(left, down);
			current->setRotation(down, right);
			current->setRotation(right, up);
		}
	}
	for (int i = 0; i < topFaces.size(); ++i)
	{
		shared_ptr<FaceNode> left, right, interior;
		shared_ptr<FaceNode> current = topFaces[i];
		left = (i == 0) ? leftFaces[0] : topFaces[i - 1];
		right = (i == topFaces.size() - 1) ? rightFaces[0] : topFaces[i + 1];
		interior = interiorFaces[0][i];

		current->setRotation(right, left);
		current->setRotation(left, interior);
		current->setRotation(interior, right);

		current = bottomFaces[i];
		left = (i == 0) ? leftFaces.back() : bottomFaces[i - 1];
		right = (i == topFaces.size() - 1) ? rightFaces.back() : bottomFaces[i + 1];
		interior = interiorFaces.back()[i];

		current->setRotation(left, right);
		current->setRotation(right, interior);
		current->setRotation(interior, left);
	}

	for (int i = 0; i < leftFaces.size(); ++i)
	{
		shared_ptr<FaceNode> top, bottom, interior;
		shared_ptr<FaceNode> current = leftFaces[i];
		top = (i == 0) ? topFaces[0] : leftFaces[i - 1];
		bottom = (i == leftFaces.size() - 1) ? bottomFaces[0] : leftFaces[i + 1];
		interior = interiorFaces[i][0];

		current->setRotation(top, bottom);
		current->setRotation(bottom, interior);
		current->setRotation(interior, top);

		current = rightFaces[i];
		top = (i == 0) ? topFaces.back() : rightFaces[i - 1];
		bottom = (i == leftFaces.size() - 1) ? bottomFaces.back() : rightFaces[i + 1];
		interior = interiorFaces[i].back();

		current->setRotation(top, interior);
		current->setRotation(interior, bottom);
		current->setRotation(bottom, top);
	}

	dualRoot.makeRFSTree();
	for (auto& row : interiorFaces)
	{
		for (auto& face : row)
		{
			if (!face->isVisited())
			{
				cout << "Error\n";
			}
		}
	}
	for (auto& face : leftFaces)
	{
		if (!face->isVisited())
		{
			cout << "Error\n";
		}
	}
	for (auto& face : rightFaces)
	{
		if (!face->isVisited())
		{
			cout << "Error\n";
		}
	}
	for (auto& face : topFaces)
	{
		if (!face->isVisited())
		{
			cout << "Error\n";
		}
	}
	for (auto& face : bottomFaces)
	{
		if (!face->isVisited())
		{
			cout << "Error\n";
		}
	}
	root->buildTree();

	if (!root->isVisited())
		cout << "Error\n";
	for (auto& row : vertices)
	{
		for (auto& v : row)
		{
			if (!v->isVisited())
				cout << "Error\n";
		}
	}

	vertices[140][240]->isSource = true;

	// Augment s to t paths
	for (auto& row : vertices)
	{
		for (auto& vertex : row)
		{
			if (vertex->isSource)
			{
				vertex->augmentPathToSink();
			}
		}
	}

	do
	{
		dualRoot.calculateLengths();
	} while (root->relax());

	string windowName = "frame";
	cv::namedWindow(windowName);
	cv::imshow(windowName, M);
	cv::waitKey(0);
	cv::destroyWindow(windowName);
    return 0;
}
