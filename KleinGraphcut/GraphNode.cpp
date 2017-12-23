#include "stdafx.h"
#include "GraphNode.h"
#include "GraphEdge.h"
#include "Constants.h"

using namespace std;

namespace klein_graphcut
{
	void GraphNode::addEdge(GraphEdge* edge)
	{
		if (!edge->matchesEnd(this))
			throw exception("Tried to add an edge that doesn't have the vertex as an endpoint");
		_edges.push_back(edge);
	}

	void GraphNode::addChild(GraphEdge * edge)
	{
		if (find(_children.begin(), _children.end(), edge) != _children.end())
			cout << "added a child a second time; possibly an error\n";
		_children.push_back(edge);
	}

	void GraphNode::removeChild(GraphEdge * edge)
	{
		if (find(_children.begin(), _children.end(), edge) == _children.end())
			throw exception("error in removeChild");
		remove(_children.begin(), _children.end(), edge);
	}

	void VertexNode::setCoordinates(int x, int y)
	{
		_x = x;
		_y = y;
	}

	GraphEdge* VertexNode::getEdge(std::shared_ptr<VertexNode> other)
	{
		for (auto& edge : _edges)
		{
			if (!edge->matchesEnd(this))
				throw exception("Invalid edge detected");
			if (edge->matchesEnds(this, other.get()))
			{
				return edge;
			}
		}
		throw exception("getEdge called with unconnected 'other'");
	}

	void VertexNode::buildTree()
	{
		vector<VertexNode*> stack;
		stack.push_back(this);

		while (!stack.empty())
		{
			auto current = stack.back();
			current->_visited = true;
			stack.pop_back();
			for (auto edge : current->_edges)
			{
				auto other = edge->getOpposite(current);
				if (!other->_visited && !edge->inDualTree)
				{
					other->_parent = edge;
					current->_children.push_back(edge);
					stack.push_back(other);
				}
			}
		}
	}

	void VertexNode::augmentPathToSink()
	{
		double cutSize = 0;
		for (auto& edge : _edges)
		{
			cutSize += edge->getLengthFromTail(this);
		}
		cutSize = -cutSize;
		auto anscestorEdge = _parent;
		auto currentTail = this;
		while (anscestorEdge != nullptr)
		{
			anscestorEdge->changeLengths(currentTail, cutSize);
			currentTail = anscestorEdge->getOpposite(currentTail);
			anscestorEdge = currentTail->_parent;
		}
	}

	bool VertexNode::relax()
	{
		bool retVal = false;
		vector<VertexNode*> stack;
		stack.push_back(this);

		while (!stack.empty())
		{
			auto current = stack.back();
			if (!current->childrenAdded)
			{
				for (auto childEdge : current->_children)
				{
					auto other = childEdge->getOpposite(current);
					stack.push_back(other);
				}
				current->childrenAdded = true;
			}
			
			if (current == stack.back())
			{
				stack.pop_back();
				current->childrenAdded = false;
				if (current->_parent != nullptr && current ->_parent->isTense(current))
				{
					retVal = true;
					if (!current->_parent->isBackEdgeInDual(current))
					{
						auto primalParentEdge = current->_parent;
						auto dualDartHead = primalParentEdge->getDualHeadFromPrimalTail(current);
						auto dualDartTail = primalParentEdge->getOpposite(dualDartHead);
						auto edgeToRemove = dualDartHead->_parent;
						edgeToRemove->getParent()->removeChild(edgeToRemove); // remove old dart from T
						primalParentEdge->getParent()->removeChild(primalParentEdge); // remove d from tau
						primalParentEdge->getChild()->_parent = nullptr;
						dualDartHead->_parent = primalParentEdge;
						dualDartTail->addChild(primalParentEdge);
						primalParentEdge->inDualTree = true;
					}
					else
					{
						auto cycle = current->_parent->findCycle(current);
						for (auto& edge : cycle)
						{
							extern cv::Mat M;
							auto vert1 = edge->getUpVertex();
							auto vert2 = edge->getDownVertex();
							M.at<cv::Vec<char, 3>>(vert1->_y, vert1->_x) = cv::Vec<char, 3>(0, 0, 255);
							M.at<cv::Vec<char, 3>>(vert2->_y, vert2->_x) = cv::Vec<char, 3>(0, 0, 255);
						}
						double length = abs(current->_parent->getLengthFromTail(current));
						auto workingEdge = current->_parent;
						auto tail = current;
						while (!workingEdge->matchesEnd(this))
						{
							workingEdge->changeLengths(tail, length);
							tail = workingEdge->getOpposite(tail);
							workingEdge = tail->_parent;
						}

					}
					return retVal;
				}
			}
		}
		return retVal;
	}


	void FaceNode::setRotation(std::shared_ptr<FaceNode> from, std::shared_ptr<FaceNode> to)
	{
		GraphEdge *fromEdge = getEdge(from);
		GraphEdge *toEdge = getEdge(to);
		_rotations.insert(make_pair(fromEdge, toEdge));
	}

	GraphEdge* FaceNode::getEdge(std::shared_ptr<FaceNode> other)
	{
		for (auto& edge : _edges)
		{
			if (!edge->matchesEnd(this))
				throw exception("Invalid edge detected");
			if (edge->matchesEnds(this, other.get()))
			{
				return edge;
			}
		}
		throw exception("getEdge called with unconnected 'other'");
	}

	void FaceNode::makeRFSTree()
	{
		vector<pair<GraphEdge*, FaceNode*>> stack;
		stack.push_back(make_pair(nullptr, this));

		while (!stack.empty())
		{
			auto stackPair = stack.back();
			stack.pop_back();
			auto current = stackPair.second;
			if (current->_visited)
				continue;
			current->_parent = stackPair.first;
			current->_visited = true;
			vector<pair<GraphEdge*, FaceNode*>> rotation;

			if (current->_parent == nullptr)
			{
				for (auto& edge : _edges)
				{
					auto other = edge->getOpposite(current);
					rotation.push_back(make_pair(edge, other));
				}
			}
			else
			{
				current->_parent->inDualTree = true;
				current->_parent->getOpposite(current)->addChild(current->_parent);
				auto next = current->_rotations[current->_parent];
				while (next != current->_parent)
				{
					auto other = next->getOpposite(current);
					rotation.push_back(make_pair(next, other));
					next = current->_rotations[next];
				}
			}
			
			stack.insert(stack.end(), rotation.rbegin(), rotation.rend());
		}
	}

	void FaceNode::calculateLengths()
	{
		vector<pair<FaceNode*, double>> stack;
		length = 0;
		stack.push_back(make_pair(this, 0));

		while (!stack.empty())
		{
			auto current = stack.back();
			stack.pop_back();
			auto node = current.first;
			auto newLength = current.second;
			node->length = newLength;
			for (auto child : node->_children)
			{
				stack.push_back(make_pair(child->getOpposite(node), node->length + child->getLengthFromTail(node)));
			}
		}
	}

}