#include "json_builder.h"

namespace json {
    void Builder::AddNode(Node node) {
        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error("Root already exists");
            }
            root_ = std::move(node);
            if (root_.IsDict() || root_.IsArray()) {
                nodes_stack_.push_back(&root_);
            }
        }
        else {
            Node* parent = nodes_stack_.back();

            if (parent->IsDict()) {
                if (!cur_key_) throw std::logic_error("Need Key()");

                auto& dict = const_cast<Dict&>(std::get<Dict>(parent->GetValue()));
                auto [it, inserted] = dict.emplace(std::move(*cur_key_), std::move(node));
                cur_key_.reset();

                if (it->second.IsDict() || it->second.IsArray()) nodes_stack_.push_back(&it->second);
            }
            else if (parent->IsArray()) {
                auto& arr = const_cast<Array&>(std::get<Array>(parent->GetValue()));
                arr.push_back(std::move(node));

                if (arr.back().IsDict() || arr.back().IsArray()) nodes_stack_.push_back(&arr.back());
            }
        }
    }

    Builder& Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || cur_key_.has_value()) {
            throw std::logic_error("Key() called outside of Dictionary");
        }
        cur_key_ = std::move(key);
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        AddNode(Node(std::move(value)));
        return *this;
    }

    Builder& Builder::StartDict() {
        AddNode(Dict());
        return *this;
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("End Dictionary called outside of Dictionary");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::StartArray() {
        AddNode(Array());
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("End Array called outside of Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    json::Node Builder::Build() {
        if (root_.IsNull() || !nodes_stack_.empty()) {
            throw std::logic_error("JSON is not finished");
        }
        return root_;
    }

    // ----------------- BaseContext -----------------
    Builder::KeyContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }
    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }
    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }
    Builder& Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }

    // ----------------- KeyContext -----------------

    Builder::DictItemContext Builder::KeyContext::Value(Node::Value value) {
        this->builder_.Value(std::move(value));
        return Builder::DictItemContext(this->builder_);
    }
} //namespace json