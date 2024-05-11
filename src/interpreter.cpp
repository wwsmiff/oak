#include "interpreter.hpp"
#include <cmath>
#include <stdexcept>
#include <string_view>

Interpreter::Interpreter() : m_source{}, m_pos{}, m_current_token{} {}

Interpreter::Interpreter(const std::string &source)
    : m_source{source}, m_pos{}, m_current_token{} {}

void Interpreter::source(const std::string &source) {
  m_source = source;
  m_pos = 0;
  m_current_token = advance();
  m_start_token = m_current_token;
}

void Interpreter::error(const std::string &message) const {
  throw std::runtime_error(message);
}

Token Interpreter::advance() {
  if (m_pos > m_source.size() - 1) {
    return Token{.type = TokenType::END_OF_FILE, .value = std::nullopt};
  }

  char current_char = m_source.at(m_pos);

  if (std::isspace(current_char)) {
    while (std::isspace(current_char) && m_pos < m_source.size() - 1) {
      current_char = m_source.at(++m_pos);
    }
  }

  if (std::isalpha(current_char)) {
    auto value = parse_id();
    if (value == "print") {
      return Token{.type = TokenType::PRINT, .value = std::nullopt};
    }
    return Token{.type = TokenType::ID, .value = value};
  }

  if (std::isdigit(current_char)) {
    auto value = parse_digit();
    return Token{.type = TokenType::INTEGER, .value = value};
  }
  if (current_char == '+') {
    m_pos++;
    return Token{.type = TokenType::PLUS, .value = std::nullopt};
  }
  if (current_char == '-') {
    m_pos++;
    if (m_source.at(m_pos) == '>') {
      m_pos++;
      return Token{.type = TokenType::REF, .value = std::nullopt};
    }
    return Token{.type = TokenType::MINUS, .value = std::nullopt};
  }
  if (current_char == '*') {
    m_pos++;
    if (m_source.at(m_pos) == '*') {
      m_pos++;
      return Token{.type = TokenType::POWER_OF, .value = std::nullopt};
    }
    return Token{.type = TokenType::STAR, .value = std::nullopt};
  }
  if (current_char == '/') {
    m_pos++;
    return Token{.type = TokenType::SLASH, .value = std::nullopt};
  }
  if (current_char == '(') {
    m_pos++;
    return Token{.type = TokenType::LPAREN, .value = std::nullopt};
  }
  if (current_char == ')') {
    m_pos++;
    return Token{.type = TokenType::RPAREN, .value = std::nullopt};
  }

  if (current_char == '=') {
    m_pos++;
    if (m_source.at(m_pos) == '=') {
      m_pos++;
      return Token{.type = TokenType::IS_EQ, .value = std::nullopt};
    }
    return Token{.type = TokenType::ASSIGN, .value = std::nullopt};
  }

  error(std::format("Unexpected character: {}", current_char));
  return {};
}

/*
 * Advance if next token's type is the expected type (the one that is passed
 * into the function). Otherwise raise an error.
 */
void Interpreter::eat(const TokenType &expected_type) {
  if (m_current_token.type == expected_type) {
    m_current_token = advance();
    return;
  } else {
    if (m_current_token.type) {
      error(std::format(
          "Expected {} but got {}",
          type_string_table_v.at(static_cast<size_t>(expected_type)),
          type_string_table_v.at(
              static_cast<size_t>(m_current_token.type.value()))));
    } else {
      error(std::format(
          "Expected {} but got {}",
          type_string_table_v.at(static_cast<size_t>(expected_type)), "NONE"));
    }
  }
}

int64_t Interpreter::factor() {
  Token token = m_current_token;
  int64_t res{};
  if (token.type == TokenType::INTEGER) {
    res = std::stoll(token.value.value());
    eat(TokenType::INTEGER);
  } else if (token.type == TokenType::ID) {
    std::string id{token.value.value()};
    if (m_variables.contains(id)) {
      res = m_variables.at(id);
      eat(TokenType::ID);
    } else {
      error(std::format("Variable '{}' does not exist.", id));
    }
  } else if (token.type == TokenType::LPAREN) {
    eat(TokenType::LPAREN);
    res = expr();
    eat(TokenType::RPAREN);
  }
  return res;
}

int64_t Interpreter::term() {
  int64_t res = factor();
  while (m_current_token.type == TokenType::STAR ||
         m_current_token.type == TokenType::SLASH ||
         m_current_token.type == TokenType::POWER_OF) {
    Token token = m_current_token;
    switch (token.type.value()) {
    case TokenType::STAR:
      eat(TokenType::STAR);
      break;
    case TokenType::SLASH:
      eat(TokenType::SLASH);
      break;
    case TokenType::POWER_OF:
      eat(TokenType::POWER_OF);
      break;
    default:
      break;
    }

    int64_t right = factor();

    switch (token.type.value()) {
    case TokenType::STAR:
      res *= right;
      break;
    case TokenType::SLASH:
      res /= right;
      break;
    case TokenType::POWER_OF:
      res = std::pow(res, right);
      break;
    default:
      break;
    }
  }

  return res;
}

int64_t Interpreter::expr() {
  int64_t res = term();
  while (m_current_token.type == TokenType::PLUS ||
         m_current_token.type == TokenType::MINUS) {
    Token op = m_current_token;
    switch (op.type.value()) {
    case TokenType::PLUS:
      eat(TokenType::PLUS);
      break;
    case TokenType::MINUS:
      eat(TokenType::MINUS);
      break;
    default:
      break;
    }
    int64_t right = term();
    switch (op.type.value()) {
    case TokenType::PLUS:
      res += right;
      break;
    case TokenType::MINUS:
      res -= right;
      break;
    default:
      error("Unexpected operator.");
    }
  }

  return res;
}

void Interpreter::run() {
  if (m_current_token.type.has_value()) {
    switch (m_current_token.type.value()) {
    case TokenType::INTEGER:
    case TokenType::PLUS:
    case TokenType::LPAREN:
    case TokenType::MINUS: {
      if (m_start_token.type.has_value()) {
        if (m_start_token.type.value() != TokenType::PRINT) {
          error("Expected primary expression such as 'print' or '='.");
          break;
        } else {
          int64_t res = expr();
          std::cout << res << std::endl;
        }
      }
      break;
    }
    case TokenType::PRINT:
      handle_print();
      break;
    case TokenType::ID:
      handle_variable();
      break;
    default:
      error("Unexpected token.");
      break;
    }
  }
}

std::string Interpreter::parse_digit() {
  std::string value{};
  while (std::isdigit(m_source.at(m_pos))) {
    value.push_back(m_source.at(m_pos));
    if (m_pos < m_source.size() - 1)
      m_pos++;
    else
      break;
  }

  return value;
}

std::string Interpreter::parse_id() {
  std::string id{};

  while (std::isalpha(m_source.at(m_pos))) {
    id.push_back(m_source.at(m_pos));
    if (m_pos < m_source.size() - 1)
      m_pos++;
    else
      break;
  }

  return id;
}

void Interpreter::handle_print() {
  m_current_token = advance();
  switch (m_current_token.type.value()) {
  case TokenType::INTEGER:
  case TokenType::PLUS:
  case TokenType::LPAREN:
  case TokenType::MINUS: {
    int64_t res = expr();
    std::cout << res << std::endl;
    break;
  }
  case TokenType::ID: {
    const std::string &id{m_current_token.value.value()};
    if (m_variables.contains(id)) {
      std::cout << m_variables.at(id) << std::endl;
    } else {
      error(std::format("Variable '{}' does not exist.", id));
    }
  }
  default:
    break;
  }
}

void Interpreter::handle_variable() {
  std::string id{m_current_token.value.value()};
  eat(TokenType::ID);
  if (m_current_token.type.value() == TokenType::ASSIGN) {
    eat(TokenType::ASSIGN);
    switch (m_current_token.type.value()) {
    case TokenType::INTEGER:
    case TokenType::PLUS:
    case TokenType::LPAREN:
    case TokenType::MINUS:
    case TokenType::ID: {
      int64_t res = expr();
      m_variables.insert_or_assign(id, res);
    } break;
    default:
      break;
    }
  } else {
    error("Expected primary expression.");
  }
}
