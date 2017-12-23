#include "stdafx.h"
#include "GraphEdge.h"
#include "GraphNode.h"

using namespace std;

namespace klein_graphcut
{
	GraphEdge::GraphEdge(std::shared_ptr<VertexNode> tail, std::shared_ptr<VertexNode> head, std::shared_ptr<FaceNode> right, std::shared_ptr<FaceNode> left, double capacity) : 
		_downVertex(tail), _upVertex(head), _rightFace(right), _leftFace(left), _upLength(capacity), _downLength(capacity), inDualTree(false), _originalCapacity(capacity)
	{
		if (capacity < 0)
			throw exception("capacities cannot be negative");
		tail->addEdge(this);
		head->addEdge(this);
		right->addEdge(this);
		left->addEdge(this);
	}

	bool GraphEdge::matchesEnds(VertexNode* nodeA, VertexNode* nodeB)
	{
		return (nodeA == _upVertex.get() && nodeB == _downVertex.get()) || (nodeA == _downVertex.get() && nodeB == _upVertex.get());
	}

	bool GraphEdge::matchesEnds(FaceNode* nodeA, FaceNode* nodeB)
	{
		return (nodeA == _leftFace.get() && nodeB == _rightFace.get()) || (nodeA == _rightFace.get() && nodeB == _leftFace.get());
	}

	bool GraphEdge::matchesEnd(GraphNode* node)
	{
		return node == _upVertex.get() || node == _downVertex.get() || node == _leftFace.get() || node == _rightFace.get();
	}

	VertexNode* GraphEdge::getOpposite(VertexNode* node)
	{
		if (node == _upVertex.get())
			return _downVertex.get();
		else if (node == _downVertex.get())
			return _upVertex.get();
		else
			throw exception("getOpposite got an invalid vertex node");
	}

	FaceNode* GraphEdge::getOpposite(FaceNode* node)
	{
		if (node == _leftFace.get())
			return _rightFace.get();
		else if (node == _rightFace.get())
			return _leftFace.get();
		else
			throw exception("getOpposite got an invalid face node");
	}

	void GraphEdge::changeLengths(VertexNode * tail, double delta)
	{
		if (tail == _upVertex.get())
		{
			_downLength += delta;
			_upLength -= delta;
		}
		else if (tail == _downVertex.get())
		{
			_upLength += delta;
			_downLength -= delta;
		}
		else
			throw exception("changeLength got an invalid endpoint");
	}

	double GraphEdge::getLengthFromTail(VertexNode * tail)
	{
		if (tail == _upVertex.get())
			return _downLength;
		else if (tail == _downVertex.get())
			return _upLength;
		else
			throw exception("getLengthWithTail got an invalid vertex");
	}

	double GraphEdge::getLengthFromTail(FaceNode * tail)
	{
		if (tail == _rightFace.get())
			return _upLength;
		else if (tail == _leftFace.get())
			return _downLength;
		else
			throw exception("getLengthWithTail got an invalid vertex");
	}

	bool GraphEdge::isTense(VertexNode * tail)
	{
		if (tail == _upVertex.get())
			return _downLength + _leftFace->getLength() - _rightFace->getLength() < 0;
		else if (tail == _downVertex.get())
			return _upLength + _rightFace->getLength() - _leftFace->getLength() < 0;
		else
			throw exception("isTense got an invalid tail");
	}

	bool GraphEdge::isBackEdgeInDual(VertexNode* tail)
	{
		auto headFace = getDualHeadFromPrimalTail(tail);
		auto tailFace = getOpposite(headFace);

		if (tailFace->_parent == nullptr)
			return false;
		auto current = tailFace->_parent->getParent();
		while (true)
		{
			if (current == headFace)
				return true;
			if (current->_parent == nullptr)
				return false;
			current = current->_parent->getParent();
		}
	}

	FaceNode * GraphEdge::getDualHeadFromPrimalTail(VertexNode * tail)
	{
		if (tail == _downVertex.get())
			return _leftFace.get();
		else if (tail == _upVertex.get())
			return _rightFace.get();
		else
			throw exception("getDualHead got invalid tail");
	}

	VertexNode * GraphEdge::getTenseVertex()
	{
		if (_downLength < 0)
			return _downVertex.get();
		else if (_upLength < 0)
			return _upVertex.get();
		else
			throw exception("no tense vertex");
	}

	vector<GraphEdge*> GraphEdge::findCycle(VertexNode * tail)
	{
		vector<GraphEdge*> cycle;
		auto headFace = getDualHeadFromPrimalTail(tail);
		auto tailFace = getOpposite(headFace);
		cycle.push_back(this);

		auto current = tailFace;
		while (current != headFace)
		{
			cycle.push_back(current->_parent);
			current = static_cast<FaceNode*>(current->_parent->getParent());
		}
		return cycle;
	}

	GraphNode * GraphEdge::getChild()
	{
		if (inDualTree)
		{
			if (_leftFace->_parent == this)
				return _leftFace.get();
			else if (_rightFace->_parent == this)
				return _rightFace.get();
			else
				throw exception("error in getChild");
		}
		else
		{
			if (_upVertex->_parent == this)
				return _upVertex.get();
			else if (_downVertex->_parent == this)
				return _downVertex.get();
			else
				throw exception("error in getChild");
		}
	}

	GraphNode * GraphEdge::getParent()
	{
		if (inDualTree)
		{
			if (_leftFace->_parent == this)
				return _rightFace.get();
			else if (_rightFace->_parent == this)
				return _leftFace.get();
			else
				throw exception("error in getChild");
		}
		else
		{
			if (_upVertex->_parent == this)
				return _downVertex.get();
			else if (_downVertex->_parent == this)
				return _upVertex.get();
			else
				throw exception("error in getChild");
		}
	}

	void GraphEdge::pivot(VertexNode * head)
	{
		auto oldParent = getParent();
		auto oldChild = getChild();
		if (!inDualTree)
			throw exception("error in pivot");

		if (head == _upVertex.get())
		{
			_upVertex->_parent = this;
			_downVertex->addChild(this);
		}
		else if (head == _downVertex.get())
		{
			_downVertex->_parent = this;
			_upVertex->addChild(this);
		}
		else
			throw exception("error in pivot");

		oldParent->removeChild(this);
		//oldChild->_parent = nullptr;
		inDualTree = false;
	}

	void GraphEdge::pivot(FaceNode * head)
	{
		auto oldParent = getParent();
		auto oldChild = getChild();
		if (inDualTree)
			throw exception("error in pivot");

		if (head == _rightFace.get())
		{
			_rightFace->_parent = this;
			_leftFace->addChild(this);
		}
		else if (head == _leftFace.get())
		{
			_leftFace->_parent = this;
			_rightFace->addChild(this);
		}
		else
			throw exception("error in pivot");

		oldParent->removeChild(this);
		//oldChild->_parent = nullptr;
		inDualTree = true;
	}
}