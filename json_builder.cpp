#include "json_builder.h"

namespace json {

	Builder::BaseContext::BaseContext(Builder& build) :builder(build) {

	}
	Builder::KeyContext Builder::BaseContext::Key(std::string key) {
		builder.Key(key);
		return builder;
	}
	Builder::BaseContext Builder::BaseContext::Value(Node value) {
		return builder.Value(value);

	}
	Builder::DictItemContext Builder::BaseContext::StartDict() {
		builder.StartDict();
		return builder;
	}
	Builder::ArrayItemContext Builder::BaseContext::StartArray() {
		builder.StartArray();
		return builder;
	}
	Builder::BaseContext Builder::BaseContext::EndDict() {
		builder.EndDict();
		return builder;
	}
	Builder::BaseContext Builder::BaseContext::EndArray() {
		builder.EndArray();
		return builder;
	}
	Node Builder::BaseContext::Build() {
		return builder.Build();
	}

	Builder::DictItemContext::DictItemContext(Builder& build) :BaseContext(build) {

	}
	Builder::KeyContext::KeyContext(Builder& build) :BaseContext(build) {

	}
	Builder::ValueDictContext Builder::KeyContext::Value(Node value) {
		builder.Value(value);
		return builder;
	}
	Builder::ArrayItemContext::ArrayItemContext(Builder& build) :BaseContext(build) {

	}
	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
		builder.Value(value);
		return builder;
	}			
	Builder::ValueDictContext::ValueDictContext(Builder& build) :BaseContext(build) {

	}
	const Builder::KeyContext Builder::Key(std::string key) {
		ready == true ? (throw  std::logic_error("Key")) : 0;
		if (root_->IsDict() && key_buffer.empty()) {
			key_buffer = key;
			root_->AsDict()[key];
			key_done = true;
		}
		else {
			throw  std::logic_error("Key");
		}
		return { *this };
	}
	Builder::BaseContext Builder::Value(Node value) {
		ready == true ? (throw  std::logic_error("Value")) : 0;
		if (root_ == nullptr) {
			root_ = new Node(value);
			ready = true;
		}
		else if (root_->IsDict()) {
			key_done ? 0 : (throw  std::logic_error("Value"));
			root_->AsDict()[key_buffer] = value;
			key_buffer = "";
			key_done = false;
			return ValueDictContext({ *this });
		}
		else if (root_->IsArray()) {
			root_->AsArray().emplace_back(value);
			return ArrayItemContext({ *this });
		}
		return { *this };
	}
	Builder::DictItemContext Builder::StartDict() {
		ready == true ? (throw  std::logic_error("StartDict")) : 0;
		if (root_ == nullptr) {
			root_ = nodes_stack_.emplace_back(new Node(Dict()));
		}
		else if (root_->IsDict()) {
			key_done ? 0 : (throw  std::logic_error("StartDict"));
			root_->AsDict()[key_buffer] = Node(Dict());
			root_ = nodes_stack_.emplace_back(&root_->AsDict().at(key_buffer));
			key_buffer = "";
			key_done = false;
		}
		else if (root_->IsArray()) {
			root_ = nodes_stack_.emplace_back(&root_->AsArray().emplace_back(Node(Dict())));
		}

		return { *this };
	}
	Builder::ArrayItemContext Builder::StartArray() {
		ready == true ? (throw  std::logic_error("StartArray")) : 0;
		if (root_ == nullptr) {
			root_ = nodes_stack_.emplace_back(new Node(Array()));
		}
		else if (root_->IsDict()) {
			key_done ? 0 : (throw  std::logic_error("StartArray"));
			root_->AsDict()[key_buffer] = Node(Array());
			root_ = nodes_stack_.emplace_back(&root_->AsDict().at(key_buffer));
			key_buffer = "";
			key_done = false;
		}
		else if (root_->IsArray()) {
			root_ = nodes_stack_.emplace_back(&root_->AsArray().emplace_back(Node(Array())));
		}
		return *this;
	}
	Builder::BaseContext Builder::EndDict() {
		(ready == true) || !nodes_stack_.back()->IsDict() ? (throw  std::logic_error("EndDict")) : 0;

		if (nodes_stack_.size() > 1) {
			root_ = *(nodes_stack_.end() - 2);
			nodes_stack_.erase(nodes_stack_.end() - 1, nodes_stack_.end());
		}
		else {
			root_ = *(nodes_stack_.begin());
			ready = true;
		}
		return *this;
	}
	Builder::BaseContext Builder::EndArray() {
		(ready == true) || !nodes_stack_.back()->IsArray() ? (throw  std::logic_error("EndArray")) : 0;

		if (nodes_stack_.size() > 1) {
			root_ = *(nodes_stack_.end() - 2);
			nodes_stack_.erase(nodes_stack_.end() - 1, nodes_stack_.end());
		}
		else {
			root_ = *(nodes_stack_.begin());
			ready = true;
		}
		return *this;
	}
	Node Builder::Build() {
		if (root_ == nullptr || nodes_stack_.size() > 1) {
			throw  std::logic_error("Build");

		}
		return *root_;
	}

}