#pragma once

#include <optional>

#include "json.h"

namespace json {

    class Builder {
    public:
        Builder() = default;

        class BaseContext;
        class KeyContext;
        class DictItemContext;
        class ArrayItemContext;

        Builder& Key(std::string key);
        Builder& Value(Node::Value value);
        Builder& StartDict();
        Builder& EndDict();
        Builder& StartArray();
        Builder& EndArray();

        Node Build();

        class BaseContext {
        public:
            BaseContext(Builder& builder) : builder_(builder) {}

            Builder::KeyContext Key(std::string key);
            Builder::DictItemContext StartDict();
            Builder::ArrayItemContext StartArray();
            Builder& EndDict();
            Builder& EndArray();

        protected:
            Builder& builder_;
        };

        class KeyContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            DictItemContext Value(Node::Value value);
            KeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
        };

        class DictItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            DictItemContext Value(Node::Value value) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            KeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
        };

    private:
        void AddNode(Node node);

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> cur_key_;
    };
} //namespace json