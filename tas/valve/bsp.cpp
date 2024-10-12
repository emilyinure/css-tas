#include "bsp.h"

#include <tao/pegtl.hpp>

#include <stack>

// All credits to: https://github.com/cursey/vdf-parser
namespace kv {
  using namespace tao::pegtl;

  struct State {
    std::string            item{};
    std::string            key{};
    std::string            value{};
    std::stack<KeyValues>  objs{};
    std::vector<KeyValues> root{};
  };

  // Comments are "//" and read until the end of the line.
  struct Comment : disable<two<'/'>, until<eolf>> {};

  // Whitespace characters.
  struct Whitespace : one<' ', '\r', '\n', '\t'> {};

  // Separators (Skip white space and comments).
  struct Sep : sor<Whitespace, Comment> {};
  struct Seps : star<Sep> {};

  // Characters.
  struct EscapedChar : if_must<one<'\\'>, sor<one<'"', '\\', 'n', 't'>>> {};
  struct UnescapedChar : not_one<'\r', '\n'> {};
  struct Char : sor<EscapedChar, UnescapedChar> {};

  // String.
  struct String : if_must<one<'"'>, until<one<'"'>, Char>> {};

  // Key & Values.
  struct Key : String {};
  struct Value : String {};
  struct KeyValue : seq<Key, Seps, Value> {};

  // KeyValue stack.
  struct ObjectName : String {};
  struct ObjectMembers : sor<list<sor<KeyValue, struct Object>, Seps>, Seps> {};
  struct Object : seq<opt<ObjectName>, Seps, one<'{'>, until<one<'}'>, ObjectMembers>> {};

  // Final grammar.
  struct Grammar : until<eof, sor<eolf, Sep, Object>> {};

  template <class Rule> struct Action : nothing<Rule> {};

  template <> struct Action<Char> {
    template <class Input> static void apply(const Input& in, State& s) {
      s.item += in.string_view();
    }
  };

  template <> struct Action<Key> {
    template <class Input> static void apply(const Input& in, State& s) {
      s.key = std::move(s.item);
    }
  };

  template <> struct Action<Value> {
    template <class Input> static void apply(const Input& in, State& s) {
      s.value = std::move(s.item);
    }
  };

  template <> struct Action<KeyValue> {
    template <class Input> static void apply(const Input& in, State& s) {
      KeyValues obj{.name = std::move(s.key), .value = std::move(s.value)};

      if (s.objs.empty()) {
        // The object has no name (happens w/ root objects sometimes). Just add an empty parent
        // to add this kv to.
        s.objs.emplace();
      }

      s.objs.top()[obj.name] = std::move(obj);
    }
  };

  template <> struct Action<ObjectName> {
    template <class Input> static void apply(const Input& in, State& s) {
      s.objs.push({.name = std::move(s.item)});
    }
  };

  template <> struct Action<Object> {
    template <class Input> static void apply(const Input& in, State& s) {
      auto obj = std::move(s.objs.top());
      s.objs.pop();

      if (s.objs.empty()) {
        s.root.emplace_back(std::move(obj));
      } else {
        s.objs.top()[obj.name] = std::move(obj);
      }
    }
  };

  std::expected<std::vector<KeyValues>, std::string> parse(std::string_view str) {
    memory_input in{str, ""};
    State        state{};

    try {
      tao::pegtl::parse<Grammar, Action>(in, state);
      return state.root;
    } catch (const std::exception& e) {
      return std::unexpected{e.what()};
    }
  }
} // namespace kv