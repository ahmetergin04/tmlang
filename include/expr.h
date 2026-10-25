#pragma once

#include <memory>
#include <vector>

#include "token.h"
#include "visitor.h"

struct expr {
    virtual ~expr() = default;
    virtual void accept(Visitor& visitor) = 0;
};

struct Set_Literal : public expr {
    std::vector<token> items;
    Set_Literal(std::vector<token> items)
        : items(std::move(items)) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Set_Ref : public expr {
    token name;
    Set_Ref(token name) : name(name) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Set_Binary : public expr {
    std::unique_ptr<expr> left;
    token op;
    std::unique_ptr<expr> right;
    Set_Binary(std::unique_ptr<expr> left, token op, std::unique_ptr<expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct WildCard : public expr {
    token name;
    WildCard(token name) : name(name) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Logical : public expr {
    std::unique_ptr<expr> left;
    token op;
    std::unique_ptr<expr> right;
    Logical(std::unique_ptr<expr> left, token op, std::unique_ptr<expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Membership : public expr {
    token in;
    std::unique_ptr<expr> set;
    Membership(token in, std::unique_ptr<expr> set)
        : in(in), set(std::move(set)) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

struct Equality : public expr { //leftside is always "head"
    token op;
    token symbol;
    Equality(token op, token symbol)
        : op(op), symbol(symbol) {}
    void accept(Visitor& visitor) override { visitor.visit(*this); }
};


