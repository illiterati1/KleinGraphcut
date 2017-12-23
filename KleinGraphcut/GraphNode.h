#pragma once
#include "stdafx.h"

namespace klein_graphcut
{
	class GraphEdge;

	class GraphNode
	{
	public:
		GraphNode() : _deleted(false), _visited(false), _parent(nullptr), childrenAdded(false) {}
		bool isVisited()
		{
			return _visited;
		}

		void addEdge(GraphEdge* edge);
		void addChild(GraphEdge* edge);
		void removeChild(GraphEdge* edge);
		GraphEdge* _parent;
		bool childrenAdded;

	protected:
		std::vector<GraphEdge*> _edges;
		bool _deleted;
		bool _visited;
		std::vector<GraphEdge*> _children;
	};


	class VertexNode : public GraphNode
	{
	public:
		VertexNode() : _x(-1), _y(-1), isSource(false)
		{}
		void setCoordinates(int x, int y);
		GraphEdge* getEdge(std::shared_ptr<VertexNode> other);
		void buildTree();
		void augmentPathToSink();
		bool relax();

		bool isSource;

		int _x, _y;
	};


	class FaceNode : public GraphNode
	{
	public:
		FaceNode(int x, int y) : x(x), y(y), length(0) {}
		void setRotation(std::shared_ptr<FaceNode> from, std::shared_ptr<FaceNode> to);
		GraphEdge* getEdge(std::shared_ptr<FaceNode> other);
		void makeRFSTree();
		void calculateLengths();
		double getLength()
		{
			return length;
		}

	private:
		std::map<GraphEdge*, GraphEdge*> _rotations;
		int x, y;
		double length;
	};
}