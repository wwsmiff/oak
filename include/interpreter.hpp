#pragma once

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <optional>
#include <unordered_map>

using std::uint64_t;
using std::uint8_t;

enum class TokenType {
  INTEGER,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  POWER_OF,
  LPAREN,
  RPAREN,
  ASSIGN,
  ASSIGN_REF,
  IS_EQ,
  ID,
  PRINT,
  NIL,
  DECL,
  END_OF_FILE,
  N_TYPES,
};

enum class LiteralType : uint8_t { NIL, INTEGER, FLOAT, BOOLEAN, N_TYPES };

constexpr std::array<std::string_view, static_cast<size_t>(TokenType::N_TYPES)>
    type_string_table_v{{"INTEGER", "PLUS", "MINUS", "STAR", "SLASH",
                         "POWER_OF", "LPAREN", "RPAREN", "ASSIGN", "ASSIGN_REF",
                         "IS_EQ", "ID", "PRINT", "NIL",
                         "DECL"
                         "END_OF_FILE"}};

constexpr std::array<std::string_view, 3> reserved_keywords_v{
    {"print", "nil", "decl"}};

struct As {
  union {
    int64_t i64;
    double f64;
    void *ref;
  };
};

struct Literal {
  LiteralType type{};
  bool is_ref{};
  LiteralType *ref_type{};
  As as;
};

struct Token {
  std::optional<TokenType> type{};
  std::optional<Literal> value{};
  std::optional<std::string> id{};
};

class Interpreter {
public:
  Interpreter();
  Interpreter(const std::string &source);
  void source(const std::string &source);
  void run();

private:
  std::string m_source{};
  uint64_t m_pos{};
  Token m_current_token{};
  Token m_start_token{};
  std::unordered_map<std::string, Literal> m_variables{};
  std::unordered_map<std::string, Literal> m_config{};

  void error(const std::string &message) const;
  Token advance();
  void eat(const TokenType &type);
  Literal factor();
  Literal term();
  Literal expr();
  Literal parse_digit();
  std::string parse_id();
  void handle_print();
  void handle_variable();
  void handle_decl();
};
