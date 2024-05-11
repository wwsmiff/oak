#include "interpreter.hpp"
#include <cmath>
#include <stdexcept>
#include <string_view>

#define CHECK_AND_PERFORM(op)                                                  \
  do {                                                                         \
    if (res.type == LiteralType::NIL || right.type == LiteralType::NIL) {      \
      res.type = LiteralType::NIL;                                             \
      right.type = LiteralType::NIL;                                           \
    } else {                                                                   \
      if (res.type == LiteralType::INTEGER &&                                  \
          right.type == LiteralType::INTEGER) {                                \
        res.as.i64 op right.as.i64;                                            \
      } else if (res.type == LiteralType::INTEGER &&                           \
                 right.type == LiteralType::FLOAT) {                           \
        res.type = LiteralType::FLOAT;                                         \
        res.as.f64 = res.as.i64;                                               \
        res.as.f64 op right.as.f64;                                            \
      } else if (res.type == LiteralType::FLOAT &&                             \
                 right.type == LiteralType::INTEGER) {                         \
        right.type = LiteralType::FLOAT;                                       \
        right.as.f64 = right.as.i64;                                           \
        res.as.f64 op right.as.f64;                                            \
      } else if (res.type == LiteralType::FLOAT &&                             \
                 right.type == LiteralType::FLOAT) {                           \
        res.as.f64 op right.as.f64;                                            \
      }                                                                        \
    }                                                                          \
  } while (0)

namespace {
std::ostream &operator<<(std::ostream &os, const Literal &literal) {
  if (literal.type == LiteralType::NIL) {
    os << "nil";
  } else if (literal.type == LiteralType::INTEGER) {
    os << literal.as.i64;
  } else if (literal.type == LiteralType::FLOAT) {
    os << literal.as.f64;
  }
  return os;
}
}; // namespace

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
    return Token{.type = TokenType::END_OF_FILE,
                 .value = std::nullopt,
                 .id = std::nullopt};
  }

  char current_char = m_source.at(m_pos);

  if (std::isspace(current_char)) {
    while (std::isspace(current_char) && m_pos < m_source.size() - 1) {
      current_char = m_source.at(++m_pos);
    }
  }

  if (std::isalpha(current_char)) {
    auto id = parse_id();
    if (id == "print") {
      return Token{.type = TokenType::PRINT, .value = std::nullopt, .id = id};
    } else if (id == "nil") {
      return Token{.type = TokenType::NIL, .value = std::nullopt, .id = id};
    }
    return Token{.type = TokenType::ID, .value = std::nullopt, .id = id};
  }

  if (std::isdigit(current_char)) {
    auto value = parse_digit();
    return Token{
        .type = TokenType::INTEGER, .value = value, .id = std::nullopt};
  }

  if (current_char == '+') {
    m_pos++;
    return Token{
        .type = TokenType::PLUS, .value = std::nullopt, .id = std::nullopt};
  }
  if (current_char == '-') {
    m_pos++;
    if (m_source.at(m_pos) == '>') {
      m_pos++;
      return Token{.type = TokenType::ASSIGN_REF,
                   .value = std::nullopt,
                   .id = std::nullopt};
    }
    return Token{
        .type = TokenType::MINUS, .value = std::nullopt, .id = std::nullopt};
  }
  if (current_char == '*') {
    m_pos++;
    if (m_source.at(m_pos) == '*') {
      m_pos++;
      return Token{.type = TokenType::POWER_OF,
                   .value = std::nullopt,
                   .id = std::nullopt};
    }
    return Token{
        .type = TokenType::STAR, .value = std::nullopt, .id = std::nullopt};
  }
  if (current_char == '/') {
    m_pos++;
    return Token{
        .type = TokenType::SLASH, .value = std::nullopt, .id = std::nullopt};
  }
  if (current_char == '(') {
    m_pos++;
    return Token{
        .type = TokenType::LPAREN, .value = std::nullopt, .id = std::nullopt};
  }
  if (current_char == ')') {
    m_pos++;
    return Token{
        .type = TokenType::RPAREN, .value = std::nullopt, .id = std::nullopt};
  }

  if (current_char == '=') {
    m_pos++;
    if (m_source.at(m_pos) == '=') {
      m_pos++;
      return Token{
          .type = TokenType::IS_EQ, .value = std::nullopt, .id = std::nullopt};
    }
    return Token{
        .type = TokenType::ASSIGN, .value = std::nullopt, .id = std::nullopt};
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

Literal Interpreter::factor() {
  Token token = m_current_token;
  Literal res{.type = LiteralType::NIL};

  if (token.type == TokenType::INTEGER) {
    if (token.value.value().type == LiteralType::INTEGER ||
        token.value.value().type == LiteralType::FLOAT) {
      res = token.value.value();
    }
    eat(TokenType::INTEGER);
  } else if (token.type == TokenType::ID) {
    std::string id{token.id.value()};
    if (m_variables.contains(id)) {
      const Literal &literal = m_variables.at(id);
      if (literal.is_ref) {
        if (literal.type == LiteralType::INTEGER) {
          res.as.i64 = *(static_cast<int64_t *>(literal.as.ref));
          res.type = LiteralType::INTEGER;
        } else if (literal.type == LiteralType::FLOAT) {
          res.as.f64 = *(static_cast<float *>(literal.as.ref));
          res.type = LiteralType::FLOAT;
        } else if (literal.type == LiteralType::BOOLEAN) {
          res.as.f64 = *(static_cast<bool *>(literal.as.ref));
          res.type = LiteralType::BOOLEAN;
        }
      } else {
        res = literal;
      }
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

Literal Interpreter::term() {
  Literal res = factor();
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

    Literal right = factor();

    switch (token.type.value()) {
    case TokenType::STAR:
      CHECK_AND_PERFORM(*=);
      break;
    case TokenType::SLASH:
      CHECK_AND_PERFORM(/=);
      break;
    case TokenType::POWER_OF:
      if (res.type == LiteralType::NIL || right.type == LiteralType::NIL) {
        res.type = LiteralType::NIL;
        right.type = LiteralType::NIL;
      } else {
        if (res.type == LiteralType::INTEGER &&
            right.type == LiteralType::INTEGER) {
          res.as.i64 = std::pow(res.as.i64, right.as.i64);
        } else if (res.type == LiteralType::INTEGER &&
                   right.type == LiteralType::FLOAT) {
          res.type = LiteralType::FLOAT;
          res.as.f64 = res.as.i64;
          res.as.f64 = std::pow(res.as.f64, right.as.f64);
        } else if (res.type == LiteralType::FLOAT &&
                   right.type == LiteralType::INTEGER) {
          right.type = LiteralType::FLOAT;
          right.as.f64 = right.as.i64;
          res.as.f64 = std::pow(res.as.f64, right.as.f64);
        } else if (res.type == LiteralType::FLOAT &&
                   right.type == LiteralType::FLOAT) {
          res.as.i64 = std::pow(res.as.f64, right.as.f64);
        }
      }
      break;
    default:
      break;
    }
  }

  return res;
}

Literal Interpreter::expr() {
  Literal res = term();
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
    Literal right = term();
    switch (op.type.value()) {
    case TokenType::PLUS:
      CHECK_AND_PERFORM(+=);
      break;
    case TokenType::MINUS:
      CHECK_AND_PERFORM(-=);
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

Literal Interpreter::parse_digit() {
  Literal literal{};
  std::string value{};
  bool found_dot{};
  while (std::isdigit(m_source.at(m_pos)) || m_source.at(m_pos) == '.') {
    if (m_source.at(m_pos) == '.') {
      found_dot = true;
    }
    value.push_back(m_source.at(m_pos));
    if (m_pos < m_source.size() - 1) {
      m_pos++;
    } else {
      break;
    }
  }

  if (found_dot) {
    literal.type = LiteralType::FLOAT;
    literal.as.f64 = std::stod(value);
  } else {
    literal.type = LiteralType::INTEGER;
    literal.as.i64 = std::stoll(value);
  }

  return literal;
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
  case TokenType::MINUS:
  case TokenType::NIL:
  case TokenType::ID: {
    Literal res = expr();
    std::cout << res << std::endl;
    break;
  }

  default:
    break;
  }
}

void Interpreter::handle_variable() {
  std::string id{m_current_token.id.value()};
  eat(TokenType::ID);
  if (m_current_token.type.value() == TokenType::ASSIGN) {
    eat(TokenType::ASSIGN);
    switch (m_current_token.type.value()) {
    case TokenType::INTEGER:
    case TokenType::PLUS:
    case TokenType::LPAREN:
    case TokenType::MINUS:
    case TokenType::ID: {
      Literal res = expr();
      m_variables.insert_or_assign(id, res);
    } break;
    case TokenType::NIL: {
      Literal literal = {.type = LiteralType::NIL};
      m_variables.insert_or_assign(id, literal);
    } break;
    default:
      break;
    }
  }

  else if (m_current_token.type == TokenType::ASSIGN_REF) {
    eat(TokenType::ASSIGN_REF);
    std::string src_id{m_current_token.id.value()};
    eat(TokenType::ID);
    if (m_variables.contains(src_id)) {
      Literal literal = {.type = m_variables.at(src_id).type, .is_ref = true};
      if (m_variables.at(src_id).type == LiteralType::NIL) {
        literal.as.ref = nullptr;
      } else if (m_variables.at(src_id).type == LiteralType::INTEGER) {
        literal.as.ref = static_cast<void *>(&m_variables.at(src_id).as.i64);
      } else if (m_variables.at(src_id).type == LiteralType::FLOAT) {
        literal.as.ref = static_cast<void *>(&m_variables.at(src_id).as.f64);
      }
      m_variables.insert_or_assign(id, literal);
    }
  }

  else {
    error("Expected primary expression.");
  }
}

#undef CHECK_AND_PERFORM