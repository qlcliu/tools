#ifndef __REGEXP_H__
#define __REGEXP_H__

#include <string>
#include <vector>
#include <stack>

class RegExp {
public:
    RegExp(const wchar_t* reg);
    virtual ~RegExp();

    bool match(const wchar_t* s);

private:
    enum {
        Split = INT_MAX - 1,
        Match
    };

    enum {
        Null = 0,
        LeftBracket,
        RightBracket,
        Catenation,
        Alternation,
        ZeroOrOne,
        ZeroOrMore,
        OneOrMore,
        MatchAny
    };

    class State {
    public:
        State(int c, State* out1 = NULL, State* out2 = NULL)
            : _c(c), _out1(out1), _out2(out2) {

        }

        int _c;
        State* _out1;
        State* _out2;
    };

    union Ptrlist {
        Ptrlist* _next;
        State* _s;
    };

    class Frag {
    public:
        Frag(State* start, Ptrlist* out)
            : _start(start), _out(out) {

        }

        State* _start;
        Ptrlist* _out;
    };

    bool _prior_or_equal(wchar_t l, wchar_t r);
    bool _is_begin(wchar_t ch);
    bool _is_end(wchar_t ch);

    void _pre(std::vector<wchar_t>& reg);
    void _to_post(const std::vector<wchar_t>& reg, std::vector<wchar_t>& post);

    State* _state(int c, State* out1, State* out2);
    Ptrlist* _list_create(State** outp);
    void _patch(Ptrlist* l, State* s);
    Ptrlist* _append(Ptrlist* l, Ptrlist* r);
    void _post_to_nfa(std::vector<wchar_t>& post);

    void _add_state(std::vector<State*>& t, State* s);
    bool _step(std::vector<State*>& cur, wchar_t c, std::vector<State*>& next);
    bool _match(State* start, const wchar_t* s);

    std::wstring _reg;
    State _match_state;
    State* _nfa;
    std::vector<State*> _states;
};

RegExp::RegExp(const wchar_t* reg)
    : _reg(reg), _match_state(State(Match)), _nfa(NULL) {

}

RegExp::~RegExp() {
    for (unsigned i = 0; i < _states.size(); ++i) {
        delete _states[i];
    }
}

bool RegExp::_prior_or_equal(wchar_t l, wchar_t r) {
    return l == Catenation || l == Alternation && r == Alternation;
}

bool RegExp::_is_begin(wchar_t ch) {
    return ch != RightBracket && ch != Catenation && ch != Alternation && ch != ZeroOrOne && ch != ZeroOrMore && ch != OneOrMore;
}

bool RegExp::_is_end(wchar_t ch) {
    return ch != LeftBracket && ch != Catenation && ch != Alternation;
}

void RegExp::_pre(std::vector<wchar_t>& reg) {
    unsigned len = _reg.size();
    std::vector<wchar_t> replace;
    replace.reserve(len);

    for (unsigned i = 0; i < len; ++i) {
        switch (_reg[i]) {
        case L'\\':
            ++i < len ? replace.push_back(_reg[i]) : void(0);
            break;
        case L'|':
            replace.push_back(Alternation);
            break;
        case L'*':
            replace.push_back(ZeroOrMore);
            break;
        case L'+':
            replace.push_back(OneOrMore);
            break;
        case L'?':
            replace.push_back(ZeroOrOne);
            break;
        case L'(':
            replace.push_back(LeftBracket);
            break;
        case L')':
            replace.push_back(RightBracket);
            break;
        case L'.':
            replace.push_back(MatchAny);
            break;
        default:
            replace.push_back(_reg[i]);
            break;
        }
    }

    len = replace.size();
    reg.clear();
    reg.reserve(len * 2);

    for (unsigned i = 0; i < len; ++i) {
        reg.push_back(replace[i]);
        if (_is_end(replace[i]) && i + 1 < len && _is_begin(replace[i + 1])) {
            reg.push_back(Catenation);
        }
    }
}

void RegExp::_to_post(const std::vector<wchar_t>& reg, std::vector<wchar_t>& post) {
    post.clear();
    post.reserve(reg.size());

    bool escape = false;
    std::stack<wchar_t> s;
    for (unsigned i = 0; i < reg.size(); ++i) {
        switch (reg[i]) {
        case LeftBracket:
            s.push(reg[i]);
            break;
        case RightBracket:
            while (!s.empty() && s.top() != LeftBracket) {
                post.push_back(s.top());
                s.pop();
            }

            if (!s.empty()) {
                s.pop();
            }
            break;
        case Catenation:
        case Alternation:
            if (s.empty()) {
                s.push(reg[i]);
            } else {
                while (!s.empty() && _prior_or_equal(s.top(), reg[i])) {
                    post.push_back(s.top());
                    s.pop();
                }
                s.push(reg[i]);
            }
            break;
        default:
            post.push_back(reg[i]);
            break;
        }
    }

    while (!s.empty()) {
        post.push_back(s.top());
        s.pop();
    }
}

RegExp::State* RegExp::_state(int c, State* out1, State* out2) {
    State* s = new State(c, out1, out2);
    _states.push_back(s);
    return s;
}

RegExp::Ptrlist* RegExp::_list_create(State** outp) {
    Ptrlist* l;
    l = (Ptrlist*)outp;
    l->_next = NULL;

    return l;
}

void RegExp::_patch(Ptrlist* l, State* s) {
    Ptrlist* next;

    for (; l; l = next) {
        next = l->_next;
        l->_s = s;
    }
}

RegExp::Ptrlist* RegExp::_append(Ptrlist* l, Ptrlist* r) {
    Ptrlist* old = l;
    while (l->_next) {
        l = l->_next;
    }
    l->_next = r;

    return old;
}

void RegExp::_post_to_nfa(std::vector<wchar_t>& post) {
    std::stack<Frag> st;

    for (unsigned i = 0; i < post.size(); ++i) {
        switch (post[i]) {
            case Catenation: {
                Frag e2 = st.top();
                st.pop();
                Frag e1 = st.top();
                st.pop();
                _patch(e1._out, e2._start);
                st.push(Frag(e1._start, e2._out));
            }
            break;
            case Alternation: {
                Frag e2 = st.top();
                st.pop();
                Frag e1 = st.top();
                st.pop();
                State* s = _state(Split, e1._start, e2._start);
                st.push(Frag(s, _append(e1._out, e2._out)));
            }
            break;
            case ZeroOrOne: {
                Frag e = st.top();
                st.pop();
                State* s = _state(Split, e._start, NULL);
                st.push(Frag(s, _append(e._out, _list_create(&s->_out2))));
            }
            break;
            case ZeroOrMore: {
                Frag e = st.top();
                st.pop();
                State* s = _state(Split, e._start, NULL);
                _patch(e._out, s);
                st.push(Frag(s, _list_create(&s->_out2)));
            }
            break;
            case OneOrMore: {
                Frag e = st.top();
                st.pop();
                State* s = _state(Split, e._start, NULL);
                _patch(e._out, s);
                st.push(Frag(e._start, _list_create(&s->_out2)));
            }
            break;
            default: {
                State* s = _state(post[i], NULL, NULL);
                st.push(Frag(s, _list_create(&s->_out1)));
            }
            break;
        }
    }

    Frag e = st.top();
    st.pop();

    _patch(e._out, &_match_state);
    _nfa = e._start;
}

void RegExp::_add_state(std::vector<State*>& t, State* s) {
    if (!s) {
        return;
    }

    if (s->_c == Split) {
        _add_state(t, s->_out1);
        _add_state(t, s->_out2);
        return;
    } else {
        t.push_back(s);
    }
}

bool RegExp::_step(std::vector<State*>& cur, wchar_t c, std::vector<State*>& next) {
    if (cur.empty()) {
        return false;
    }

    for (unsigned i = 0; i < cur.size(); ++i) {
        if (cur[i]->_c == c || cur[i]->_c == MatchAny) {
            _add_state(next, cur[i]->_out1);
        }
    }

    return true;
}

bool RegExp::_match(State* start, const wchar_t* s) {
    std::vector<State*> cur;
    std::vector<State*> next;

    _add_state(cur, start);

    for (; *s; ++s) {
        if (!_step(cur, *s, next)) {
            return false;
        }
        cur.swap(next);
        next.clear();
    }

    for (unsigned i = 0; i < cur.size(); ++i) {
        if (cur[i] == &_match_state) {
            return true;
        }
    }

    return false;
}

bool RegExp::match(const wchar_t* s) {
    if (!_nfa) {
        std::vector<wchar_t> reg;
        _pre(reg);

        std::vector<wchar_t> post;
        _to_post(reg, post);

        _post_to_nfa(post);
    }

    return _match(_nfa, s);
}
#endif
