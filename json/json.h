#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final: private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        Node(Value value) : variant(std::move(value)) {}

        inline const variant& GetValue() const { return *this; }

        inline Node operator[](const std::string& key) const {
            if (IsDict()) {
                variant map = GetValue();
                auto it = std::get<Dict>(map).find(key);
                if (it != std::get<Dict>(map).end()) {
                    return it->second;
                }
            }
            throw ParsingError("Key not found");
        }

        inline Node operator[](size_t index) const {
            if (IsArray()) {
                auto& array = std::get<Array>(GetValue());
                if (index < array.size()) {
                    return array[index];
                }
            }
            throw ParsingError("Index out of bounds");
        }

        const Array& AsArray() const;
        const Dict& AsDict() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsDict() const;

    private:
        variant value_;
    };

    inline bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() != rhs.GetValue();
    }

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    void Print(const Document& doc, std::ostream& output);
}  // namespace json