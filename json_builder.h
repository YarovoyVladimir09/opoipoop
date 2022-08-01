#pragma once

#include "json.h"

#include <algorithm>

namespace json {
	
	class Builder {
		friend Node;
	public:
		class DictItemContext;
		class KeyContext;
		class BaseContext;
		class ArrayItemContext;
		class ValueDictContext;

		Builder() = default;
		const KeyContext Key(std::string key);
		BaseContext Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();
		Node Build();

	private:
		Node* root_ = nullptr;
		std::string key_buffer ;
		std::vector<Node*> nodes_stack_;
		bool ready = false;
		bool key_done = false;

	public:
		class BaseContext
		{
		public:
			BaseContext(Builder& build);
			KeyContext Key(std::string key);
			BaseContext Value(Node value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
			BaseContext EndDict();
			BaseContext EndArray();
			Node Build();
			Builder& builder;
		};

		class DictItemContext :public BaseContext
		{
		public:
			DictItemContext(Builder& build);
			DictItemContext& Value(Node value) = delete;
			DictItemContext& StartDict() = delete;
			DictItemContext& StartArray() = delete;
			DictItemContext& EndArray() = delete;
			Node Build() = delete;
		};

		class KeyContext :public BaseContext
		{
		public:
			KeyContext(Builder& build);
			ValueDictContext Value(Node value);
			Builder& Key(std::string key) = delete;
			KeyContext& EndDict() = delete;
			KeyContext& EndArray() = delete;
			Node Build() = delete;
		};

		class ArrayItemContext :public BaseContext
		{
		public:
			ArrayItemContext(Builder& build);
			ArrayItemContext Value(Node value);
			Builder& Key(std::string key) = delete;
			KeyContext& EndDict() = delete;
			Node Build() = delete;
		};

		class ValueDictContext :public BaseContext
		{
		public:
			ValueDictContext(Builder& build);
			Node Build() = delete;
			Builder& Value(Node value) = delete;
			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			Builder& EndArray() = delete;
		};
	};


}