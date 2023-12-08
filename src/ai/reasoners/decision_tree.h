#pragma once

#include "ai/considerations/consideration.h"
#include "ai/options/option.h"

#include <memory>
#include <type_traits>

namespace VEAI
{
	class DecisionTreeNode
	{
	public:
		DecisionTreeNode() = default;
		~DecisionTreeNode() = default;

		template<typename T, typename... Args>
		DecisionTreeNode* addTrue(Args&&... args)
		{
			return DecisionTreeNode::addNode<T>(m_true, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		DecisionTreeNode* addFalse(Args&&... args)
		{
			return DecisionTreeNode::addNode<T>(m_false, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		static std::enable_if_t<std::is_base_of_v<Option, T>, DecisionTreeNode*>
		addNode(std::unique_ptr<DecisionTreeNode>& node, Args&&... args)
		{
			node = std::make_unique<DecisionTreeNode>();
			node->m_option = std::make_unique<T>(std::forward<Args>(args)...);
			return node.get();
		}

		template<typename T, typename... Args>
		static std::enable_if_t<std::is_base_of_v<Consideration, T>, DecisionTreeNode*>
		addNode(std::unique_ptr<DecisionTreeNode>& node, Args&&... args)
		{
			node = std::make_unique<DecisionTreeNode>();
			node->m_consideration = std::make_unique<T>(std::forward<Args>(args)...);
			return node.get();
		}

		DecisionTreeNode* next() const;

		Option* option() const { return m_option.get(); }
		Consideration* consideration() const { return m_consideration.get(); }

	private:
		std::unique_ptr<DecisionTreeNode> m_true;
		std::unique_ptr<DecisionTreeNode> m_false;

		std::unique_ptr<Option> m_option;
		std::unique_ptr<Consideration> m_consideration;
	};

	class DecisionTree
	{
	public:
		DecisionTree() = default;
		~DecisionTree() = default;

		template<typename T, typename... Args>
		DecisionTreeNode* addRoot(Args&&... args)
		{
			return DecisionTreeNode::addNode<T>(m_root, std::forward<Args>(args)...);
		}

		Option* evaluate() const;

	private:
		std::unique_ptr<DecisionTreeNode> m_root;
	};

} // namespace VEAI
