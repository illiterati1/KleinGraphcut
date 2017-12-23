#pragma once
#include "stdafx.h"

namespace klein_graphcut
{
	class VertexNode;
	class FaceNode;
	class GraphNode;

	class GraphEdge
	{
	public:
		GraphEdge() = default;
		GraphEdge(std::shared_ptr<VertexNode> tail, std::shared_ptr<VertexNode> head, std::shared_ptr<FaceNode> right, std::shared_ptr<FaceNode> left, double capacity);
		bool matchesEnds(VertexNode* nodeA, VertexNode* nodeB);
		bool matchesEnds(FaceNode* nodeA, FaceNode* nodeB);
		bool matchesEnd(GraphNode* node);
		VertexNode* getOpposite(VertexNode* node);
		FaceNode* getOpposite(FaceNode* node);
		double getOriginalCap()
		{
			return _originalCapacity;
		}
		void changeLengths(VertexNode* tail, double delta);
		double getLengthFromTail(VertexNode* tail);
		double getLengthFromTail(FaceNode* tail);
		bool isTense(VertexNode* tail);
		bool isBackEdgeInDual(VertexNode* tail);
		FaceNode* getDualHeadFromPrimalTail(VertexNode* tail);
		VertexNode* getTenseVertex();

		VertexNode* getUpVertex()
		{
			return _upVertex.get();
		}
		VertexNode* getDownVertex()
		{
			return _downVertex.get();
		}

		std::vector<GraphEdge*> findCycle(VertexNode* tail);

		GraphNode* getChild();
		GraphNode* getParent();

		void pivot(VertexNode* head);
		void pivot(FaceNode* head);

		bool inDualTree;

	private:
		double _upLength, _downLength;
		const double _originalCapacity;
		std::shared_ptr<VertexNode> _upVertex, _downVertex;
		std::shared_ptr<FaceNode> _rightFace, _leftFace;
	};
}
