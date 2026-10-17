#include <httplib.h>
#include <nlohmann/json.hpp>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using json = nlohmann::json;
enum class TokenType { Number, Plus, Minus, Multiply, Divide, LeftParen, RightParen, End };
struct Token { TokenType type; std::string value; std::size_t position; };
static std::string tokenName(TokenType t) {
 switch(t) { case TokenType::Number:return "NUMBER"; case TokenType::Plus:return "PLUS"; case TokenType::Minus:return "MINUS"; case TokenType::Multiply:return "MULTIPLY"; case TokenType::Divide:return "DIVIDE"; case TokenType::LeftParen:return "LEFT_PAREN"; case TokenType::RightParen:return "RIGHT_PAREN"; case TokenType::End:return "END"; } return "UNKNOWN";
}
class ExpressionCompiler {
 std::string source_; std::size_t pos_=0; std::vector<Token> tokens_; std::size_t current_=0;
 static std::string formatNumber(double v) { if(std::fabs(v)<1e-12)v=0; std::string s=std::to_string(v); while(!s.empty()&&s.back()=='0')s.pop_back(); if(!s.empty()&&s.back()=='.')s.pop_back(); return s.empty()?"0":s; }
 void tokenize() {
  while(pos_<source_.size()) {
   unsigned char ch=static_cast<unsigned char>(source_[pos_]);
   if(std::isspace(ch)){++pos_;continue;}
   if(std::isdigit(ch)||source_[pos_]=='.') {
    std::size_t start=pos_; int dots=0;
    while(pos_<source_.size()) { char c=source_[pos_]; if(std::isdigit(static_cast<unsigned char>(c)))++pos_; else if(c=='.'){if(++dots>1)throw std::runtime_error("Invalid number near position "+std::to_string(start+1)+".");++pos_;} else break; }
    std::string n=source_.substr(start,pos_-start); if(n==".")throw std::runtime_error("A decimal point must be part of a number."); tokens_.push_back({TokenType::Number,n,start}); continue;
   }
   TokenType t; switch(source_[pos_]) { case '+':t=TokenType::Plus;break;case '-':t=TokenType::Minus;break;case '*':t=TokenType::Multiply;break;case '/':t=TokenType::Divide;break;case '(':t=TokenType::LeftParen;break;case ')':t=TokenType::RightParen;break;default:throw std::runtime_error("Invalid character '"+std::string(1,source_[pos_])+"' at position "+std::to_string(pos_+1)+"."); }
   tokens_.push_back({t,std::string(1,source_[pos_]),pos_}); ++pos_;
  }
  tokens_.push_back({TokenType::End,"END",source_.size()});
 }
 const Token& peek() const { return tokens_.at(current_); }
 bool match(TokenType t){if(peek().type==t){++current_;return true;}return false;}
 double factor(){ if(match(TokenType::Plus))return factor(); if(match(TokenType::Minus))return -factor(); if(match(TokenType::Number)){try{return std::stod(tokens_[current_-1].value);}catch(...){throw std::runtime_error("Invalid numeric literal.");}} if(match(TokenType::LeftParen)){double v=expression();if(!match(TokenType::RightParen))throw std::runtime_error("Missing closing parenthesis ')'.");return v;} if(peek().type==TokenType::End)throw std::runtime_error("Expression ends unexpectedly. Enter a number or use parentheses correctly."); throw std::runtime_error("Expected a number or '(' before '"+peek().value+"'."); }
 double term(){double v=factor();while(true){if(match(TokenType::Multiply))v*=factor();else if(match(TokenType::Divide)){double d=factor();if(d==0.0)throw std::runtime_error("Division by zero is not allowed.");v/=d;}else break;if(!std::isfinite(v))throw std::runtime_error("The result is outside the supported numeric range.");}return v;}
 double expression(){double v=term();while(true){if(match(TokenType::Plus))v+=term();else if(match(TokenType::Minus))v-=term();else break;if(!std::isfinite(v))throw std::runtime_error("The result is outside the supported numeric range.");}return v;}
public:
 explicit ExpressionCompiler(std::string s):source_(std::move(s)){}
 json run(){tokenize();json ts=json::array();for(const auto&t:tokens_){if(t.type==TokenType::End)break;ts.push_back({{"type",tokenName(t.type)},{"value",t.value},{"position",t.position}});}current_=0;double result=expression();if(peek().type!=TokenType::End)throw std::runtime_error("Unexpected token '"+peek().value+"'.");if(!std::isfinite(result))throw std::runtime_error("The result is outside the supported numeric range.");return {{"ok",true},{"expression",source_},{"tokens",ts},{"result",result},{"formattedResult",formatNumber(result)}};}
};
int main(){httplib::Server server;server.set_payload_max_length(16*1024);server.set_mount_point("/","./public");server.Get("/api/health",[](const httplib::Request&,httplib::Response&r){r.set_content(R"({"ok":true,"service":"mini-compiler"})","application/json");});server.Post("/api/evaluate",[](const httplib::Request&req,httplib::Response&res){try{auto body=json::parse(req.body);if(!body.contains("expression")||!body["expression"].is_string()){res.status=400;res.set_content(json({{"ok",false},{"error","Please provide an expression."}}).dump(),"application/json");return;}std::string expr=body["expression"].get<std::string>();if(expr.empty()||expr.size()>1000){res.status=400;res.set_content(json({{"ok",false},{"error","Expression must contain between 1 and 1000 characters."}}).dump(),"application/json");return;}ExpressionCompiler c(expr);res.set_content(c.run().dump(),"application/json");}catch(const std::exception&e){res.status=400;res.set_content(json({{"ok",false},{"error",e.what()}}).dump(),"application/json");}});const char* env=std::getenv("PORT");int port=env?std::atoi(env):8080;if(port<=0||port>65535)port=8080;std::cout<<"Mini Compiler listening on 0.0.0.0:"<<port<<std::endl;if(!server.listen("0.0.0.0",port)){std::cerr<<"Failed to start HTTP server."<<std::endl;return 1;}return 0;}
