#include "json.h"

using namespace std;

namespace json {

    Node LoadNode(istream& input);

    Node LoadArray(istream& input) {
        Array result;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        if (it == end) {
            throw ParsingError("Array parsing error");
        }

        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNumber(istream& input) {
        std::string parsed_num;

        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
            };

        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
            };

        if (input.peek() == '-') {
            read_char();
        }
        if (input.peek() == '0') {
            read_char();
        }
        else {
            read_digits();
        }

        bool is_int = true;
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                try {
                    return Node(std::stoi(parsed_num));
                }
                catch (...) {
                }
            }
            return Node(std::stod(parsed_num));
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Node LoadString(istream& input) {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                ++it;
                break;
            }
            else if (ch == '\\') {
                ++it;
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                s.push_back(ch);
            }
            ++it;
        }
        return Node(move(s));
    }

    Node LoadDict(istream& input) {
        Dict result;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        if (it == end) {
            throw ParsingError("Map parsing error");
        }

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.insert({ move(key), LoadNode(input) });
        }

        return Node(move(result));
    }

    std::string LoadLiteral(std::istream& input) {
        std::string s;
        while (std::isalpha(input.peek())) {
            s.push_back(static_cast<char>(input.get()));
        }
        return s;
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        }
        else if (c == '{') {
            return LoadDict(input);
        }
        else if (c == '"') {
            return LoadString(input);
        }
        else {
            input.putback(c);

            std::string literal = LoadLiteral(input);
            if (literal == "null") {
                return Node(nullptr);
            }
            else if (literal == "true") {
                return Node(true);
            }
            else if (literal == "false") {
                return Node(false);
            }
            else {
                return LoadNumber(input);
            }
        }
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Not an int");
        }
        return std::get<int>(*this);
    }

    double Node::AsDouble() const {
        if (IsDouble()) {
            if (std::holds_alternative<int>(*this)) {
                return static_cast<double>(std::get<int>(*this));
            }
            else {
                return std::get<double>(*this);
            }
        }
        else {
            throw std::logic_error("Not a double");
        }
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Not a bool");
        }
        return std::get<bool>(*this);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Not a string");
        }
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Not an array");
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw std::logic_error("Not a map");
        }
        return std::get<Dict>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return (std::holds_alternative<int>(*this)) || (std::holds_alternative<double>(*this));
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintValue(const double value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(const int value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const std::string& str, const PrintContext& ctx) {
        ctx.out << '"';
        for (const auto ch : str) {
            switch (ch) {
            case '\n':
                ctx.out << "\\n";
                break;
            case '\r':
                ctx.out << "\\r";
                break;
            case '"':
                ctx.out << "\\\"";
                break;
            case '\t':
                ctx.out << "\\t";
                break;
            case '\\':
                ctx.out << "\\\\";
                break;
            default:
                ctx.out << ch;
                break;
            }
        }
        ctx.out << '"';
    }

    void PrintValue(bool value, const PrintContext& ctx) {
        if (value) ctx.out << "true";
        else ctx.out << "false";
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    void PrintValue(const Array& array, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : array) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : dict) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintValue(key, ctx);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }


    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }
}  // namespace json