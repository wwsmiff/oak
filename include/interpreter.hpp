#pragma once

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <optional>
#include <unordered_map>

using std::uint64_t;

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
  IS_EQ,
  REF,
  ID,
  PRINT,
  END_OF_FILE,
  N_TYPES,
};

constexpr std::array<std::string_view, static_cast<size_t>(TokenType::N_TYPES)>
    type_string_table_v{{"INTEGER", "PLUS", "MINUS", "STAR", "SLASH",
                         "POWER_OF", "LPAREN", "RPAREN", "ASSIGN", "IS_EQ",
                         "REF", "ID", "PRINT", "END_OF_FILE"}};

constexpr std::array<std::string_view, 1> reserved_keywords_v{{"print"}};

struct Token {
  std::optional<TokenType> type{};
  std::optional<std::string> value{};

  std::string str() const {
    if (type && value) {
      return std::format(
          "Token(.type = {}, .value = {})",
          type_string_table_v.at(static_cast<size_t>(type.value())),
          value.value());
    } else if (type && !value) {
      return std::format(
          "Token(.type = {}, .value = {})",
          type_string_table_v.at(static_cast<size_t>(type.value())), "NONE");
    }
    return std::format("Token(.type = {}, .value = {})", "NONE", "NONE");
  }
};

class Interpreter {
public:
  Interpreter();
  Interpreter(const std::string &source);

  static Interpreter create();

  void source(const std::string &source);
  void run();

private:
  std::string m_source{};
  uint64_t m_pos{};
  Token m_current_token{};
  Token m_start_token{};
  std::unordered_map<std::string, int64_t> m_variables{};

  void error(const std::string &message) const;
  Token advance();
  void eat(const TokenType &type);

  int64_t factor();
  int64_t term();
  int64_t expr();

  std::string parse_digit();
  std::string parse_id();
  void handle_print();
  void handle_variable();
};
