OPDEF(push, 1, 1)
OPDEF(pop, 2, 1)
OPDEF(dup, 3, 0)
OPDEF(add, 10, 0)
OPDEF(sub, 11, 0)
OPDEF(mul, 12, 0)
OPDEF(div, 13, 0)
OPDEF(sin, 20, 0)
OPDEF(cos, 21, 0)
OPDEF(sqrt, 22, 0)
OPDEF(in, 50, 0)
OPDEF(out, 51, 0)
OPDEF(jmp, 101, VALUE_LABEL)
OPDEF(ja, 102, VALUE_LABEL)
OPDEF(jae, 103, VALUE_LABEL)
OPDEF(jb, 104, VALUE_LABEL)
OPDEF(jbe, 105, VALUE_LABEL)
OPDEF(je, 106, VALUE_LABEL)
OPDEF(jne, 107, VALUE_LABEL)
OPDEF(call, 108, VALUE_LABEL)
OPDEF(ret, 109, 0)
OPDEF(halt, 255, 0)
