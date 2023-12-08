#include "decision_tree.h"

#include <cassert>

namespace VEAI
{
	DecisionTreeNode* DecisionTreeNode::next() const
	{
		if (!m_consideration)
			return nullptr;

		if (m_consideration->evaluate())
		{
			return m_true.get();
		}
		else
		{
			return m_false.get();
		}
	}

	Option* DecisionTree::evaluate() const
	{
		auto next = m_root.get();
		auto node = next;
		assert(node != nullptr && "Root node has to exist before evalutating the decision tree");
		
		while (next)
		{
			node = next;
			next = node->next();
			// TODO: Maybe check each node for an option and return a vector of options
		}

		return node->option();
	}

} // namespace VEAI