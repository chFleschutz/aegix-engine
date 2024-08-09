#pragma once

#include "ai/considerations/consideration.h"
#include "ai/options/option.h"
#include "utils/factory.h"

#include <memory>

namespace Aegix::AI
{
	class TreeNode
	{
	public:
		virtual TreeNode* next() const { return nullptr; }
		virtual std::unique_ptr<Option> createOption() const { return nullptr; }

	protected:
		std::unique_ptr<TreeNode> m_true;
		std::unique_ptr<TreeNode> m_false;

		friend class DecisionTree;
	};

	template <typename T>
	class DecisionNode : public TreeNode
	{
	public:
		template <typename... Args>
		DecisionNode(Args&&... args) : m_decision(std::make_unique<T>(std::forward<Args>(args)...)) {}

		virtual TreeNode* next() const override
		{
			return m_decision->evaluate() ? m_true.get() : m_false.get();
		}

	private:
		std::unique_ptr<T> m_decision;
	};

	template <typename T>
	class OptionNode : public TreeNode
	{
	public:
		template <typename... Args>
		OptionNode(Args&&... args) : factory(std::forward<Args>(args)...) {}

		virtual std::unique_ptr<Option> createOption() const override
		{
			return factory.create();
		}

	private:
		Factory<T> factory;
	};

	class DecisionTree
	{
	public:
		DecisionTree() = default;

		template <typename T, typename... Args>
		TreeNode* addRoot(Args&&... args)
		{
			return addNode<T>(m_root, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static TreeNode* addTrue(TreeNode* node, Args&&... args)
		{
			return addNode<T>(node->m_true, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static TreeNode* addFalse(TreeNode* node, Args&&... args)
		{
			return addNode<T>(node->m_false, std::forward<Args>(args)...);
		}

		std::unique_ptr<Option> evaluate() const 
		{
			auto node = m_root.get();
			auto next = node;

			while (next)
			{
				node = next;
				next = node->next();
			}

			if (node)
				return node->createOption();

			return nullptr;
		}

	private:
		template <typename T, typename... Args>
		static auto addNode(std::unique_ptr<TreeNode>& node, Args&&... args) -> std::enable_if_t<std::is_base_of_v<Aegix::AI::Option, T>, TreeNode*>
		{
			node = std::make_unique<OptionNode<T>>(std::forward<Args>(args)...);
			return node.get();
		}

		template <typename T, typename... Args>
		static auto addNode(std::unique_ptr<TreeNode>& node, Args&&... args) -> std::enable_if_t<std::is_base_of_v<Aegix::AI::Consideration, T>, TreeNode*>
		{
			node = std::make_unique<DecisionNode<T>>(std::forward<Args>(args)...);
			return node.get();
		}

		std::unique_ptr<TreeNode> m_root;
	};
}
